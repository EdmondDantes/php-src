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
#include "zend_async_API.h"

zend_async_spawn_t zend_async_spawn_fn = NULL;
zend_async_suspend_t zend_async_suspend_fn = NULL;
zend_async_resume_t zend_async_resume_fn = NULL;
zend_async_cancel_t zend_async_cancel_fn = NULL;
zend_async_shutdown_t zend_async_shutdown_fn = NULL;
zend_async_get_coroutines_t zend_async_get_coroutines_fn = NULL;
zend_async_add_microtask_t zend_async_add_microtask_fn = NULL;

zend_async_add_event_t zend_async_add_event_fn = NULL;
zend_async_remove_event_t zend_async_remove_event_fn = NULL;
zend_async_new_socket_event_t zend_async_new_socket_event_fn = NULL;
zend_async_new_poll_event_t zend_async_new_poll_event_fn = NULL;
zend_async_new_timer_event_t zend_async_new_timer_event_fn = NULL;
zend_async_new_signal_event_t zend_async_new_signal_event_fn = NULL;
zend_async_new_process_event_t zend_async_new_process_event_fn = NULL;
zend_async_new_thread_event_t zend_async_new_thread_event_fn = NULL;
zend_async_new_filesystem_event_t zend_async_new_filesystem_event_fn = NULL;

zend_async_queue_task_t zend_async_queue_task_fn = NULL;

bool zend_async_is_enabled(void)
{
	return zend_async_spawn_fn != NULL;
}

bool zend_async_reactor_is_enabled(void)
{
	return zend_async_add_event_fn != NULL;
}

void zend_async_init(void)
{
	// Initialization code for async API
}

void zend_async_shutdown(void)
{
	// Shutdown code for async API
}

ZEND_API void zend_async_scheduler_register(
    zend_async_spawn_t spawn_fn,
    zend_async_suspend_t suspend_fn,
    zend_async_resume_t resume_fn,
    zend_async_cancel_t cancel_fn,
    zend_async_shutdown_t shutdown_fn,
    zend_async_get_coroutines_t get_coroutines_fn,
    zend_async_add_microtask_t add_microtask_fn
)
{
    zend_async_spawn_fn = spawn_fn;
    zend_async_suspend_fn = suspend_fn;
    zend_async_resume_fn = resume_fn;
    zend_async_cancel_fn = cancel_fn;
    zend_async_shutdown_fn = shutdown_fn;
    zend_async_get_coroutines_fn = get_coroutines_fn;
    zend_async_add_microtask_fn = add_microtask_fn;
}

ZEND_API void zend_async_reactor_register(
    zend_async_add_event_t add_event_fn,
    zend_async_remove_event_t remove_event_fn,
    zend_async_new_socket_event_t new_socket_event_fn,
    zend_async_new_poll_event_t new_poll_event_fn,
    zend_async_new_timer_event_t new_timer_event_fn,
    zend_async_new_signal_event_t new_signal_event_fn,
    zend_async_new_process_event_t new_process_event_fn,
    zend_async_new_thread_event_t new_thread_event_fn,
    zend_async_new_filesystem_event_t new_filesystem_event_fn
)
{
    zend_async_add_event_fn = add_event_fn;
    zend_async_remove_event_fn = remove_event_fn;
    zend_async_new_socket_event_fn = new_socket_event_fn;
    zend_async_new_poll_event_fn = new_poll_event_fn;
    zend_async_new_timer_event_fn = new_timer_event_fn;
    zend_async_new_signal_event_fn = new_signal_event_fn;
    zend_async_new_process_event_fn = new_process_event_fn;
    zend_async_new_thread_event_fn = new_thread_event_fn;
    zend_async_new_filesystem_event_fn = new_filesystem_event_fn;
}

ZEND_API void zend_async_thread_pool_register(zend_async_queue_task_t queue_task_fn)
{
    zend_async_queue_task_fn = queue_task_fn;
}

//////////////////////////////////////////////////////////////////////
/* Waker API */
//////////////////////////////////////////////////////////////////////

static void waker_events_dtor(zval *item)
{
	zend_async_waker_trigger_t * waker_trigger = Z_PTR_P(item);

	waker_trigger->event->del_callback(waker_trigger->event, waker_trigger->callback);
	efree(waker_trigger);
}

ZEND_API zend_async_waker_t *zend_async_waker_create(zend_async_coroutine_t *coroutine)
{
	if (UNEXPECTED(coroutine->waker != NULL)) {
		return coroutine->waker;
	}

	zend_async_waker_t *waker = pecalloc(1, sizeof(zend_async_waker_t), 0);

	waker->triggered_events = NULL;
	waker->error = NULL;
	waker->dtor = NULL;

	zend_hash_init(&waker->events, 2, NULL, waker_events_dtor, 0);

	coroutine->waker = waker;

	return waker;
}

ZEND_API void zend_async_waker_destroy(zend_async_coroutine_t *coroutine)
{
	if (UNEXPECTED(coroutine->waker == NULL)) {
		return;
	}

	if (coroutine->waker->dtor != NULL) {
		coroutine->waker->dtor(coroutine);
	}

	zend_async_waker_t * waker = coroutine->waker;

	// default dtor
	if (waker->error != NULL) {
		zend_object_release(waker->error);
		waker->error = NULL;
	}

	if (waker->triggered_events != NULL) {
		zend_array_release(waker->triggered_events);
		waker->triggered_events = NULL;
	}

	if (waker->filename != NULL) {
		zend_string_release(waker->filename);
		waker->filename = NULL;
		waker->lineno = 0;
	}

	zend_hash_destroy(&waker->events);
}

ZEND_API void zend_async_waker_add_event(zend_async_coroutine_t *coroutine, zend_async_event_t *event, zend_async_waker_callback_t *callback)
{
	if (UNEXPECTED(coroutine->waker == NULL)) {
		return;
	}

}

ZEND_API void zend_async_waker_del_event(zend_async_coroutine_t *coroutine, zend_async_event_t *event)
{
	if (UNEXPECTED(coroutine->waker == NULL)) {
		return;
	}

}
