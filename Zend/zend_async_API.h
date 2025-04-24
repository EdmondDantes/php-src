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
#ifndef ZEND_ASYNC_API_H
#define ZEND_ASYNC_API_H

#include "zend_fibers.h"
#include "zend_globals.h"

/**
 * zend_coroutine_t is a Basic data structure that represents a coroutine in the Zend Engine.
 */
typedef struct _zend_coroutine_t zend_coroutine_t;
typedef struct _zend_async_waker_t zend_async_waker_t;
typedef struct _zend_async_microtask_t zend_async_microtask_t;
typedef struct _zend_async_scope_t zend_async_scope_t;

typedef struct _zend_async_poll_event zend_async_poll_event;
typedef struct _zend_async_socket_event zend_async_socket_event;
typedef struct _zend_async_timer_event zend_async_timer_event;
typedef struct _zend_async_signal_event zend_async_signal_event;
typedef struct _zend_async_filesystem_event zend_async_filesystem_event;

typedef struct _zend_async_process_event zend_async_process_event;
typedef struct _zend_async_thread_event zend_async_thread_event;

typedef struct _zend_async_task zend_async_task;

typedef void (*zend_async_new_coroutine_t)(zend_async_scope_t *scope);
typedef zend_coroutine_t * (*zend_async_spawn_t)(zend_async_scope_t *scope);
typedef void (*zend_async_suspend_t)(zend_coroutine_t *coroutine);
typedef void (*zend_async_resume_t)(zend_coroutine_t *coroutine);
typedef void (*zend_async_cancel_t)(zend_coroutine_t *coroutine, zend_object * error, const bool transfer_error);
typedef void (*zend_async_shutdown_t)();
typedef void (*zend_async_get_coroutines_t)();
typedef void (*zend_async_add_microtask_t)(zend_async_microtask_t *microtask);
typedef zend_array* (*zend_async_get_awaiting_info_t)(zend_coroutine_t * coroutine);

typedef void (*zend_async_reactor_startup_t)();
typedef void (*zend_async_reactor_shutdown_t)();
typedef bool (*zend_async_reactor_execute_t)(bool no_wait);
typedef bool (*zend_async_reactor_loop_alive_t)();
typedef void (*zend_async_add_event_t)();
typedef void (*zend_async_remove_event_t)();

typedef void (*zend_async_new_socket_event_t)();
typedef void (*zend_async_new_poll_event_t)();
typedef void (*zend_async_new_timer_event_t)();
typedef void (*zend_async_new_signal_event_t)();
typedef void (*zend_async_new_process_event_t)();
typedef void (*zend_async_new_thread_event_t)();
typedef void (*zend_async_new_filesystem_event_t)();

typedef void (*zend_async_queue_task_t)(zend_async_task *task);

typedef struct _zend_async_event_t zend_async_event_t;
typedef struct _zend_async_event_callback_t zend_async_event_callback_t;
typedef void (*zend_async_event_callback_fn)(
	zend_async_event_t *event, zend_async_event_callback_t *callback, void * result, zend_object *exception
);
typedef void (*zend_async_event_callback_dispose_fn)(zend_async_event_t *event, zend_async_event_callback_t *callback);
typedef void (*zend_async_event_add_callback_t)(zend_async_event_t *event, zend_async_event_callback_t *callback);
typedef void (*zend_async_event_del_callback_t)(zend_async_event_t *event, zend_async_event_callback_t *callback);
typedef bool (*zend_async_event_is_closed_t)(zend_async_event_t *event);

typedef void (*zend_async_microtask_handler_t)(zend_async_microtask_t *microtask);

struct _zend_async_microtask_t {
	zend_async_microtask_handler_t handler;
	zend_async_microtask_handler_t dtor;
	bool is_cancelled;
	int ref_count;
};

struct _zend_async_event_callback_t {
	zend_async_event_callback_fn callback;
	zend_async_event_callback_dispose_fn dispose;
};

struct _zend_async_event_t {
	zend_async_event_add_callback_t add_callback;
	zend_async_event_del_callback_t del_callback;
	zend_async_event_is_closed_t is_closed;
};

struct _zend_async_poll_event {
	zend_async_event_t event;
};

struct _zend_async_socket_event {
	zend_async_poll_event poll_event;
};

struct _zend_async_timer_event {
	zend_async_event_t event;
};

struct _zend_async_signal_event {
	zend_async_event_t event;
};

struct _zend_async_process_event {
	zend_async_event_t event;
};

struct _zend_async_thread_event {
	zend_async_event_t event;
};

struct _zend_async_filesystem_event {
	zend_async_event_t event;
};

struct _zend_async_task {
	zend_async_event_t event;
};

struct _zend_async_scope_t {

};

typedef void (*zend_async_waker_dtor)(zend_coroutine_t *coroutine);

typedef struct {
	zend_async_event_t *event;
	zend_async_event_callback_t *callback;
} zend_async_waker_trigger_t;

typedef enum {
	ZEND_ASYNC_WAKER_NO_STATUS = 0,
	ZEND_ASYNC_WAKER_WAITING = 1,
	ZEND_ASYNC_WAKER_SUCCESS = 2,
	ZEND_ASYNC_WAKER_ERROR = 3,
	ZEND_ASYNC_WAKER_IGNORED = 4
} ZEND_ASYNC_WAKER_STATUS;

struct _zend_async_waker_t {
	/* The waker status. */
	ZEND_ASYNC_WAKER_STATUS status;
	/* The array of zend_async_waker_trigger_t. */
	HashTable events;
	/* A list of events objects (zend_async_event_t) that occurred during the last iteration of the event loop. */
	HashTable *triggered_events;
	/* Error object. */
	zend_object *error;
	/* Filename of the waker object creation point. */
	zend_string *filename;
	/* Line number of the waker object creation point. */
	uint32_t lineno;
	/* The waker destructor. */
	zend_async_waker_dtor dtor;
};

typedef void (*zend_async_coroutine_dispose)(zend_coroutine_t *coroutine);

struct _zend_coroutine_t {
	/* Callback and info / cache to be used when coroutine is started. */
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	/* Coroutine waker */
	zend_async_waker_t *waker;

	/* Storage for return value. */
	zval result;

	// Dispose handler
	zend_async_coroutine_dispose dispose;
};

#endif //ZEND_ASYNC_API_H

#define ZEND_IS_ASYNC_ON EG(is_async)
#define ZEND_IS_ASYNC_OFF !EG(is_async)
#define ZEND_ASYNC_ON EG(is_async) = true
#define ZEND_IN_SCHEDULER_CONTEXT EG(in_scheduler_context)
#define ZEND_IS_SCHEDULER_CONTEXT EG(in_scheduler_context) == true
#define ZEND_GRACEFUL_SHUTDOWN EG(graceful_shutdown)
#define ZEND_EXIT_EXCEPTION EG(exit_exception)
#define ZEND_CURRENT_COROUTINE EG(coroutine)

BEGIN_EXTERN_C()

ZEND_API bool zend_async_is_enabled(void);
ZEND_API bool zend_scheduler_is_enabled(void);

ZEND_API void zend_async_init(void);
ZEND_API void zend_async_shutdown(void);

/* Scheduler API */

ZEND_API zend_async_spawn_t zend_async_spawn_fn;
ZEND_API zend_async_new_coroutine_t zend_async_new_coroutine_fn;
ZEND_API zend_async_suspend_t zend_async_suspend_fn;
ZEND_API zend_async_resume_t zend_async_resume_fn;
ZEND_API zend_async_cancel_t zend_async_cancel_fn;
ZEND_API zend_async_shutdown_t zend_async_shutdown_fn;
ZEND_API zend_async_get_coroutines_t zend_async_get_coroutines_fn;
ZEND_API zend_async_add_microtask_t zend_async_add_microtask_fn;

/* Reactor API */

ZEND_API bool zend_async_reactor_is_enabled(void);
ZEND_API zend_async_reactor_startup_t zend_async_reactor_startup_fn;
ZEND_API zend_async_reactor_shutdown_t zend_async_reactor_shutdown_fn;
ZEND_API zend_async_reactor_execute_t zend_async_reactor_execute_fn;
ZEND_API zend_async_reactor_loop_alive_t zend_async_reactor_loop_alive_fn;
ZEND_API zend_async_add_event_t zend_async_add_event_fn;
ZEND_API zend_async_remove_event_t zend_async_remove_event_fn;
ZEND_API zend_async_new_socket_event_t zend_async_new_socket_event_fn;
ZEND_API zend_async_new_poll_event_t zend_async_new_poll_event_fn;
ZEND_API zend_async_new_timer_event_t zend_async_new_timer_event_fn;
ZEND_API zend_async_new_signal_event_t zend_async_new_signal_event_fn;
ZEND_API zend_async_new_process_event_t zend_async_new_process_event_fn;
ZEND_API zend_async_new_thread_event_t zend_async_new_thread_event_fn;
ZEND_API zend_async_new_filesystem_event_t zend_async_new_filesystem_event_fn;

/* Thread pool API */
ZEND_API bool zend_async_thread_pool_is_enabled(void);
ZEND_API zend_async_queue_task_t zend_async_queue_task_fn;

ZEND_API void zend_async_scheduler_register(
	zend_async_new_coroutine_t new_coroutine_fn,
    zend_async_spawn_t spawn_fn,
    zend_async_suspend_t suspend_fn,
    zend_async_resume_t resume_fn,
    zend_async_cancel_t cancel_fn,
    zend_async_shutdown_t shutdown_fn,
    zend_async_get_coroutines_t get_coroutines_fn,
    zend_async_add_microtask_t add_microtask_fn,
    zend_async_get_awaiting_info_t get_awaiting_info_fn
);

ZEND_API void zend_async_reactor_register(
	zend_async_reactor_startup_t reactor_startup_fn,
	zend_async_reactor_shutdown_t reactor_shutdown_fn,
	zend_async_reactor_execute_t reactor_execute_fn,
	zend_async_reactor_loop_alive_t reactor_loop_alive_fn,
    zend_async_add_event_t add_event_fn,
    zend_async_remove_event_t remove_event_fn,
    zend_async_new_socket_event_t new_socket_event_fn,
    zend_async_new_poll_event_t new_poll_event_fn,
    zend_async_new_timer_event_t new_timer_event_fn,
    zend_async_new_signal_event_t new_signal_event_fn,
    zend_async_new_process_event_t new_process_event_fn,
    zend_async_new_thread_event_t new_thread_event_fn,
    zend_async_new_filesystem_event_t new_filesystem_event_fn
);

ZEND_API void zend_async_thread_pool_register(zend_async_queue_task_t queue_task_fn);
/* Waker API */
ZEND_API zend_async_waker_t *zend_async_waker_create(zend_coroutine_t *coroutine);
ZEND_API void zend_async_waker_destroy(zend_coroutine_t *coroutine);
ZEND_API void zend_async_waker_add_event(zend_coroutine_t *coroutine, zend_async_event_t *event, zend_async_event_callback_t *callback);
ZEND_API void zend_async_waker_del_event(zend_coroutine_t *coroutine, zend_async_event_t *event);

END_EXTERN_C()

#define ZEND_ASYNC_SPAWN() zend_async_spawn_fn()
#define ZEND_ASYNC_SUSPEND(coroutine) zend_async_suspend_fn(coroutine)
#define ZEND_ASYNC_RESUME(coroutine) zend_async_resume_fn(coroutine)
#define ZEND_ASYNC_CANCEL(coroutine, error, transfer_error) zend_async_cancel_fn(coroutine, error, transfer_error)
#define ZEND_ASYNC_SHUTDOWN() zend_async_shutdown_fn()
#define ZEND_ASYNC_GET_COROUTINES() zend_async_get_coroutines_fn()
#define ZEND_ASYNC_ADD_MICROTASK(microtask) zend_async_add_microtask_fn(microtask)

#define ZEND_ASYNC_REACTOR_STARTUP() zend_async_reactor_startup_fn()
#define ZEND_ASYNC_REACTOR_SHUTDOWN() zend_async_reactor_shutdown_fn()

#define ZEND_ASYNC_REACTOR_EXECUTE(no_wait) zend_async_reactor_execute_fn(no_wait)
#define ZEND_ASYNC_REACTOR_LOOP_ALIVE() zend_async_reactor_loop_alive_fn()

#define ZEND_ASYNC_ADD_EVENT() zend_async_add_event_fn()
#define ZEND_ASYNC_REMOVE_EVENT() zend_async_remove_event_fn()
#define ZEND_ASYNC_NEW_SOCKET_EVENT() zend_async_new_socket_event_fn()
#define ZEND_ASYNC_NEW_POLL_EVENT() zend_async_new_poll_event_fn()
#define ZEND_ASYNC_NEW_TIMER_EVENT() zend_async_new_timer_event_fn()
#define ZEND_ASYNC_NEW_SIGNAL_EVENT() zend_async_new_signal_event_fn()
#define ZEND_ASYNC_NEW_PROCESS_EVENT() zend_async_new_process_event_fn()
#define ZEND_ASYNC_NEW_THREAD_EVENT() zend_async_new_thread_event_fn()
#define ZEND_ASYNC_NEW_FILESYSTEM_EVENT() zend_async_new_filesystem_event_fn()
#define ZEND_ASYNC_QUEUE_TASK(task) zend_async_queue_task_fn(task)
#define ZEND_ASYNC_IS_ENABLED() zend_async_is_enabled()
#define ZEND_ASYNC_REACTOR_IS_ENABLED() zend_async_reactor_is_enabled()
