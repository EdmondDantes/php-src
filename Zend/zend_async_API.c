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

#define ASYNC_THROW_ERROR(error) zend_throw_error(NULL, error);

static zend_coroutine_t * spawn(zend_async_scope_t *scope)
{
	ASYNC_THROW_ERROR("Async API is not enabled");
	return NULL;
}

static void suspend(zend_coroutine_t *coroutine) {}

static zend_string * scheduler_module_name = NULL;
zend_async_spawn_t zend_async_spawn_fn = spawn;
zend_async_suspend_t zend_async_suspend_fn = suspend;
zend_async_resume_t zend_async_resume_fn = NULL;
zend_async_cancel_t zend_async_cancel_fn = NULL;
zend_async_shutdown_t zend_async_shutdown_fn = NULL;
zend_async_get_coroutines_t zend_async_get_coroutines_fn = NULL;
zend_async_add_microtask_t zend_async_add_microtask_fn = NULL;
zend_async_get_awaiting_info_t zend_async_get_awaiting_info_fn = NULL;

static zend_string * reactor_module_name = NULL;
zend_async_reactor_startup_t zend_async_reactor_startup_fn = NULL;
zend_async_reactor_shutdown_t zend_async_reactor_shutdown_fn = NULL;
zend_async_reactor_execute_t zend_async_reactor_execute_fn = NULL;
zend_async_reactor_loop_alive_t zend_async_reactor_loop_alive_fn = NULL;
zend_async_new_socket_event_t zend_async_new_socket_event_fn = NULL;
zend_async_new_poll_event_t zend_async_new_poll_event_fn = NULL;
zend_async_new_timer_event_t zend_async_new_timer_event_fn = NULL;
zend_async_new_signal_event_t zend_async_new_signal_event_fn = NULL;
zend_async_new_process_event_t zend_async_new_process_event_fn = NULL;
zend_async_new_thread_event_t zend_async_new_thread_event_fn = NULL;
zend_async_new_filesystem_event_t zend_async_new_filesystem_event_fn = NULL;

zend_async_getnameinfo_t zend_async_getnameinfo_fn = NULL;
zend_async_getaddrinfo_t zend_async_getaddrinfo_fn = NULL;

static zend_string * thread_pool_module_name = NULL;
zend_async_queue_task_t zend_async_queue_task_fn = NULL;

ZEND_API bool zend_async_is_enabled(void)
{
	return EG(is_async);
}

ZEND_API bool zend_scheduler_is_enabled(void)
{
	return zend_async_spawn_fn != NULL && zend_async_spawn_fn != spawn;
}

ZEND_API bool zend_async_reactor_is_enabled(void)
{
	return zend_async_reactor_startup_fn != NULL;
}

ZEND_API void zend_async_init(void)
{
	// Initialization code for async API
}

ZEND_API void zend_async_shutdown(void)
{
	// Shutdown code for async API
}

ZEND_API void zend_async_scheduler_register(
	zend_string *module,
	bool allow_override,
	zend_async_new_coroutine_t new_coroutine_fn,
    zend_async_spawn_t spawn_fn,
    zend_async_suspend_t suspend_fn,
    zend_async_resume_t resume_fn,
    zend_async_cancel_t cancel_fn,
    zend_async_shutdown_t shutdown_fn,
    zend_async_get_coroutines_t get_coroutines_fn,
    zend_async_add_microtask_t add_microtask_fn,
    zend_async_get_awaiting_info_t get_awaiting_info_fn
)
{
	if (scheduler_module_name != NULL && false == allow_override) {
		zend_error(
			E_CORE_ERROR, "The module %s is trying to override Scheduler, which was registered by the module %s.",
			ZSTR_VAL(module), ZSTR_VAL(scheduler_module_name)
		);
		return;
	}

	if (scheduler_module_name != NULL) {
		zend_string_release(scheduler_module_name);
	}

	scheduler_module_name = zend_string_copy(module);

	zend_async_new_coroutine_fn = new_coroutine_fn;
    zend_async_spawn_fn = spawn_fn;
    zend_async_suspend_fn = suspend_fn;
    zend_async_resume_fn = resume_fn;
    zend_async_cancel_fn = cancel_fn;
    zend_async_shutdown_fn = shutdown_fn;
    zend_async_get_coroutines_fn = get_coroutines_fn;
    zend_async_add_microtask_fn = add_microtask_fn;
	zend_async_get_awaiting_info_fn = get_awaiting_info_fn;
}

ZEND_API void zend_async_reactor_register(
	zend_string *module,
	bool allow_override,
	zend_async_reactor_startup_t reactor_startup_fn,
	zend_async_reactor_shutdown_t reactor_shutdown_fn,
	zend_async_reactor_execute_t reactor_execute_fn,
	zend_async_reactor_loop_alive_t reactor_loop_alive_fn,
    zend_async_new_socket_event_t new_socket_event_fn,
    zend_async_new_poll_event_t new_poll_event_fn,
    zend_async_new_timer_event_t new_timer_event_fn,
    zend_async_new_signal_event_t new_signal_event_fn,
    zend_async_new_process_event_t new_process_event_fn,
    zend_async_new_thread_event_t new_thread_event_fn,
    zend_async_new_filesystem_event_t new_filesystem_event_fn,
    zend_async_getnameinfo_t getnameinfo_fn,
    zend_async_getaddrinfo_t getaddrinfo_fn
)
{
	if (reactor_module_name != NULL && false == allow_override) {
		zend_error(
			E_CORE_ERROR, "The module %s is trying to override Reactor, which was registered by the module %s.",
			ZSTR_VAL(module), ZSTR_VAL(reactor_module_name)
		);
		return;
	}

	if (reactor_module_name != NULL) {
		zend_string_release(reactor_module_name);
	}

	reactor_module_name = zend_string_copy(module);

	zend_async_reactor_startup_fn = reactor_startup_fn;
	zend_async_reactor_shutdown_fn = reactor_shutdown_fn;
	zend_async_reactor_execute_fn = reactor_execute_fn;
	zend_async_reactor_loop_alive_fn = reactor_loop_alive_fn;

    zend_async_new_socket_event_fn = new_socket_event_fn;
    zend_async_new_poll_event_fn = new_poll_event_fn;
    zend_async_new_timer_event_fn = new_timer_event_fn;
    zend_async_new_signal_event_fn = new_signal_event_fn;

	zend_async_new_process_event_fn = new_process_event_fn;
    zend_async_new_thread_event_fn = new_thread_event_fn;
    zend_async_new_filesystem_event_fn = new_filesystem_event_fn;

	zend_async_getnameinfo_fn = getnameinfo_fn;
	zend_async_getaddrinfo_fn = getaddrinfo_fn;
}

ZEND_API void zend_async_thread_pool_register(zend_string *module, bool allow_override, zend_async_queue_task_t queue_task_fn)
{
	if (thread_pool_module_name != NULL && false == allow_override) {
		zend_error(
			E_CORE_ERROR, "The module %s is trying to override Thread Pool, which was registered by the module %s.",
			ZSTR_VAL(module), ZSTR_VAL(thread_pool_module_name)
		);
	}

	if (thread_pool_module_name != NULL) {
		zend_string_release(thread_pool_module_name);
	}

	thread_pool_module_name = zend_string_copy(module);
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

ZEND_API zend_async_waker_t *zend_async_waker_create(zend_coroutine_t *coroutine)
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

ZEND_API void zend_async_waker_destroy(zend_coroutine_t *coroutine)
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

ZEND_API void zend_async_waker_add_event(zend_coroutine_t *coroutine, zend_async_event_t *event, zend_async_event_callback_t *callback)
{
	if (UNEXPECTED(coroutine->waker == NULL)) {
		return;
	}

	event->add_callback(event, callback);

	if (UNEXPECTED(EG(exception) != NULL)) {
		return;
	}

	zval zval_callback;
	ZVAL_PTR(&zval_callback, callback);

	if (zend_hash_next_index_insert(&coroutine->waker->events, &zval_callback) == NULL) {
		ASYNC_THROW_ERROR("Failed to add event to waker");
		return;
	}
}

ZEND_API void zend_async_waker_del_event(zend_coroutine_t *coroutine, zend_async_event_t *event)
{
	if (UNEXPECTED(coroutine->waker == NULL)) {
		return;
	}

	zval *item;
	zend_ulong index;

	ZEND_HASH_FOREACH_NUM_KEY_VAL(&coroutine->waker->events, index, item)
	{
		const zend_async_waker_trigger_t * waker_trigger = Z_PTR_P(item);

		if (waker_trigger->event == event) {
			zend_hash_index_del(&coroutine->waker->events, index);
			break;
		}
	}
	ZEND_HASH_FOREACH_END();
}

//////////////////////////////////////////////////////////////////////
/* Waker API end */
//////////////////////////////////////////////////////////////////////
