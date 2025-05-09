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
#include "zend_exceptions.h"

#define ASYNC_THROW_ERROR(error) zend_throw_error(NULL, error);

static zend_coroutine_t * spawn(zend_async_scope_t *scope)
{
	ASYNC_THROW_ERROR("Async API is not enabled");
	return NULL;
}

static void suspend(zend_coroutine_t *coroutine) {}

static zend_class_entry * get_exception_ce(zend_async_exception_type type)
{
	return zend_ce_exception;
}

static zend_string * scheduler_module_name = NULL;
zend_async_spawn_t zend_async_spawn_fn = spawn;
zend_async_suspend_t zend_async_suspend_fn = suspend;
zend_async_resume_t zend_async_resume_fn = NULL;
zend_async_cancel_t zend_async_cancel_fn = NULL;
zend_async_shutdown_t zend_async_shutdown_fn = NULL;
zend_async_get_coroutines_t zend_async_get_coroutines_fn = NULL;
zend_async_add_microtask_t zend_async_add_microtask_fn = NULL;
zend_async_get_awaiting_info_t zend_async_get_awaiting_info_fn = NULL;
zend_async_get_exception_ce_t zend_async_get_exception_ce_fn = get_exception_ce;

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

zend_async_exec_t zend_async_exec_fn = NULL;

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
    zend_async_get_awaiting_info_t get_awaiting_info_fn,
    zend_async_get_exception_ce_t get_exception_ce_fn
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
	zend_async_get_exception_ce_fn = get_exception_ce_fn;
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
    zend_async_getaddrinfo_t getaddrinfo_fn,
    zend_async_new_exec_event_t new_exec_event_fn,
    zend_async_exec_t exec_fn
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

	zend_async_new_exec_event_fn = new_exec_event_fn;
	zend_async_exec_fn = exec_fn;
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

static void event_callback_dispose(zend_async_event_callback_t *callback)
{
	if (callback->ref_count != 0) {
		return;
	}

	efree(callback);
}

ZEND_API void zend_async_resume_when(
		zend_coroutine_t			*coroutine,
		zend_async_event_t			*event,
		const bool					trans_event,
		zend_async_event_callback_fn callback,
		zend_coroutine_event_callback_t *event_callback
	)
{
	if (UNEXPECTED(Z_TYPE(event->is_closed) == IS_TRUE)) {
		zend_throw_error(NULL, "The event cannot be used after it has been terminated");
		return;
	}

	if (UNEXPECTED(callback == NULL && event_callback == NULL)) {
		zend_error(E_WARNING, "Callback cannot be NULL");

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
			zend_throw_error(NULL, "Failed to add event to the waker");
			return;
		}
	}
}

ZEND_API void zend_async_resume_when_callback_resolve(
	zend_async_event_t *event, zend_async_event_callback_t *callback, void * result, zend_object *exception
)
{
	zend_coroutine_t * coroutine = ((zend_coroutine_event_callback_t *) callback)->coroutine;

	if (exception == NULL && coroutine->waker != NULL) {
		zend_hash_index_add_ptr(coroutine->waker->triggered_events, (zend_ulong)event, event);
	}

	ZEND_ASYNC_RESUME_WITH_ERROR(coroutine, exception, false);
}

ZEND_API void zend_async_resume_when_callback_cancel(
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

ZEND_API void zend_async_resume_when_callback_timeout(
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


//////////////////////////////////////////////////////////////////////
/* Waker API end */
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/* Exception API */
//////////////////////////////////////////////////////////////////////

ZEND_API ZEND_COLD zend_object * zend_async_throw(zend_async_exception_type type, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	zend_string *message = zend_vstrpprintf(0, format, args);
	va_end(args);

	zend_object *obj = zend_throw_exception(ZEND_ASYNC_GET_EXCEPTION_CE(type), ZSTR_VAL(message), 0);
	zend_string_release(message);
	return obj;
}

ZEND_API ZEND_COLD zend_object * zend_async_throw_cancellation(const char *format, ...)
{
	const zend_object *previous = EG(exception);

	if (format == NULL
		&& previous != NULL
		&& instanceof_function(previous->ce, ZEND_ASYNC_GET_EXCEPTION_CE(ZEND_ASYNC_EXCEPTION_TIMEOUT))) {
			format = "The operation was canceled by timeout";
		} else {
			format = format ? format : "The operation was canceled";
		}

	va_list args;
	va_start(args, format);

	zend_object *obj = zend_throw_exception_ex(
		ZEND_ASYNC_GET_EXCEPTION_CE(ZEND_ASYNC_EXCEPTION_CANCELLATION), 0, format, args
	);

	va_end(args);
	return obj;
}

ZEND_API ZEND_COLD zend_object * zend_async_throw_timeout(const char *format, const zend_long timeout)
{
	format = format ? format : "A timeout of %u microseconds occurred";

	return zend_throw_exception_ex(ZEND_ASYNC_GET_EXCEPTION_CE(ZEND_ASYNC_EXCEPTION_TIMEOUT), 0, format, timeout);
}

//////////////////////////////////////////////////////////////////////
/* Exception API end */
//////////////////////////////////////////////////////////////////////
