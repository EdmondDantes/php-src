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

zend_always_inline static void execute_microtasks(circular_buffer_t *buffer)
{
	zend_async_microtask_t * task = NULL;

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

static void async_scheduler_dtor(void)
{
	IN_SCHEDULER_CONTEXT = true;

	execute_microtasks(&ASYNC_G(microtasks));

	IN_SCHEDULER_CONTEXT = false;

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
	if (IS_ASYNC_ON) {
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

	ASYNC_ON;

	zend_try
	{
		bool has_handles = true;

		do {

			IN_SCHEDULER_CONTEXT = true;

			execute_microtasks(&ASYNC_G(microtasks));
			TRY_HANDLE_EXCEPTION();

			has_handles = ZEND_ASYNC_REACTOR_EXECUTE(circular_buffer_is_not_empty(&ASYNC_G(coroutine_queue)));
			TRY_HANDLE_EXCEPTION();

			execute_microtasks(&ASYNC_G(microtasks));
			TRY_HANDLE_EXCEPTION();

			IN_SCHEDULER_CONTEXT = false;

			bool was_executed = execute_next_fiber();
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

	zend_object * exit_exception = EG(exit_exception);
	EG(exit_exception) = NULL;

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
