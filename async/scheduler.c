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
#include "php_layer/zend_common.h"

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

static bool execute_next_fiber(void)
{
	async_resume_t *resume = async_next_deferred_resume();

	if (resume == NULL) {
		return false;
	}

	if (UNEXPECTED(resume->status == ASYNC_RESUME_IGNORED)) {
		async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

		if (state != NULL) {
			state->resume = NULL;
		}

		OBJ_RELEASE(&resume->std);
		return false;
	}

	if (UNEXPECTED(resume->status == ASYNC_RESUME_WAITING)) {
		zend_error(E_ERROR, "Attempt to resume a fiber that has not been resolved");
		async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

		if (state != NULL) {
			state->resume = NULL;
		}

		OBJ_RELEASE(&resume->std);
		return false;
	}

	zval retval;
	ZVAL_UNDEF(&retval);

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);
	ZEND_ASSERT(state != NULL && "Fiber state not found but required");

	if (EXPECTED(state != NULL)) {
		state->resume = NULL;
	}

	// After the fiber is resumed, the resume object is no longer needed.
	// So we need to release the reference to the object before resuming the fiber.
	// Copy the resume object status and fiber to local variables.
	ASYNC_RESUME_STATUS status = resume->status;
	resume->status = ASYNC_RESUME_NO_STATUS;
	zend_fiber *fiber = resume->fiber;
	zval result = resume->result;
	ZVAL_UNDEF(&resume->result);

	zend_object * error = resume->error;
	resume->error = NULL;

	OBJ_RELEASE(&resume->std);

	if (EXPECTED(status == ASYNC_RESUME_SUCCESS)) {

		if (UNEXPECTED(fiber->context.status == ZEND_FIBER_STATUS_INIT)) {
			zend_fiber_start(fiber, &retval);
		} else {
			zend_fiber_resume(fiber, &result, &retval);
		}

	} else {

		if (UNEXPECTED(fiber->context.status == ZEND_FIBER_STATUS_INIT)) {
			async_warning("Attempt to resume with error a fiber that has not been started");
			zend_fiber_start(fiber, &retval);
		} else {
			zval zval_error;
			ZVAL_OBJ(&zval_error, error);
			zend_fiber_resume_exception(fiber, &zval_error, &retval);
		}
	}

	// Ignore the exception if it is a cancellation exception
	if (UNEXPECTED(EG(exception) && instanceof_function(EG(exception)->ce, async_ce_cancellation_exception))) {
        zend_clear_exception();
    }

	// Free fiber if it is completed
	if (fiber->context.status == ZEND_FIBER_STATUS_DEAD) {
		OBJ_RELEASE(&fiber->std);
	}

	if (error != NULL) {
		OBJ_RELEASE(error);
	}

	zval_ptr_dtor(&result);
	zval_ptr_dtor(&retval);

	return true;
}

static bool resolve_deadlocks(void)
{
	zval *value;
	zend_ulong index;
	zend_string *key;

	async_warning(
		"No active fibers, deadlock detected. Fibers in waiting: %u", zend_hash_num_elements(&ASYNC_G(fibers_state))
	);

	ZEND_HASH_FOREACH_KEY_VAL(&ASYNC_G(fibers_state), index, key, value)

		const async_fiber_state_t* fiber_state = (async_fiber_state_t*)Z_PTR_P(value);

		if (fiber_state->resume->filename != NULL) {

			//Maybe we need to get the function name
			//zend_string * function_name = NULL;
			//zend_get_function_name_by_fci(&fiber_state->fiber->fci, &fiber_state->fiber->fci_cache, &function_name);

			async_warning(
				"Resume that suspended in file: %s, line: %d will be canceled",
				ZSTR_VAL(fiber_state->resume->filename),
				fiber_state->resume->lineno
			);
		}

		async_cancel_fiber(
			fiber_state->fiber,
			async_new_exception(async_ce_cancellation_exception, "Deadlock detected"),
			true
		);

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

zend_always_inline void execute_deferred_fibers(void)
{
	const async_next_fiber_handler_t execute_next_fiber_handler = ASYNC_G(execute_next_fiber_handler) ?
									ASYNC_G(execute_next_fiber_handler) : execute_next_fiber;

	while (false == circular_buffer_is_empty(&ASYNC_G(deferred_resumes))) {
		execute_next_fiber_handler();

		if (UNEXPECTED(EG(exception))) {
			zend_exception_save();
		}
	}
}

static void call_fiber_deferred_callbacks(void)
{
	zval * current;

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), current) {
		const async_fiber_state_t *fiber_state = Z_PTR_P(current);

		if (fiber_state->resume != NULL) {
			fiber_state->resume->status = ASYNC_RESUME_IGNORED;
		}

		zend_fiber_finalize(fiber_state->fiber);

		if (EG(exception)) {
			zend_exception_save();
		}

	} ZEND_HASH_FOREACH_END();
}

static void cancel_deferred_fibers(void)
{
	zend_exception_save();

	// 1. Walk through all fibers and cancel them if they are suspended.
	zval * current;

	zend_object * cancellation_exception = async_new_exception(async_ce_cancellation_exception, "Graceful shutdown");

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), current) {
		async_fiber_state_t *fiber_state = Z_PTR_P(current);

		if (fiber_state->fiber->context.status == ZEND_FIBER_STATUS_INIT) {
			// No need to cancel the fiber if it has not been started.
			fiber_state->resume->status = ASYNC_RESUME_IGNORED;
			zend_fiber_finalize(fiber_state->fiber);
		} else {
			async_cancel_fiber(fiber_state->fiber, cancellation_exception, false);
		}

		if (EG(exception)) {
			zend_exception_save();
		}

	} ZEND_HASH_FOREACH_END();

	OBJ_RELEASE(cancellation_exception);

	zend_exception_restore();
}

static void finally_shutdown(void)
{
	if (ASYNC_G(exit_exception) != NULL && EG(exception) != NULL) {
		zend_exception_set_previous(EG(exception), ASYNC_G(exit_exception));
		GC_DELREF(ASYNC_G(exit_exception));
		ASYNC_G(exit_exception) = EG(exception);
		GC_ADDREF(EG(exception));
		zend_clear_exception();
	}

	cancel_deferred_fibers();
	execute_deferred_fibers();

	const async_microtasks_handler_t execute_microtasks_handler = ASYNC_G(execute_microtasks_handler)
							? ASYNC_G(execute_microtasks_handler) : execute_microtasks;

	execute_microtasks_handler();

	if (UNEXPECTED(EG(exception))) {
		if (ASYNC_G(exit_exception) != NULL) {
			zend_exception_set_previous(EG(exception), ASYNC_G(exit_exception));
			GC_DELREF(ASYNC_G(exit_exception));
			ASYNC_G(exit_exception) = EG(exception);
			GC_ADDREF(EG(exception));
		}
	}
}

static void start_graceful_shutdown(void)
{
	ASYNC_G(graceful_shutdown) = true;
	ASYNC_G(exit_exception) = EG(exception);
	GC_ADDREF(EG(exception));

	zend_clear_exception();
	cancel_deferred_fibers();

	if (UNEXPECTED(EG(exception)) != NULL) {
		zend_exception_set_previous(EG(exception), ASYNC_G(exit_exception));
		GC_DELREF(ASYNC_G(exit_exception));
		ASYNC_G(exit_exception) = EG(exception);
		GC_ADDREF(EG(exception));
		zend_clear_exception();
	}
}

static void async_scheduler_dtor(void)
{
	ASYNC_G(in_scheduler_context) = false;

	if (UNEXPECTED(false == circular_buffer_is_empty(&ASYNC_G(microtasks)))) {
		async_warning(
			"%u microtasks were not executed", circular_buffer_count(&ASYNC_G(microtasks))
		);
	}

	if (UNEXPECTED(false == circular_buffer_is_empty(&ASYNC_G(deferred_resumes)))) {
		async_warning(
			"%u deferred resumes were not executed",
			circular_buffer_count(&ASYNC_G(deferred_resumes))
		);
	}

	zval_c_buffer_cleanup(&ASYNC_G(deferred_resumes));
	zval_c_buffer_cleanup(&ASYNC_G(microtasks));
	zend_hash_clean(&ASYNC_G(defer_callbacks));

	zval *current;
	// foreach by fibers_state and release all fibers
	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), current) {
		async_fiber_state_t *fiber_state = Z_PTR_P(current);

		if (fiber_state->fiber != NULL) {
			OBJ_RELEASE(&fiber_state->fiber->std);
		}
	} ZEND_HASH_FOREACH_END();

	zend_hash_clean(&ASYNC_G(fibers_state));

	reactor_shutdown_fn();
	ASYNC_G(graceful_shutdown) = false;
	ASYNC_G(in_scheduler_context) = false;
	ASYNC_G(is_async) = false;

	zend_exception_restore();
}

#define TRY_HANDLE_EXCEPTION() \
	if (UNEXPECTED(EG(exception) != NULL && handle_exception_handler != NULL)) { \
		handle_exception_handler(); \
	} \
	if (UNEXPECTED(EG(exception) != NULL)) { \
	    if(ASYNC_G(graceful_shutdown)) { \
			finally_shutdown(); \
            break; \
        } \
		start_graceful_shutdown(); \
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

	ASYNC_G(is_async) = true;

	reactor_startup_fn();

	/**
	 * Handlers for the scheduler.
	 * This functions pointer will be set to the actual functions.
	 */
	const async_callbacks_handler_t execute_callbacks_handler = ASYNC_G(execute_callbacks_handler);
	const async_microtasks_handler_t execute_microtasks_handler = ASYNC_G(execute_microtasks_handler)
													? ASYNC_G(execute_microtasks_handler) : execute_microtasks;
	const async_next_fiber_handler_t execute_next_fiber_handler = ASYNC_G(execute_next_fiber_handler)
													? ASYNC_G(execute_next_fiber_handler) : execute_next_fiber;
	const async_exception_handler_t handle_exception_handler = ASYNC_G(exception_handler);

	zend_try
	{
		zend_bool has_handles = true;

		do {

			ASYNC_G(in_scheduler_context) = true;

			execute_microtasks_handler();
			TRY_HANDLE_EXCEPTION();

			has_handles = execute_callbacks_handler(circular_buffer_is_not_empty(&ASYNC_G(deferred_resumes)));
			TRY_HANDLE_EXCEPTION();

			execute_microtasks_handler();
			TRY_HANDLE_EXCEPTION();

			ASYNC_G(in_scheduler_context) = false;

			bool was_executed = execute_next_fiber_handler();
			TRY_HANDLE_EXCEPTION();

			if (UNEXPECTED(
				false == has_handles
				&& false == was_executed
				&& zend_hash_num_elements(&ASYNC_G(fibers_state)) > 0
				&& circular_buffer_is_empty(&ASYNC_G(deferred_resumes))
				&& circular_buffer_is_empty(&ASYNC_G(microtasks))
				&& resolve_deadlocks()
				)) {
				break;
			}

		} while (zend_hash_num_elements(&ASYNC_G(fibers_state)) > 0
			|| circular_buffer_is_not_empty(&ASYNC_G(microtasks))
			|| reactor_loop_alive_fn()
		);

	} zend_catch {
		call_fiber_deferred_callbacks();
		async_scheduler_dtor();
		zend_bailout();
	} zend_end_try();

	ZEND_ASSERT(reactor_loop_alive_fn() == false && "The event loop must be stopped");

	zend_object * exit_exception = ASYNC_G(exit_exception);
	ASYNC_G(exit_exception) = NULL;

	async_scheduler_dtor();

	if (EG(exception) != NULL && exit_exception != NULL) {
		zend_exception_set_previous(EG(exception), exit_exception);
		GC_DELREF(exit_exception);
		exit_exception = EG(exception);
		GC_ADDREF(exit_exception);
		zend_clear_exception();
	}

	if (exit_exception != NULL) {
		zend_throw_exception_internal(exit_exception);
	}
}
