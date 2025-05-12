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
#include "iterator.h"
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

////////////////////////////////////////////////////////////////////
/// async_await_futures
////////////////////////////////////////////////////////////////////

static zend_always_inline zend_async_event_t * zval_to_event(const zval * current)
{
	// An array element can be either an object implementing
	// the Awaitable interface
	// or an internal structure zend_async_event_t.

	if (Z_TYPE_P(current) == IS_OBJECT
		&& instanceof_function(Z_OBJCE_P(current), async_ce_awaitable)) {
		return ZEND_AWAITABLE_TO_EVENT(Z_OBJ_P(current));
	} else if (Z_TYPE_P(current) == IS_PTR) {
		return (zend_async_event_t *) Z_PTR_P(current);
	} else if (Z_TYPE_P(current) == IS_NULL || Z_TYPE_P(current) == IS_UNDEF) {
		return NULL;
	} else {
		async_throw_error("Expected item to be an Async\\Awaitable object");
		return NULL;
	}
}

void async_waiting_callback(
	zend_async_event_t *event,
	zend_async_event_callback_t *callback,
	void *result,
	zend_object *exception
)
{
	async_await_callback_t * await_callback = (async_await_callback_t *) callback;
	async_await_context_t * await_context = await_callback->await_context;

	await_context->resolved_count++;

	// remove the callback from the event
	// We remove the callback because we treat all events
	// as FUTURE-type objects, where the trigger can be activated only once.
	event->del_callback(event, callback);

	if (await_context->errors != NULL && exception != NULL) {
		const zval *success = NULL;
		zval exception_obj;
		ZVAL_OBJ(&exception_obj, exception);

		if (Z_TYPE(await_callback->key) == IS_STRING) {
			success = zend_hash_update(await_context->errors, Z_STR(await_callback->key), &exception_obj);
		} else if (Z_TYPE(await_callback->key) == IS_LONG) {
			success = zend_hash_index_update(await_context->errors, Z_LVAL(await_callback->key), &exception_obj);
		}

		if (success != NULL) {
			GC_ADDREF(exception);
		}
	}

	if (exception != NULL && false == await_context->ignore_errors) {
		ZEND_ASYNC_RESUME_WITH_ERROR(
			await_callback->callback.coroutine,
			exception,
			false
		);

		return;
	}

	if (await_context->results != NULL && ZEND_ASYNC_EVENT_WILL_ZVAL_RESULT(event) && result != NULL) {

		const zval *success = NULL;

		if (Z_TYPE(await_callback->key) == IS_STRING) {
			success = zend_hash_update(await_context->results, Z_STR(await_callback->key), result);
		} else if (Z_TYPE(await_callback->key) == IS_LONG) {
			success = zend_hash_index_update(await_context->results, Z_LVAL(await_callback->key), result);
		}

		if (success != NULL) {
			zval_add_ref(result);
		}
	}

	if (await_context->resolved_count >= await_context->waiting_count) {
		ZEND_ASYNC_RESUME(
			await_callback->callback.coroutine,
			NULL,
			false
		);
	}
}

zend_result await_iterator_handler(async_iterator_t *iterator, zval *current, zval *key)
{
	async_await_iterator_t * await_iterator = ((async_await_iterator_iterator_t *) iterator)->await_iterator;

	// An array element can be either an object implementing
	// the Awaitable interface
	// or an internal structure zend_async_event_t.

	zend_async_event_t* awaitable = zval_to_event(current);

	if (UNEXPECTED(EG(exception))) {
		return FAILURE;
	}

	if (awaitable == NULL || ZEND_ASYNC_EVENT_IS_CLOSED(awaitable)) {
		return SUCCESS;
	}

	async_await_callback_t * callback = ecalloc(1, sizeof(async_await_callback_t));
	callback->callback.base.callback = async_waiting_callback;
	callback->await_context = await_iterator->await_context;

	ZVAL_COPY(&callback->key, key);

	zend_async_resume_when(await_iterator->waiting_coroutine, awaitable, false, NULL, &callback->callback);

	return SUCCESS;
}

void iterator_coroutine_entry(void)
{
	zend_coroutine_t *coroutine = ZEND_CURRENT_COROUTINE;

	if (UNEXPECTED(coroutine == NULL)) {
		async_throw_error("Cannot run iterator coroutine");
		return;
	}

	async_await_iterator_t * await_iterator = coroutine->extended_data;

	ZEND_ASSERT(await_iterator != NULL && "The async_await_iterator_t should not be NULL");
	if (UNEXPECTED(await_iterator == NULL)) {
		async_throw_error("Cannot run concurrent iterator coroutine");
		return;
	}

	async_await_context_t * await_context = await_iterator->await_context;

	if (UNEXPECTED(await_context == NULL)) {
		return;
	}

	 async_await_iterator_iterator_t * iterator = (async_await_iterator_iterator_t *) async_new_iterator(
		NULL,
		await_iterator->zend_iterator,
		NULL,
		await_iterator_handler,
		await_context->concurrency,
		sizeof(async_await_iterator_iterator_t)
	);

	if (UNEXPECTED(iterator == NULL)) {
		return;
	}

	async_run_iterator(&iterator->iterator);
	efree(iterator);
}

void iterator_coroutine_finish_callback(
	zend_async_event_t *event,
	zend_async_event_callback_t *callback,
	void * result,
	zend_object *exception
)
{
	async_await_iterator_t * iterator = (async_await_iterator_t *)
			((zend_coroutine_event_callback_t*) callback)->coroutine->extended_data;

	if (exception != NULL) {
		// Resume the waiting coroutine with the exception
		ZEND_ASYNC_RESUME_WITH_ERROR(
			iterator->waiting_coroutine,
			exception,
			false
		);
	} else if (iterator->await_context->resolved_count >= iterator->await_context->waiting_count) {
		// If iteration is finished, resume the waiting coroutine
		ZEND_ASYNC_RESUME(iterator->waiting_coroutine);
	}
}

void async_await_iterator_coroutine_dispose(zend_coroutine_t *coroutine)
{
	if (coroutine == NULL || coroutine->extended_data == NULL) {
		return;
	}

	async_await_iterator_t * iterator = (async_await_iterator_t *) coroutine->extended_data;
	coroutine->extended_data = NULL;

	efree(iterator);
}

void async_await_futures(
	zval *iterable,
	int count,
	bool ignore_errors,
	zend_async_event_t *cancellation,
	zend_ulong timeout,
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

	async_await_context_t *await_context = NULL;

	if (UNEXPECTED(futures != NULL && zend_hash_num_elements(futures) == 0)) {
		return;
	}

	zend_coroutine_t *coroutine = ZEND_CURRENT_COROUTINE;

	if (UNEXPECTED(coroutine == NULL)) {
		async_throw_error("Cannot await futures outside of a coroutine");
		return;
	}

	if (UNEXPECTED(zend_async_waker_new_with_timeout(coroutine, timeout, cancellation) == NULL)) {
		return;
	}

	await_context = ecalloc(1, sizeof(async_await_context_t));
	await_context->total = futures != NULL ? (int) zend_hash_num_elements(futures) : 0;
	await_context->waiting_count = count > 0 ? count : await_context->total;
	await_context->resolved_count = 0;
	await_context->ignore_errors = ignore_errors;
	await_context->concurrency = concurrency;

	if (futures != NULL)
	{
		ZEND_HASH_FOREACH_KEY_VAL(futures, index, key, current) {

			// An array element can be either an object implementing
			// the Awaitable interface
			// or an internal structure zend_async_event_t.

			zend_async_event_t* awaitable = zval_to_event(current);

			if (UNEXPECTED(EG(exception))) {
				efree(await_context);
				return;
			}

			if (awaitable == NULL || ZEND_ASYNC_EVENT_IS_CLOSED(awaitable)) {
				continue;
			}

			async_await_callback_t * callback = ecalloc(1, sizeof(async_await_callback_t));
			callback->callback.base.callback = async_waiting_callback;
			callback->await_context = await_context;

			ZEND_ASYNC_EVENT_SET_RESULT_USED(awaitable);
			ZEND_ASYNC_EVENT_SET_EXC_CAUGHT(awaitable);

			if (key != NULL) {
				ZVAL_STR(&callback->key, key);
				zval_add_ref(&callback->key);
			} else {
				ZVAL_LONG(&callback->key, index);
			}

			zend_async_resume_when(
				coroutine, (zend_async_event_t *) awaitable, false, NULL, &callback->callback
			);

		} ZEND_HASH_FOREACH_END();
	} else {

		// To launch the concurrent iterator,
		// we need a separate coroutine because we're needed to suspend the current one.

		// Coroutines associated with concurrent iteration are created in a child Scope,
		// which ensures that all child tasks are stopped if the main task is cancelled.
		zend_async_scope_t * scope = ZEND_ASYNC_NEW_SCOPE(ZEND_CURRENT_ASYNC_SCOPE);

		if (UNEXPECTED(scope == NULL || EG(exception))) {
			efree(await_context);
			return;
		}

		zend_coroutine_t * iterator_coroutine = ZEND_ASYNC_SPAWN_WITH(scope);

		if (UNEXPECTED(iterator_coroutine == NULL || EG(exception))) {
			efree(await_context);
			return;
		}

		await_context->scope = scope;
		iterator_coroutine->internal_entry = iterator_coroutine_entry;

		async_await_iterator_t * iterator = ecalloc(1, sizeof(async_await_iterator_t));
		iterator->zend_iterator = zend_iterator;
		iterator->waiting_coroutine = coroutine;
		iterator->iterator_coroutine = iterator_coroutine;

		iterator_coroutine->extended_data = iterator;
		iterator_coroutine->extended_dispose = async_await_iterator_coroutine_dispose;

		zend_async_resume_when(
			coroutine, iterator_coroutine, false, iterator_coroutine_finish_callback, NULL
		);
	}

	ZEND_ASYNC_SUSPEND();

	// Free the coroutine scope if it was created for the iterator.
	if (await_context->scope != NULL) {
		await_context->scope->dispose(await_context->scope);
		await_context->scope = NULL;
	}

	await_context->callback.base.dispose(&await_context->callback.base, NULL);
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