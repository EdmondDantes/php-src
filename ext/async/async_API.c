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
#include "scope.h"
#include "zend_common.h"

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

	waker->status = ZEND_ASYNC_WAKER_SUCCESS;

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

void suspend(zend_coroutine_t *coroutine)
{
}

void resume(zend_coroutine_t *coroutine)
{
}

void cancel(zend_coroutine_t *coroutine, zend_object *error, const bool transfer_error)
{
}

void shutdown(void)
{

}

void get_coroutines(void)
{

}

void add_microtask(zend_async_microtask_t *microtask)
{

}

zend_array *get_awaiting_info(zend_coroutine_t *coroutine)
{
	return NULL;
}

void async_api_register(void)
{
	zend_async_scheduler_register(
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
}