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
#include "libuv_event_loop.h"

#include <zend_exceptions.h>
#include "../async.h"
#include "../scheduler.h"
#include "../php_layer/exceptions.h"

static zend_always_inline int libuv_events_from_php(const zend_long events)
{
	int internal_events = 0;

	if (events & ASYNC_READABLE) {
		internal_events |= UV_READABLE;
	}

	if (events & ASYNC_WRITABLE) {
		internal_events |= UV_WRITABLE;
	}

	if (events & ASYNC_DISCONNECT) {
		internal_events |= UV_DISCONNECT;
	}

	if (events & ASYNC_PRIORITIZED) {
		internal_events |= UV_PRIORITIZED;
	}

	return internal_events;
}

static zend_always_inline zend_long libuv_events_to_php(const int events)
{
	zend_long php_events = 0;

	if (events & UV_READABLE) {
		php_events |= ASYNC_READABLE;
	}

	if (events & UV_WRITABLE) {
		php_events |= ASYNC_WRITABLE;
	}

	if (events & UV_DISCONNECT) {
		php_events |= ASYNC_DISCONNECT;
	}

	if (events & UV_PRIORITIZED) {
		php_events |= ASYNC_PRIORITIZED;
	}

	return php_events;
}

static void on_poll_event(const uv_poll_t* handle, const int status, const int events)
{
	libuv_poll_t *poll_handle = handle->data;
	zval php_events;
	ZVAL_LONG(&php_events, libuv_events_to_php(events));

	zval error;
	ZVAL_NULL(&error);

	if (status < 0) {
		zend_object *exception = async_new_exception(
			async_ce_input_output_exception, "Input output error: %s", uv_strerror(status)
		);

		ZVAL_OBJ(&error, exception);
	}

	async_notifier_notify(&poll_handle->handle, &php_events, &error);

	// TODO: handle error
}

static void libuv_poll_dtor(async_ev_handle_t *handle)
{
	libuv_poll_t *poll = (libuv_poll_t *)handle;
	uv_close((uv_handle_t *)&poll->uv_handle, NULL);
}

libuv_poll_t *libuv_poll_ctor()
{
	libuv_poll_t *poll = pecalloc(1, sizeof(libuv_poll_t), 1);
	poll->handle.dtor = libuv_poll_dtor;
	poll->uv_handle.data = poll;

	return poll;
}

static libuv_poll_t* libuv_poll_new(const int fd, const ASYNC_HANDLE_TYPE type, const zend_long events)
{
	libuv_poll_t *poll_handle = libuv_poll_ctor();

	if (poll_handle == NULL) {
		return NULL;
	}

	poll_handle->handle.type = type;

	int res = uv_poll_init(&ASYNC_G(uv_loop), &poll_handle->uv_handle, fd);

	if (res < 0) {
		async_throw_poll("Failed to initialize poll handle: %s", uv_strerror(res));
		pefree(poll_handle, 1);
		return NULL;
	}

	res = uv_poll_start(&poll_handle->uv_handle, libuv_events_from_php(events), on_poll_event);

	if (res < 0) {
		async_throw_poll("Failed to start poll handle: %s", uv_strerror(res));
		uv_close((uv_handle_t*)poll_handle, NULL);
		pefree(poll_handle, 1);
		return NULL;
	}

	return poll_handle;
}

static void handle_callbacks(void)
{
	uv_run(&ASYNC_G(uv_loop), UV_RUN_ONCE);
}

void async_libuv_startup(void)
{
	async_scheduler_set_callback_handler(handle_callbacks);
}

void async_libuv_shutdown(void)
{
	async_scheduler_set_callback_handler(NULL);
}