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

#include "zend_coroutine.h"

typedef struct _zend_async_api zend_async_api;
typedef struct _zend_async_scope zend_async_scope;

typedef struct _zend_async_poll_event zend_async_poll_event;
typedef struct _zend_async_socket_event zend_async_socket_event;
typedef struct _zend_async_timer_event zend_async_timer_event;
typedef struct _zend_async_signal_event zend_async_signal_event;
typedef struct _zend_async_filesystem_event zend_async_filesystem_event;

typedef struct _zend_async_process_event zend_async_process_event;
typedef struct _zend_async_thread_event zend_async_thread_event;

typedef struct _zend_async_task zend_async_task;

typedef void (*zend_async_spawn_t)();
typedef void (*zend_async_suspend_t)(zend_coroutine *coroutine);
typedef void (*zend_async_resume_t)(zend_coroutine *coroutine);
typedef void (*zend_async_cancel_t)(zend_coroutine *coroutine);
typedef void (*zend_async_shutdown_t)();
typedef void (*zend_async_get_coroutines_t)();

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

typedef struct _zend_async_event zend_async_event;
typedef struct _zend_async_event_callback_t zend_async_event_callback_t;
typedef void (*zend_async_event_callback_fn)(zend_async_event *event, zend_async_event_callback_t *callback, zend_object *exception);
typedef void (*zend_async_event_callback_dispose_fn)(zend_async_event *event, zend_async_event_callback_t *callback);
typedef void (*zend_async_event_add_callback_t)(zend_async_event *event, zend_async_event_callback_t callback);
typedef void (*zend_async_event_del_callback_t)(zend_async_event *event, zend_async_event_callback_t callback);
typedef bool (*zend_async_event_is_closed_t)(zend_async_event *event);

struct _zend_async_event_callback_t {
	zend_async_event_callback_fn callback;
	zend_async_event_callback_dispose_fn dispose;
};

struct _zend_async_event {
	zend_async_event_add_callback_t add_callback;
	zend_async_event_del_callback_t del_callback;
	zend_async_event_is_closed_t is_closed;
};

struct _zend_async_poll_event {
	zend_async_event event;
};

struct _zend_async_socket_event {
	zend_async_poll_event poll_event;
};

struct _zend_async_timer_event {
	zend_async_event event;
};

struct _zend_async_signal_event {
	zend_async_event event;
};

struct _zend_async_process_event {
	zend_async_event event;
};

struct _zend_async_thread_event {
	zend_async_event event;
};

struct _zend_async_filesystem_event {
	zend_async_event event;
};

struct _zend_async_task {
	zend_async_event event;
};

struct _zend_async_scope {

};

#endif //ZEND_ASYNC_API_H

BEGIN_EXTERN_C()

ZEND_API bool zend_async_is_enabled(void);

ZEND_API void zend_async_init(void);
ZEND_API void zend_async_shutdown(void);

/* Scheduler API */

ZEND_API zend_async_spawn_t zend_async_spawn_fn;
ZEND_API zend_async_suspend_t zend_async_suspend_fn;
ZEND_API zend_async_resume_t zend_async_resume_fn;
ZEND_API zend_async_cancel_t zend_async_cancel_fn;
ZEND_API zend_async_shutdown_t zend_async_shutdown_fn;
ZEND_API zend_async_get_coroutines_t zend_async_get_coroutines_fn;

/* Reactor API */

ZEND_API bool zend_async_reactor_is_enabled(void);
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

ZEND_API zend_async_queue_task_t zend_async_queue_task_fn;

END_EXTERN_C()

#define ZEND_ASYNC_SPAWN() zend_async_spawn_fn()
#define ZEND_ASYNC_SUSPEND(coroutine) zend_async_suspend_fn(coroutine)
#define ZEND_ASYNC_RESUME(coroutine) zend_async_resume_fn(coroutine)
#define ZEND_ASYNC_CANCEL(coroutine) zend_async_cancel_fn(coroutine)
#define ZEND_ASYNC_SHUTDOWN() zend_async_shutdown_fn()
#define ZEND_ASYNC_GET_COROUTINES() zend_async_get_coroutines_fn()
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
