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
#include <Zend/zend_fibers.h>
#include "php_scheduler.h"
#include "php_async.h"
#include "php_reactor.h"
#include "internal/zval_circular_buffer.h"
#include "php_layer/exceptions.h"

//
// The coefficient of the maximum number of microtasks that can be executed
// without suspicion of an infinite loop (a task that creates microtasks).
//
#define MICROTASK_CYCLE_THRESHOLD_C 4

static void invoke_microtask(zval *task)
{
	if (Z_TYPE_P(task) == IS_PTR) {
		// Call the function directly
		((void (*)(void)) Z_PTR_P(task))();
	} else {
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

static zend_bool handle_callbacks(zend_bool no_wait)
{
	async_throw_error("Event Loop API method handle_callbacks not implemented");
	return false;
}

static void execute_next_fiber(void)
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
		zend_fiber_resume(resume->fiber, &GET_RESUME_RESULT(resume), &retval);
	} else {
		zval zval_error;
		ZVAL_OBJ(&zval_error, resume->error);
		zend_fiber_resume_exception(resume->fiber, &zval_error, &retval);
	}

	GC_DELREF(&resume->std);
	zval_ptr_dtor(&retval);
}

static zend_always_inline void put_fiber_to_pending(const async_resume_t *resume)
{
	circular_buffer_push(&ASYNC_G(pending_fibers), resume, true);
}

static void validate_fiber_status(zend_fiber *fiber, const zend_ulong index)
{
	if (fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED) {
		// TODO: create resume object
		put_fiber_to_pending(NULL);
	} else if (fiber->context.status == ZEND_FIBER_STATUS_DEAD) {
		// Just remove the fiber from the list
		GC_DELREF(&fiber->std);
		zend_hash_index_del(&ASYNC_G(fibers_state), index);
	} else {
		async_throw_error(
			"Fiber detected without a Resume object in an invalid state. Fiber status: %d",
			fiber->context.status
		);
	}
}

static void analyze_resume_waiting(const async_resume_t *resume)
{
    if (resume->status == ASYNC_RESUME_PENDING) {
        put_fiber_to_pending(resume);
    }
}

/**
 * The method checks all fibers and their Resume objects for the existence of a DeadLock or a logical processing error.
 * If a DeadLock is detected, the method throws a PHP exception and terminates the loop execution.
 *
 * @return zend_bool
 */
static zend_bool check_deadlocks(void)
{
	zval *value;
	zend_ulong index;
	zend_string *key;

	ZEND_HASH_FOREACH_KEY_VAL(&ASYNC_G(fibers_state), index, key, value)

		const async_fiber_state_t* fiber_state = (async_fiber_state_t*)Z_PTR_P(value);

		if (fiber_state->resume == NULL) {
			validate_fiber_status(fiber_state->fiber, index);
        } else {
        	analyze_resume_waiting(fiber_state->resume);
        }

		if (EG(exception) != NULL) {
			return true;
		}
	ZEND_HASH_FOREACH_END();

	return false;
}


ZEND_API async_callbacks_handler_t async_scheduler_set_callbacks_handler(async_callbacks_handler_t handler)
{
    const async_callbacks_handler_t prev = ASYNC_G(execute_callbacks_handler);
    ASYNC_G(execute_callbacks_handler) = handler ? handler : handle_callbacks;
    return prev;
}

ZEND_API async_next_fiber_handler_t async_scheduler_set_next_fiber_handler(const async_next_fiber_handler_t handler)
{
	const async_next_fiber_handler_t prev = ASYNC_G(execute_next_fiber_handler);
	ASYNC_G(execute_next_fiber_handler) = handler ? handler : execute_next_fiber;
	return prev;
}

ZEND_API async_microtasks_handler_t async_scheduler_set_microtasks_handler(const async_microtasks_handler_t handler)
{
	const async_microtasks_handler_t prev = ASYNC_G(execute_microtasks_handler);
	ASYNC_G(execute_microtasks_handler) = handler ? handler : execute_microtasks;
	return prev;
}

ZEND_API async_exception_handler_t async_scheduler_set_exception_handler(const async_exception_handler_t handler)
{
	const async_exception_handler_t prev = ASYNC_G(exception_handler);
	ASYNC_G(exception_handler) = handler;
	return prev;
}

ZEND_API void async_scheduler_add_microtask(zval *microtask)
{
	if (EXPECTED(Z_TYPE_P(microtask) == IS_PTR || zend_is_callable(microtask, 0, NULL))) {
		zval_c_buffer_push_with_resize(&ASYNC_G(microtasks), microtask);
		return;
	}

	async_throw_error("Invalid microtask type: should be a callable or a internal pointer to a function");
}

/**
 * Async globals destructor.
 */
ZEND_API void async_scheduler_dtor(zend_async_globals *async_globals)
{
	if (!async_globals->is_async) {
		return;
	}

	async_globals->is_async = false;
	async_globals->in_scheduler_context = false;

	circular_buffer_dtor(&async_globals->microtasks);
	circular_buffer_dtor(&async_globals->pending_fibers);
	zend_hash_destroy(&async_globals->fibers_state);
}

/**
 * Async globals constructor.
 */
static void async_scheduler_ctor(zend_async_globals *async_globals)
{
	if (async_globals->is_async) {
		return;
	}

	async_globals->is_async = true;
	async_globals->in_scheduler_context = false;

	circular_buffer_ctor(&async_globals->microtasks, 32, sizeof(zval), &zend_std_persistent_allocator);
	circular_buffer_ctor(&async_globals->pending_fibers, 128, sizeof(async_resume_t *), &zend_std_persistent_allocator);
	zend_hash_init(&async_globals->fibers_state, 128, NULL, NULL, 1);

	async_globals->execute_callbacks_handler = NULL;
	async_globals->exception_handler = NULL;
	async_globals->execute_next_fiber_handler = execute_next_fiber;
	async_globals->execute_microtasks_handler = execute_microtasks;

	if (EG(exception) != NULL) {
		async_scheduler_dtor(async_globals);
		return;
	}

	if (EG(exception) != NULL) {
		async_scheduler_dtor(async_globals);
	}
}

#define TRY_HANDLE_EXCEPTION() \
	if (EG(exception) != NULL && handle_exception_handler != NULL) { \
		handle_exception_handler(); \
	} \
	if (EG(exception) != NULL) { \
		break; \
	}

/**
 * The main loop of the scheduler.
 */
void async_scheduler_launch(void)
{
	if (EG(active_fiber) != NULL) {
		async_throw_error("The scheduler cannot be started from a Fiber");
		return;
	}

	if (false == reactor_is_enabled()) {
		async_throw_error("The scheduler cannot be started without the Reactor");
		return;
	}

	async_scheduler_ctor(ASYNC_GLOBAL);

	/**
	 * Handlers for the scheduler.
	 * This functions pointer will be set to the actual functions.
	 */
	const async_callbacks_handler_t execute_callbacks_handler = ASYNC_G(execute_callbacks_handler);
	const async_microtasks_handler_t execute_microtasks_handler = ASYNC_G(execute_microtasks_handler);
	const async_next_fiber_handler_t execute_next_fiber_handler = ASYNC_G(execute_next_fiber_handler);
	const async_exception_handler_t handle_exception_handler = ASYNC_G(exception_handler);

	zend_try
	{
		zend_bool has_handles = true;

		do {

			ASYNC_G(in_scheduler_context) = true;

			execute_microtasks_handler();
			TRY_HANDLE_EXCEPTION();

			has_handles = execute_callbacks_handler(circular_buffer_is_not_empty(&ASYNC_G(pending_fibers)));
			TRY_HANDLE_EXCEPTION();

			execute_microtasks_handler();
			TRY_HANDLE_EXCEPTION();

			ASYNC_G(in_scheduler_context) = false;

			execute_next_fiber_handler();
			TRY_HANDLE_EXCEPTION();

			if (false == has_handles && circular_buffer_is_empty(&ASYNC_G(pending_fibers)) && check_deadlocks()) {
				break;
			}

		} while (zend_hash_num_elements(&ASYNC_G(fibers_state)) > 0);

	} zend_catch {
		ASYNC_G(in_scheduler_context) = false;
		async_scheduler_dtor(ASYNC_GLOBAL);
		zend_bailout();
	} zend_end_try();
}