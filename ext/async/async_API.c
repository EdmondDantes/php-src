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
#include "async_API.h"

#include "exceptions.h"
#include "php_async.h"
#include "scheduler.h"
#include "scope.h"
#include "zend_common.h"

static void event_callback_dispose(zend_async_event_callback_t *callback)
{
	if (callback->ref_count != 0) {
		return;
	}

	efree(callback);
}

ZEND_API void async_resume_when_callback_resolve(
	zend_async_event_t *event, zend_async_event_callback_t *callback, void * result, zend_object *exception
)
{
	zend_coroutine_t * coroutine = ((zend_coroutine_event_callback_t *) callback)->coroutine;

	if (exception == NULL && coroutine->waker != NULL) {
		zend_hash_index_add_ptr(&coroutine->waker->events, (zend_ulong)event, result);
	}

	ZEND_ASYNC_RESUME_WITH_ERROR(coroutine, exception, false);
}

ZEND_API void async_resume_when_callback_cancel(
	zend_async_event_t *event, zend_async_event_callback_t *callback, void * result, zend_object *exception
)
{
	if (UNEXPECTED(error != NULL) && Z_TYPE_P(error) == IS_OBJECT) {
		async_resume_fiber(resume, NULL, Z_OBJ_P(error));
	} else {
		zend_object * exception;

		if (notifier->std.ce == async_ce_timer_handle) {
			exception = async_new_exception(async_ce_cancellation_exception, "Operation has been cancelled by timeout");
		} else {
			exception = async_new_exception(async_ce_cancellation_exception, "Operation has been cancelled");
		}

		async_resume_fiber(resume, NULL, exception);
		GC_DELREF(exception);
	}
}

ZEND_API void async_resume_when_callback_timeout(
	async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error, async_resume_notifier_t *resume_notifier
)
{
	if (UNEXPECTED(error != NULL) && Z_TYPE_P(error) == IS_OBJECT) {
		async_resume_fiber(resume, NULL, Z_OBJ_P(error));
	} else if (resume->status == ASYNC_RESUME_WAITING) {
		//
		// If the operation has not been completed yet, we will resume the Fiber with a timeout exception.
		//
		zend_object * exception = async_new_exception(async_ce_timeout_exception, "Operation has been cancelled by timeout");
		async_resume_fiber(resume, NULL, exception);
		GC_DELREF(exception);
	}
}

ZEND_API void async_resume_when(
		zend_coroutine_t			*coroutine,
		zend_async_event_t			*event,
		const bool					trans_event,
		zend_async_event_callback_fn callback,
		zend_coroutine_event_callback_t *event_callback
	)
{
	if (UNEXPECTED(Z_TYPE(event->is_closed) == IS_TRUE)) {
		async_throw_error("The event cannot be used after it has been terminated");
		return;
	}

	if (UNEXPECTED(callback == NULL && event_callback == NULL)) {
		async_warning("Callback cannot be NULL");

		if (trans_event) {
			event->dispose(event);
		}

		return;
	}

	if (event_callback == NULL) {
		event_callback = emalloc(sizeof(zend_coroutine_event_callback_t));
		event_callback->base.ref_count = 1;
		event_callback->base.callback = callback;
		event_callback->base.dispose = event_callback_dispose;
	}

	event_callback->coroutine = coroutine;
	event->add_callback(event, &event_callback->base);

	if (UNEXPECTED(EG(exception) != NULL)) {
		if (trans_event) {
			event->dispose(event);
		}

		return;
	}

	if (false == trans_event) {
		event->ref_count++;
	}

	if (EXPECTED(coroutine->waker != NULL)) {
		if (UNEXPECTED(zend_hash_index_add_ptr(&coroutine->waker->events, (zend_ulong)event, event) == NULL)) {
			async_throw_error("Failed to add event to the waker");
			return;
		}
	}
}

zend_async_waker_t * async_new_waker_with_timeout(
	async_coroutine_t * coroutine, const zend_ulong timeout, zend_async_event_t *cancellation
)
{
	zend_async_waker_t * waker = zend_async_waker_create(&coroutine->coroutine);

	if (UNEXPECTED(EG(exception))) {
		return NULL;
	}

	if (timeout > 0) {
		async_resume_when(
			&coroutine->coroutine,
			&ZEND_ASYNC_NEW_TIMER_EVENT(timeout, false)->base,
			true,
			async_resume_when_callback_resolve,
			NULL
		);
	}

	if (cancellation != NULL) {
		async_resume_when(
			&coroutine->coroutine,
			cancellation,
			false,
			async_resume_when_callback_cancel,
			NULL
		);
	}

	return waker;
}

async_scope_t * new_scope(void)
{
	DEFINE_ZEND_INTERNAL_OBJECT(async_scope_t, scope, async_ce_scope);

	if (UNEXPECTED(EG(exception))) {
		return NULL;
	}

	return scope;
}

zend_coroutine_t *new_coroutine(zend_async_scope_t *scope)
{
	DEFINE_ZEND_INTERNAL_OBJECT(async_coroutine_t, coroutine, async_ce_coroutine);

	if (UNEXPECTED(EG(exception))) {
		return NULL;
	}

	return &coroutine->coroutine;
}

zend_coroutine_t *spawn(zend_async_scope_t *scope)
{
	if (UNEXPECTED(ZEND_IS_ASYNC_OFF)) {
		async_throw_error("Cannot spawn a coroutine when async is disabled");
		return NULL;
	}

	if (scope == NULL) {

		if (UNEXPECTED(ZEND_CURRENT_ASYNC_SCOPE == NULL)) {
			ZEND_CURRENT_ASYNC_SCOPE = (zend_async_scope_t *) new_scope();

			if (UNEXPECTED(EG(exception))) {
				return NULL;
			}
		}

		scope = ZEND_CURRENT_ASYNC_SCOPE;
	}

	if (UNEXPECTED(scope == NULL)) {
		async_throw_error("Cannot spawn a coroutine without a scope");
		return NULL;
	}

	if (UNEXPECTED(scope->is_closed)) {
		async_throw_error("Cannot spawn a coroutine in a closed scope");
		return NULL;
	}

	async_coroutine_t *coroutine = (async_coroutine_t *) new_coroutine(scope);
	if (UNEXPECTED(EG(exception))) {
		return NULL;
	}

	zval options;
	ZVAL_UNDEF(&options);
	scope->before_coroutine_enqueue(&coroutine->coroutine, scope, &options);
	zval_dtor(&options);

	if (UNEXPECTED(EG(exception))) {
		coroutine->coroutine.dispose(&coroutine->coroutine);
		return NULL;
	}

	zend_async_waker_t *waker = zend_async_waker_create(&coroutine->coroutine);
	if (UNEXPECTED(EG(exception))) {
		coroutine->coroutine.dispose(&coroutine->coroutine);
		return NULL;
	}

	waker->status = ZEND_ASYNC_WAKER_QUEUED;

	if (UNEXPECTED(circular_buffer_push(&ASYNC_G(coroutine_queue), coroutine, true)) == FAILURE) {
		coroutine->coroutine.dispose(&coroutine->coroutine);
		async_throw_error("Failed to enqueue coroutine");
		return NULL;
	}

	scope->after_coroutine_enqueue(&coroutine->coroutine, scope);
	if (UNEXPECTED(EG(exception))) {
		waker->status = ZEND_ASYNC_WAKER_IGNORED;
		return NULL;
	}

	if (UNEXPECTED(zend_hash_index_add_ptr(&ASYNC_G(coroutines), coroutine->std.handle, coroutine) == NULL)) {
		waker->status = ZEND_ASYNC_WAKER_IGNORED;
		async_throw_error("Failed to add coroutine to the list");
		return NULL;
	}

	ASYNC_G(active_coroutine_count)++;

	return &coroutine->coroutine;
}

void suspend(void)
{
	async_scheduler_coroutine_suspend(NULL);
}

void resume(zend_coroutine_t *coroutine)
{
	if (UNEXPECTED(coroutine->waker == NULL)) {
		async_throw_error("Cannot resume a coroutine that has not been suspended");
		return;
	}

	if (UNEXPECTED(coroutine->waker->status == ZEND_ASYNC_WAKER_QUEUED)) {
		return;
	}

	if (UNEXPECTED(circular_buffer_push(&ASYNC_G(coroutine_queue), coroutine, true)) == FAILURE) {
		async_throw_error("Failed to enqueue coroutine");
		return;
	}

	coroutine->waker->status = ZEND_ASYNC_WAKER_QUEUED;
}

void cancel(zend_coroutine_t *coroutine, zend_object *error, const bool transfer_error)
{
	if (coroutine->waker == NULL) {
		zend_async_waker_create(coroutine);
	}

	if (UNEXPECTED(coroutine->waker == NULL)) {
		async_throw_error("Waker is not initialized");

		if (transfer_error) {
			OBJ_RELEASE(error);
		}

		return;
	}

	const bool is_error_null = (error == NULL);

	if (is_error_null) {
		error = async_new_exception(async_ce_cancellation_exception, "Coroutine cancelled");
		if (UNEXPECTED(EG(exception))) {
			return;
		}
	}

	if (coroutine->waker->error != NULL) {
		zend_exception_set_previous(error, coroutine->waker->error);
		OBJ_RELEASE(coroutine->waker->error);
	}

	coroutine->waker->error = error;

	if (false == transfer_error && false == is_error_null) {
		GC_ADDREF(error);
	}
}

void shutdown(void)
{
	start_graceful_shutdown();
}

zend_array * get_coroutines(void)
{
	return &ASYNC_G(coroutines);
}

void add_microtask(zend_async_microtask_t *microtask)
{
	if (microtask->is_cancelled) {
		return;
	}

	if (UNEXPECTED(circular_buffer_push(&ASYNC_G(microtasks), microtask, true) == FAILURE)) {
		async_throw_error("Failed to enqueue microtask");
		return;
	}
}

zend_array *get_awaiting_info(zend_coroutine_t *coroutine)
{
	/* @todo: implement get_awaiting_info */
	return NULL;
}

void async_await_futures(
	zval *iterable,
	int count,
	bool ignore_errors,
	zend_async_event_t *cancellation,
	zend_ulong timeout,
	HashTable *results,
	HashTable *errors,
	unsigned int concurrency
)
{
	HashTable *futures = NULL;
	zend_object_iterator *zend_iterator = NULL;

	if (Z_TYPE_P(iterable) == IS_ARRAY) {
		futures = Z_ARR_P(iterable);
	} else if (Z_TYPE_P(iterable) == IS_OBJECT && Z_OBJCE_P(iterable)->get_iterator) {
		zend_iterator = Z_OBJCE_P(iterable)->get_iterator(Z_OBJCE_P(iterable), iterable, 0);

		if (EG(exception) == NULL && zend_iterator == NULL) {
			async_throw_error("Failed to create iterator");
		}

    } else {
	    async_throw_error("Expected parameter 'iterable' to be an array or an object implementing Traversable");
    }

	if (UNEXPECTED(EG(exception))) {
		return;
	}

	zend_ulong index;
	zend_string *key;
	zval * current;
	zend_async_event_t * future_state;

	zend_async_waker_t *waker;
	async_await_conditions_t *conditions = NULL;
	future_resume_callback_t *callback = NULL;

	if (futures != NULL)
	{
		if (zend_hash_num_elements(futures) == 0) {
			return;
		}

		waker = async_new_waker_with_timeout(NULL, timeout, cancellation);

		if (waker == NULL) {
			return;
		}

		conditions = emalloc(sizeof(future_await_conditions_t));
		conditions->total = (int) zend_hash_num_elements(futures);
		conditions->waiting_count = count > 0 ? count : conditions->total;
		conditions->resolved_count = 0;
		conditions->ignore_errors = ignore_errors;

		ZEND_HASH_FOREACH_KEY_VAL(futures, index, key, current) {
			if (Z_TYPE_P(current) != IS_OBJECT) {
				continue;
			}

			// Resolve the Future object to the FutureState object.
			if (instanceof_function(Z_OBJCE_P(current), async_ce_future_state)) {
				future_state = (async_future_state_t *) Z_OBJ_P(current);
			} else if (instanceof_function(Z_OBJCE_P(current), async_ce_future)) {
				future_state = (async_future_state_t * )((async_future_t *) Z_OBJ_P(current))->future_state;
			} else {
				continue;
			}

			if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE) {
				continue;
			}

			callback = emalloc(sizeof(future_resume_callback_t));
			ZVAL_PTR(&callback->resume_notifier.callback, future_resume_callback);
			callback->conditions = conditions;

			future_state->is_used = true;
			future_state->will_exception_caught = true;

			async_resume_when_ex(waker, &future_state->notifier, false, (async_resume_notifier_t *)callback);

		} ZEND_HASH_FOREACH_END();
	} else {

		waker = async_new_resume_with_timeout(NULL, timeout, cancellation);

		if (waker == NULL) {
			return;
		}

		futures = zend_new_array(8);
		future_state = (async_future_state_t *) async_future_state_new();
		async_resume_when(waker, &future_state->notifier, false, async_resume_when_callback_resolve);

		conditions = emalloc(sizeof(future_await_conditions_t));
		conditions->total = (int) zend_hash_num_elements(futures);
		conditions->waiting_count = count > 0 ? count : conditions->total;
		conditions->resolved_count = 0;
		conditions->ignore_errors = ignore_errors;

		start_concurrent_iterator(future_state, zend_iterator, waker, futures, conditions);
	}

	async_wait(waker);

	if (conditions != NULL) {
		efree(conditions);
		conditions = NULL;
	}

	// Save the results and errors.

	ZEND_HASH_FOREACH_KEY_VAL(futures, index, key, current) {

		if (Z_TYPE_P(current) != IS_OBJECT) {
			continue;
		}

		// Resolve the Future object to the FutureState object.
		if (instanceof_function(Z_OBJCE_P(current), async_ce_future_state)) {
			future_state = (async_future_state_t *) Z_OBJ_P(current);
		} else if (instanceof_function(Z_OBJCE_P(current), async_ce_future)) {
			future_state = (async_future_state_t * )((async_future_t *) Z_OBJ_P(current))->future_state;
		} else {
			continue;
		}

		if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE) {

			if (future_state->throwable != NULL) {
				if (errors != NULL) {
					zval error;
					ZVAL_OBJ(&error, future_state->throwable);

					if (key != NULL) {
						zend_hash_update(errors, key, &error);
					} else {
						zend_hash_index_update(errors, index, &error);
					}

					GC_ADDREF(future_state->throwable);
                }
            } else {
                if (results != NULL) {
                    if (key != NULL) {
                    	zend_hash_update(errors, key, &future_state->result);
                    } else {
                    	zend_hash_index_update(results, index, &future_state->result);
                    }

                	Z_TRY_ADDREF_P(&future_state->result);
                }
			}
		}

	} ZEND_HASH_FOREACH_END();

	OBJ_RELEASE(&waker->std);

	if (zend_iterator != NULL) {
		zend_array_release(futures);
	}
}

void async_api_register(void)
{
	zend_string *module = zend_string_init(PHP_ASYNC_NAME_VERSION, sizeof(PHP_ASYNC_NAME_VERSION) - 1, 0);

	zend_async_scheduler_register(
		module,
		false,
		new_coroutine,
		spawn,
		suspend,
		resume,
		cancel,
		shutdown,
		get_coroutines,
		add_microtask,
		get_awaiting_info
	);

	zend_string_release(module);
}