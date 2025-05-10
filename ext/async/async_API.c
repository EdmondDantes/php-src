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

async_scope_t * new_scope(async_scope_t * parent_scope)
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

	zend_async_waker_t *waker = zend_async_waker_new(&coroutine->coroutine);
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

void resume(zend_coroutine_t *coroutine, zend_object * error, const bool transfer_error)
{
	if (UNEXPECTED(coroutine->waker == NULL)) {
		async_throw_error("Cannot resume a coroutine that has not been suspended");
		return;
	}

	if (error != NULL) {
		if (coroutine->waker->error != NULL) {
			zend_exception_set_previous(error, coroutine->waker->error);
			OBJ_RELEASE(coroutine->waker->error);
			coroutine->waker->error = error;
		}

		if (false == transfer_error) {
			GC_ADDREF(error);
		}
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
		zend_async_waker_new(coroutine);
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

static zend_class_entry* async_get_exception_ce(zend_async_exception_type type)
{
	switch (type) {
		case ZEND_ASYNC_EXCEPTION_CANCELLATION:
			return async_ce_cancellation_exception;
		case ZEND_ASYNC_EXCEPTION_TIMEOUT:
			return async_ce_timeout_exception;
		case ZEND_ASYNC_EXCEPTION_POLL:
			return async_ce_poll_exception;
		default:
			return async_ce_async_exception;
	}
}

void iterator_coroutine_entry(void)
{
	zend_coroutine_t *coroutine = ZEND_CURRENT_COROUTINE;

	if (UNEXPECTED(coroutine == NULL)) {
		async_throw_error("Cannot run iterator coroutine");
		return;
	}

	zend_async_event_t *event = (zend_async_event_t *) coroutine->extended_data;
	async_await_callback_t *await_callback = (async_await_callback_t *) event->extended_data;

	if (UNEXPECTED(EG(exception))) {
		return;
	}

	if (UNEXPECTED(coroutine->waker == NULL)) {
		async_throw_error("Waker is not initialized");
		return;
	}

	if (UNEXPECTED(coroutine->waker->status == ZEND_ASYNC_WAKER_QUEUED)) {
		return;
	}

	if (UNEXPECTED(circular_buffer_push(&ASYNC_G(coroutine_queue), coroutine, true) == FAILURE)) {
		async_throw_error("Failed to enqueue coroutine");
		return;
	}

	coroutine->waker->status = ZEND_ASYNC_WAKER_QUEUED;
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

	zend_async_awaitable_t *awaitable;

	zend_async_waker_t *waker;
	async_await_callback_t *await_callback = NULL;

	if (UNEXPECTED(futures != NULL && zend_hash_num_elements(futures) == 0)) {
		return;
	}

	zend_coroutine_t *coroutine = ZEND_CURRENT_COROUTINE;

	if (UNEXPECTED(coroutine == NULL)) {
		async_throw_error("Cannot await futures outside of a coroutine");
		return;
	}

	waker = zend_async_waker_new_with_timeout(coroutine, timeout, cancellation);

	if (waker == NULL) {
		return;
	}

	await_callback = ecalloc(1, sizeof(async_await_callback_t));
	await_callback->total = futures != NULL ? (int) zend_hash_num_elements(futures) : 0;
	await_callback->waiting_count = count > 0 ? count : await_callback->total;
	await_callback->resolved_count = 0;
	await_callback->ignore_errors = ignore_errors;

	if (futures != NULL)
	{
		ZEND_HASH_FOREACH_KEY_VAL(futures, index, key, current) {

			// An array element can be either an object implementing
			// the Awaitable interface
			// or an internal structure zend_async_event_t.

			if (Z_TYPE_P(current) == IS_OBJECT
				&& instanceof_function(Z_OBJCE_P(current), async_ce_awaitable)) {
				awaitable = (zend_async_awaitable_t *) Z_OBJ_P(current);

				// We mark that the object has been used and will be used for exception handling.
				awaitable->is_used = true;
				awaitable->will_exception_caught = true;
			} else if (Z_TYPE_P(current) == IS_PTR) {
				awaitable = (zend_async_awaitable_t *) Z_PTR_P(current);
			} else if (Z_TYPE_P(current) == IS_NULL || Z_TYPE_P(current) == IS_UNDEF) {
				continue;
			} else {
				async_throw_error("Expected parameter to be an Async\\Awaitable object");
				continue;
			}

			if (((zend_async_event_t * )awaitable)->is_closed) {
				continue;
			}

			zend_async_resume_when(
				coroutine, (zend_async_event_t *) awaitable, false, NULL, &await_callback->callback
			);

		} ZEND_HASH_FOREACH_END();
	} else {

		zend_async_event_t * iterator_finished_event = ecalloc(1, sizeof(zend_async_event_t));
		zend_async_resume_when(coroutine, iterator_finished_event, false, zend_async_waker_callback_resolve, NULL);

		// To launch the concurrent iterator,
		// we need a separate coroutine because we're needed to suspend the current one.

		zend_coroutine_t * iterator_coroutine = ZEND_ASYNC_SPAWN_WITH(NULL);

		if (UNEXPECTED(iterator_coroutine == NULL || EG(exception))) {
			return;
		}

		iterator_coroutine->internal_entry = iterator_coroutine_entry;

		async_await_iterator_t * iterator = ecalloc(1, sizeof(async_await_iterator_t));
		iterator->iterator = zend_iterator;
		iterator->waiting_coroutine = coroutine;
		iterator->iterator_finished_event = iterator_finished_event;

		iterator_coroutine->extended_data = iterator;
		iterator_coroutine->extended_dispose = async_await_iterator_coroutine_dispose;
	}

	ZEND_ASYNC_SUSPEND();

	await_callback->callback.base.dispose(&await_callback->callback.base, NULL);

	// Save the results and errors.

	ZEND_HASH_FOREACH_KEY_VAL(futures, index, key, current) {

		if (Z_TYPE_P(current) != IS_OBJECT) {
			continue;
		}

		// Resolve the Future object to the FutureState object.
		if (instanceof_function(Z_OBJCE_P(current), async_ce_future_state)) {
			awaitable = (async_future_state_t *) Z_OBJ_P(current);
		} else if (instanceof_function(Z_OBJCE_P(current), async_ce_future)) {
			awaitable = (async_future_state_t * )((async_future_t *) Z_OBJ_P(current))->future_state;
		} else {
			continue;
		}

		if (Z_TYPE(awaitable->notifier.is_closed) == IS_TRUE) {

			if (awaitable->throwable != NULL) {
				if (errors != NULL) {
					zval error;
					ZVAL_OBJ(&error, awaitable->throwable);

					if (key != NULL) {
						zend_hash_update(errors, key, &error);
					} else {
						zend_hash_index_update(errors, index, &error);
					}

					GC_ADDREF(awaitable->throwable);
                }
            } else {
                if (results != NULL) {
                    if (key != NULL) {
                    	zend_hash_update(errors, key, &awaitable->result);
                    } else {
                    	zend_hash_index_update(results, index, &awaitable->result);
                    }

                	Z_TRY_ADDREF_P(&awaitable->result);
                }
			}
		}

	} ZEND_HASH_FOREACH_END();

	waker->dtor(coroutine);

	if (futures != NULL) {
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
		new_scope,
		spawn,
		suspend,
		resume,
		cancel,
		shutdown,
		get_coroutines,
		add_microtask,
		get_awaiting_info,
		async_get_exception_ce
	);

	zend_string_release(module);
}