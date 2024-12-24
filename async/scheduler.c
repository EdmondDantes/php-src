/*
+----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Edmond                                                       |
  +----------------------------------------------------------------------+
*/
#include "scheduler.h"

#include <Zend/zend_fibers.h>

#include "internal/zval_circular_buffer.h"
#include "async.h"
#include "php_layer/exceptions.h"

//
// The coefficient of the maximum number of microtasks that can be executed
// without suspicion of an infinite loop (a task that creates microtasks).
//
#define MICROTASK_CYCLE_THRESHOLD_C 4

static void invoke_microtask(zval *task)
{
	if (Z_TYPE(task) == IS_PTR) {
		// Call the function directly
		((void (*)(void)) Z_PTR(*task))();
	} else if (zend_is_callable(task, 0, NULL)) {
		zval retval;
		ZVAL_UNDEF(&retval);
		call_user_function(CG(function_table), NULL, task, &retval, 0, NULL);
		zval_ptr_dtor(&retval);
	}
}

static zend_result execute_microtasks_stage(circular_buffer_t *buffer, const size_t max_count)
{
	if (circular_buffer_is_empty(buffer)) {
		return SUCCESS;
	}

	for (size_t i = 0; i < max_count; i++) {
		zval task;

		zval_c_buffer_pop(buffer, &task);
		invoke_microtask(&task);
		zval_ptr_dtor(&task);

		if (circular_buffer_is_empty(buffer)) {
			return SUCCESS;
		}
	}

	return FAILURE;
}

static void execute_microtasks(void)
{
	circular_buffer_t *buffer = &ASYNC_G(microtasks);

	if (circular_buffer_is_empty(buffer)) {
		return;
	}

	/**
	 * The execution of the microtask queue occurs in two stages:
	 * * All microtasks in the queue that were initially scheduled are executed.
	 * * All subsequent microtasks in the queue are executed, but no more than MICROTASK_CYCLE_THRESHOLD_C the buffer size.
	*/

	if (execute_microtasks_stage(buffer, circular_buffer_count(buffer)) == SUCCESS) {
		return;
	}

	const size_t max_count = circular_buffer_capacity(buffer) * MICROTASK_CYCLE_THRESHOLD_C;

	if (execute_microtasks_stage(buffer, max_count) == SUCCESS) {
		return;
	}

	// TODO: make critical error
	async_throw_error(
		"A possible infinite loop was detected during microtask execution. Max count: %u, remaining: %u",
		max_count,
		circular_buffer_count(buffer)
	);
}

static void handle_callbacks(void)
{
	async_throw_error("Event Loop API method handle_callbacks not implemented");
}

static void resume_next_fiber(void)
{
	if (circular_buffer_is_empty(&ASYNC_G(pending_fibers))) {
        return;
    }

	async_resume_t *resume;
	circular_buffer_pop(&ASYNC_G(pending_fibers), &resume);

	if (resume->status == ASYNC_RESUME_PENDING) {
		zend_error(E_ERROR, "Attempt to resume a fiber that has not been resolved");
		GC_DELREF(&resume->std);
		return;
	}

	zval retval;
	ZVAL_UNDEF(&retval);

	if (resume->status == ASYNC_RESUME_SUCCESS) {
		zend_fiber_resume(resume->fiber, resume->value, &retval);
	} else {
		zend_fiber_resume_exception(resume->fiber, resume->error, &retval);
	}

	GC_DELREF(&resume->std);
	zval_ptr_dtor(&retval);
}

/**
 * Handlers for the scheduler.
 * This functions pointer will be set to the actual functions.
 */
static async_execute_microtasks_handler_t execute_microtasks_fn = execute_microtasks;
static async_callback_handler_t handle_callbacks_fn = handle_callbacks;
static async_resume_next_fiber_handler_t resume_next_fiber_fn = resume_next_fiber;

ZEND_API async_callback_handler_t async_scheduler_set_callback_handler(const async_callback_handler_t handler)
{
	const async_callback_handler_t prev = handle_callbacks_fn;
	handle_callbacks_fn = handler ? handler : handle_callbacks;
	return prev;
}

ZEND_API async_resume_next_fiber_handler_t async_scheduler_set_next_fiber_handler(const async_resume_next_fiber_handler_t handler)
{
	const async_resume_next_fiber_handler_t prev = resume_next_fiber_fn;
	resume_next_fiber_fn = handler ? handler : resume_next_fiber;
	return prev;
}

ZEND_API async_execute_microtasks_handler_t async_scheduler_set_microtasks_handler(const async_execute_microtasks_handler_t handler)
{
	const async_execute_microtasks_handler_t prev = execute_microtasks_fn;
	execute_microtasks_fn = handler ? handler : execute_microtasks;
	return prev;
}

zend_result async_scheduler_fiber_resume()
{
    return SUCCESS;
}

void async_scheduler_loop(void)
{
	do {

		ASYNC_G(is_scheduler_running) = true;

		zend_try {

			execute_microtasks_fn();
			handle_callbacks_fn();
			execute_microtasks_fn();

		} zend_catch {
			ASYNC_G(is_scheduler_running) = false;
			zend_bailout();
		} zend_end_try();

		ASYNC_G(is_scheduler_running) = false;

	} while (circular_buffer_is_empty(&ASYNC_G(pending_fibers)));

	resume_next_fiber_fn();
}

