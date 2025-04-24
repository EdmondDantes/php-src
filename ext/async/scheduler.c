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
#include <Zend/zend_async_API.h>

#include "coroutine.h"
#include "internal/zval_circular_buffer.h"
#include "php_scheduler.h"
#include "php_async.h"
#include "exceptions.h"
#include "zend_common.h"

void async_scheduler_startup(void)
{
}

void async_scheduler_shutdown(void)
{
}

zend_always_inline static void execute_microtasks(void)
{
	circular_buffer_t *buffer = &ASYNC_G(microtasks);
	zend_async_microtask_t *task = NULL;

	while (circular_buffer_is_not_empty(buffer)) {

		circular_buffer_pop(buffer, &task);

		if (EXPECTED(false == task->is_cancelled)) {
			task->handler(task);
		}

		task->ref_count--;

		if (task->ref_count <= 0) {
			task->dtor(task);
		}

		if (UNEXPECTED(EG(exception) != NULL)) {
			return;
		}
	}
}

zend_always_inline static async_coroutine_t * next_coroutine(void)
{
	if (circular_buffer_is_empty(&ASYNC_G(coroutine_queue))) {
		return NULL;
	}

	async_coroutine_t *coroutine;

	if (UNEXPECTED(circular_buffer_pop(&ASYNC_G(coroutine_queue), &coroutine) == FAILURE)) {
		ZEND_ASSERT("Failed to pop the coroutine from the pending queue.");
		return NULL;
	}

	return coroutine;
}

static zend_always_inline void switch_context(async_coroutine_t *coroutine, zend_object * exception)
{
	zend_fiber_transfer transfer = {
		.context = &coroutine->context,
		.flags = exception != NULL ? ZEND_FIBER_TRANSFER_FLAG_ERROR : 0,
	};

	if (exception != NULL) {
		ZVAL_OBJ(&transfer.value, exception);
	} else {
		ZVAL_NULL(&transfer.value);
	}

	ZEND_CURRENT_COROUTINE = &coroutine->coroutine;

	zend_fiber_switch_context(&transfer);

	/* Forward bailout into current coroutine. */
	if (UNEXPECTED(transfer.flags & ZEND_FIBER_TRANSFER_FLAG_BAILOUT)) {
		ZEND_CURRENT_COROUTINE = NULL;
		zend_bailout();
	}
}


static bool execute_next_coroutine(void)
{
	async_coroutine_t *async_coroutine = next_coroutine();
	zend_coroutine_t *coroutine = &async_coroutine->coroutine;

	if (UNEXPECTED(coroutine == NULL)) {
		return false;
	}

	if (UNEXPECTED(coroutine->waker == NULL)) {
		coroutine->dispose(coroutine);
		return true;
	}

	zend_async_waker_t * waker = coroutine->waker;

	if (UNEXPECTED(waker->status == ZEND_ASYNC_WAKER_IGNORED)) {

		//
		// This state triggers if the fiber has never been started;
		// in this case, it is deallocated differently than usual.
		// Finalizing handlers are called. Memory is freed in the correct order!
		//
		coroutine->dispose(coroutine);
		return true;
	}

	if (UNEXPECTED(waker->status == ZEND_ASYNC_WAKER_WAITING)) {
		zend_error(E_ERROR, "Attempt to resume a fiber that has not been resolved");
		coroutine->dispose(coroutine);
		return false;
	}

	ZEND_ASYNC_WAKER_STATUS status = waker->status;
	waker->status = ZEND_ASYNC_WAKER_NO_STATUS;

	zend_object * error = waker->error;
	waker->error = NULL;
	waker->dtor(coroutine);

	switch_context(async_coroutine, error);

	// Ignore the exception if it is a cancellation exception
	if (UNEXPECTED(EG(exception) && instanceof_function(EG(exception)->ce, async_ce_cancellation_exception))) {
        zend_clear_exception();
    }

	// Free if it is completed
	if (async_coroutine->context.status == ZEND_FIBER_STATUS_DEAD) {
		coroutine->dispose(coroutine);
	}

	if (error != NULL) {
		OBJ_RELEASE(error);
	}

	return true;
}

static bool resolve_deadlocks(void)
{
	zval *value;

	async_warning(
		"No active fibers, deadlock detected. Fibers in waiting: %u", zend_hash_num_elements(&ASYNC_G(fibers_state))
	);

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), value)

		const async_fiber_state_t* fiber_state = (async_fiber_state_t*)Z_PTR_P(value);

		ZEND_ASSERT(fiber_state->resume != NULL && "Fiber state has no resume object");

		if (fiber_state->resume != NULL && fiber_state->resume->filename != NULL) {

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

zend_always_inline static void execute_queued_coroutines(void)
{
	while (false == circular_buffer_is_empty(&ASYNC_G(coroutine_queue))) {
		execute_next_coroutine();

		if (UNEXPECTED(EG(exception))) {
			zend_exception_save();
		}
	}
}

static void async_scheduler_dtor(void)
{
	ZEND_IN_SCHEDULER_CONTEXT = true;

	execute_microtasks();

	ZEND_IN_SCHEDULER_CONTEXT = false;

	if (UNEXPECTED(false == circular_buffer_is_empty(&ASYNC_G(microtasks)))) {
		async_warning(
			"%u microtasks were not executed", circular_buffer_count(&ASYNC_G(microtasks))
		);
	}

	if (UNEXPECTED(false == circular_buffer_is_empty(&ASYNC_G(coroutine_queue)))) {
		async_warning(
			"%u deferred coroutines were not executed",
			circular_buffer_count(&ASYNC_G(coroutine_queue))
		);
	}

	zval_c_buffer_cleanup(&ASYNC_G(coroutine_queue));
	zval_c_buffer_cleanup(&ASYNC_G(microtasks));

	zval *current;
	// foreach by fibers_state and release all fibers
	ZEND_HASH_FOREACH_VAL(&ASYNC_G(coroutines), current) {
		zend_coroutine_t *coroutine = Z_PTR_P(current);
		OBJ_RELEASE(&coroutine->std);
	} ZEND_HASH_FOREACH_END();

	zend_hash_clean(&ASYNC_G(coroutines));

	ZEND_ASYNC_REACTOR_SHUTDOWN();

	EG(graceful_shutdown) = false;
	EG(in_scheduler_context) = false;
	EG(is_async) = false;

	zend_exception_restore();
}

static void dispose_coroutines(void)
{
	zval * current;

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(coroutines), current) {
		zend_coroutine_t *coroutine = Z_PTR_P(current);

		if (coroutine->waker != NULL) {
			coroutine->waker->status = ZEND_ASYNC_WAKER_IGNORED;
		}

		coroutine->dispose(coroutine);

		if (EG(exception)) {
			zend_exception_save();
		}

	} ZEND_HASH_FOREACH_END();
}

static void cancel_queued_coroutines(void)
{
	zend_exception_save();

	// 1. Walk through all fibers and cancel them if they are suspended.
	zval * current;

	zend_object * cancellation_exception = async_new_exception(async_ce_cancellation_exception, "Graceful shutdown");

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(coroutine_queue), current) {
		zend_coroutine_t *coroutine = Z_PTR_P(current);

		if (coroutine->context.status == ZEND_FIBER_STATUS_INIT) {
			// No need to cancel the fiber if it has not been started.
			coroutine->waker->status = ZEND_ASYNC_WAKER_IGNORED;
			coroutine->dispose(coroutine);
		} else {
			ZEND_ASYNC_CANCEL(coroutine, cancellation_exception, false);
		}

		if (EG(exception)) {
			zend_exception_save();
		}

	} ZEND_HASH_FOREACH_END();

	OBJ_RELEASE(cancellation_exception);

	zend_exception_restore();
}

static void start_graceful_shutdown(void)
{
	EG(graceful_shutdown) = true;
	EG(exit_exception) = EG(exception);
	GC_ADDREF(EG(exception));

	zend_clear_exception();
	cancel_queued_coroutines();

	if (UNEXPECTED(EG(exception) != NULL)) {
		zend_exception_set_previous(EG(exception), EG(exit_exception));
		GC_DELREF(EG(exit_exception));
		EG(exit_exception) = EG(exception);
		GC_ADDREF(EG(exception));
		zend_clear_exception();
	}
}

static void finally_shutdown(void)
{
	if (ZEND_EXIT_EXCEPTION != NULL && EG(exception) != NULL) {
		zend_exception_set_previous(EG(exception), ZEND_EXIT_EXCEPTION);
		GC_DELREF(ZEND_EXIT_EXCEPTION);
		ZEND_EXIT_EXCEPTION = EG(exception);
		GC_ADDREF(EG(exception));
		zend_clear_exception();
	}

	cancel_queued_coroutines();
	execute_queued_coroutines();

	execute_microtasks();

	if (UNEXPECTED(EG(exception))) {
		if (ZEND_EXIT_EXCEPTION != NULL) {
			zend_exception_set_previous(EG(exception), ZEND_EXIT_EXCEPTION);
			GC_DELREF(ZEND_EXIT_EXCEPTION);
			ZEND_EXIT_EXCEPTION = EG(exception);
			GC_ADDREF(EG(exception));
		}
	}
}

#define TRY_HANDLE_EXCEPTION() \
	if (UNEXPECTED(EG(exception) != NULL)) { \
	    if(ZEND_GRACEFUL_SHUTDOWN) { \
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
	if (ZEND_IS_ASYNC_ON) {
		async_throw_error("The scheduler cannot be started when is already enabled");
		return;
	}

	if (false == zend_async_reactor_is_enabled()) {
		async_throw_error("The scheduler cannot be started without the Reactor");
		return;
	}

	ZEND_ASYNC_REACTOR_STARTUP();

	if (UNEXPECTED(EG(exception) != NULL)) {
		return;
	}

	ZEND_ASYNC_ON;

	zend_try
	{
		bool has_handles = true;

		do {

			ZEND_IN_SCHEDULER_CONTEXT = true;

			execute_microtasks();
			TRY_HANDLE_EXCEPTION();

			has_handles = ZEND_ASYNC_REACTOR_EXECUTE(circular_buffer_is_not_empty(&ASYNC_G(coroutine_queue)));
			TRY_HANDLE_EXCEPTION();

			execute_microtasks();
			TRY_HANDLE_EXCEPTION();

			ZEND_IN_SCHEDULER_CONTEXT = false;

			bool was_executed = execute_next_coroutine();
			TRY_HANDLE_EXCEPTION();

			if (UNEXPECTED(
				false == has_handles
				&& false == was_executed
				&& &ASYNC_G(active_coroutine_count) > 0
				&& circular_buffer_is_empty(&ASYNC_G(coroutine_queue))
				&& circular_buffer_is_empty(&ASYNC_G(microtasks))
				&& resolve_deadlocks()
				)) {
				break;
			}

		} while (zend_hash_num_elements(&ASYNC_G(coroutines)) > 0
			|| circular_buffer_is_not_empty(&ASYNC_G(microtasks))
			|| ZEND_ASYNC_REACTOR_LOOP_ALIVE()
		);

	} zend_catch {
		dispose_coroutines();
		async_scheduler_dtor();
		zend_bailout();
	} zend_end_try();

	ZEND_ASSERT(ZEND_ASYNC_REACTOR_LOOP_ALIVE() == false && "The event loop must be stopped");

	zend_object * exit_exception = ZEND_EXIT_EXCEPTION;
	ZEND_EXIT_EXCEPTION = NULL;

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
