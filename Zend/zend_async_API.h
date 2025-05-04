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

/* Reactor Poll API */
typedef enum {
	ASYNC_READABLE = 1,
	ASYNC_WRITABLE = 2,
	ASYNC_DISCONNECT = 4,
	ASYNC_PRIORITIZED = 8
} async_poll_event;

//
// Definitions compatibles with proc_open()
//
#ifdef PHP_WIN32
typedef HANDLE zend_file_descriptor_t;
typedef DWORD zend_process_id_t;
typedef HANDLE zend_process_t;
typedef SOCKET zend_socket_t;
#else
typedef int zend_file_descriptor_t;
typedef pid_t zend_process_id_t;
typedef pid_t zend_process_t;
typedef int zend_socket_t;
#endif

/**
 * zend_coroutine_t is a Basic data structure that represents a coroutine in the Zend Engine.
 */
typedef struct _zend_coroutine_t zend_coroutine_t;
typedef struct _zend_async_waker_t zend_async_waker_t;
typedef struct _zend_async_microtask_t zend_async_microtask_t;
typedef struct _zend_async_scope_t zend_async_scope_t;
typedef struct _zend_fcall_t zend_fcall_t;
typedef void (*zend_coroutine_internal_t)(void);

typedef struct _zend_async_event_t zend_async_event_t;
typedef struct _zend_async_event_callback_t zend_async_event_callback_t;
typedef void (*zend_async_event_callback_fn)(
	zend_async_event_t *event, zend_async_event_callback_t *callback, void * result, zend_object *exception
);
typedef void (*zend_async_event_callback_dispose_fn)(zend_async_event_t *event, zend_async_event_callback_t *callback);
typedef void (*zend_async_event_add_callback_t)(zend_async_event_t *event, zend_async_event_callback_t *callback);
typedef void (*zend_async_event_del_callback_t)(zend_async_event_t *event, zend_async_event_callback_t *callback);
typedef void (*zend_async_event_start_t) (zend_async_event_t *event);
typedef void (*zend_async_event_stop_t) (zend_async_event_t *event);
typedef void (*zend_async_event_dispose_t) (zend_async_event_t *event);

typedef struct _zend_async_poll_event_t zend_async_poll_event_t;
typedef struct _zend_async_timer_event_t zend_async_timer_event_t;
typedef struct _zend_async_signal_event_t zend_async_signal_event_t;
typedef struct _zend_async_filesystem_event_t zend_async_filesystem_event_t;

typedef struct _zend_async_process_event_t zend_async_process_event_t;
typedef struct _zend_async_thread_event_t zend_async_thread_event_t;

typedef struct _zend_async_dns_nameinfo_t zend_async_dns_nameinfo_t;
typedef struct _zend_async_dns_addrinfo_t zend_async_dns_addrinfo_t;

typedef struct _zend_async_task_t zend_async_task_t;

typedef zend_coroutine_t * (*zend_async_new_coroutine_t)(zend_async_scope_t *scope);
typedef zend_coroutine_t * (*zend_async_spawn_t)(zend_async_scope_t *scope);
typedef void (*zend_async_suspend_t)(zend_coroutine_t *coroutine);
typedef void (*zend_async_resume_t)(zend_coroutine_t *coroutine);
typedef void (*zend_async_cancel_t)(zend_coroutine_t *coroutine, zend_object * error, const bool transfer_error);
typedef void (*zend_async_shutdown_t)();
typedef zend_array* (*zend_async_get_coroutines_t)();
typedef void (*zend_async_add_microtask_t)(zend_async_microtask_t *microtask);
typedef zend_array* (*zend_async_get_awaiting_info_t)(zend_coroutine_t * coroutine);

typedef void (*zend_async_reactor_startup_t)();
typedef void (*zend_async_reactor_shutdown_t)();
typedef bool (*zend_async_reactor_execute_t)(bool no_wait);
typedef bool (*zend_async_reactor_loop_alive_t)();

typedef zend_async_poll_event_t* (*zend_async_new_socket_event_t)(zend_socket_t socket, async_poll_event events);
typedef zend_async_poll_event_t* (*zend_async_new_poll_event_t)(zend_file_descriptor_t fh, zend_socket_t socket, async_poll_event events);
typedef zend_async_timer_event_t* (*zend_async_new_timer_event_t)(int timeout, bool is_periodic);
typedef zend_async_signal_event_t* (*zend_async_new_signal_event_t)(int signum);
typedef zend_async_process_event_t* (*zend_async_new_process_event_t)();
typedef zend_async_thread_event_t* (*zend_async_new_thread_event_t)();
typedef zend_async_filesystem_event_t* (*zend_async_new_filesystem_event_t)(zend_string * path, const unsigned int flags);

typedef zend_async_dns_nameinfo_t* (*zend_async_getnameinfo_t)(const struct sockaddr* addr, int flags);
typedef zend_async_dns_addrinfo_t* (*zend_async_getaddrinfo_t)(const char *node, const char *service, const struct addrinfo *hints, int flags);

typedef void (*zend_async_queue_task_t)(zend_async_task_t *task);

typedef void (*zend_async_microtask_handler_t)(zend_async_microtask_t *microtask);

struct _zend_fcall_t {
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
};

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

/* Dynamic array of async event callbacks */
typedef struct _zend_async_callbacks_vector_t {
	uint32_t                      length;   /* current number of callbacks          */
	uint32_t                      capacity; /* allocated slots in the array         */
	zend_async_event_callback_t  **data;    /* dynamically allocated callback array */
} zend_async_callbacks_vector_t;

struct _zend_async_event_t {
	/* If event is closed, it cannot be started or stopped. */
	bool is_closed;
	/* The refcount of the event. */
	unsigned int ref_count;
	/* The Event loop reference count. */
	unsigned int loop_ref_count;
	zend_async_callbacks_vector_t callbacks;
	/* Methods */
	zend_async_event_add_callback_t add_callback;
	zend_async_event_del_callback_t del_callback;
	zend_async_event_start_t start;
	zend_async_event_stop_t stop;
	zend_async_event_dispose_t dispose;
};

/* Append a callback; grows the buffer when needed */
static zend_always_inline void
zend_async_callbacks_push(zend_async_event_t *event, zend_async_event_callback_t *callback)
{
	if (event->callbacks.data == NULL) {
		event->callbacks.data = safe_emalloc(4, sizeof(zend_async_event_callback_t *), 0);
		event->callbacks.capacity = 4;
	}

	zend_async_callbacks_vector_t *vector = &event->callbacks;

	if (vector->length == vector->capacity) {
		vector->capacity = vector->capacity ? vector->capacity * 2 : 4;
		vector->data = safe_erealloc(vector->data,
									 vector->capacity,
									 sizeof(zend_async_event_callback_t),
									 0);
	}

	vector->data[vector->length++] = callback;
}

/* Remove a specific callback; order is NOT preserved */
static zend_always_inline void
zend_async_callbacks_remove(zend_async_event_t *event, const zend_async_event_callback_t *callback)
{
	zend_async_callbacks_vector_t *vector = &event->callbacks;

	for (uint32_t i = 0; i < vector->length; ++i) {
		if (vector->data[i] == callback) {
			vector->data[i] = vector->data[--vector->length]; /* O(1) removal */
			callback->dispose(event, vector->data[i]);
			return;
		}
	}
}

/* Call all callbacks */
static zend_always_inline void
zend_async_callbacks_notify(zend_async_event_t *event, void *result, zend_object *exception)
{
	if (event->callbacks.data == NULL) {
		return;
	}

	const zend_async_callbacks_vector_t *vector = &event->callbacks;

	for (uint32_t i = 0; i < vector->length; ++i) {
		vector->data[i]->callback(event, vector->data[i], result, exception);
		if (UNEXPECTED(EG(exception) != NULL)) {
			break;
		}
	}
}

/* Free the vector’s memory */
static zend_always_inline void
zend_async_callbacks_free(zend_async_event_t *event)
{
	if (event->callbacks.data != NULL) {
		for (uint32_t i = 0; i < event->callbacks.length; ++i) {
			event->callbacks.data[i]->dispose(event, event->callbacks.data[i]);
		}

		efree(event->callbacks.data);
	}

	event->callbacks.data     = NULL;
	event->callbacks.length   = 0;
	event->callbacks.capacity = 0;
}

struct _zend_async_poll_event_t {
	zend_async_event_t base;
	bool is_socket;
	union {
		zend_file_descriptor_t file;
		zend_socket_t socket;
	};
	async_poll_event events;
	async_poll_event triggered_events;
};

struct _zend_async_timer_event_t {
	zend_async_event_t base;
	/* The timeout in milliseconds. */
	unsigned int timeout;
	/* The timer is periodic. */
	bool is_periodic;
};

struct _zend_async_signal_event_t {
	zend_async_event_t base;
	int signal;
};

struct _zend_async_process_event_t {
	zend_async_event_t base;
};

struct _zend_async_thread_event_t {
	zend_async_event_t base;
};

struct _zend_async_filesystem_event_t {
	zend_async_event_t base;
	zend_string *path;
	unsigned int flags;
	unsigned int triggered_events;
	zend_string *triggered_filename;
};

struct _zend_async_dns_nameinfo_t {
	zend_async_event_t base;
	const char *hostname;
	const char *service;
};

struct _zend_async_dns_addrinfo_t {
	zend_async_event_t base;
	const char *node;
	const char *service;
	const struct addrinfo *hints;
	int flags;
};

struct _zend_async_task_t {
	zend_async_event_t base;
};

typedef void (*zend_async_before_coroutine_enqueue_t)(zend_coroutine_t *coroutine, zend_async_scope_t *scope, zval *result);
typedef void (*zend_async_after_coroutine_enqueue_t)(zend_coroutine_t *coroutine, zend_async_scope_t *scope);

struct _zend_async_scope_t {
	bool is_closed;
	zend_async_before_coroutine_enqueue_t before_coroutine_enqueue;
	zend_async_after_coroutine_enqueue_t after_coroutine_enqueue;
};

typedef void (*zend_async_waker_dtor)(zend_coroutine_t *coroutine);

typedef struct {
	zend_async_event_t *event;
	zend_async_event_callback_t *callback;
} zend_async_waker_trigger_t;

typedef enum {
	ZEND_ASYNC_WAKER_NO_STATUS = 0,
	ZEND_ASYNC_WAKER_WAITING = 1,
	ZEND_ASYNC_WAKER_QUEUED = 2,
	ZEND_ASYNC_WAKER_IGNORED = 3
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
	zend_fcall_t *fcall;

	zend_coroutine_internal_t internal_function;

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
#define ZEND_CURRENT_ASYNC_SCOPE EG(async_scope)

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
ZEND_API zend_async_new_socket_event_t zend_async_new_socket_event_fn;
ZEND_API zend_async_new_poll_event_t zend_async_new_poll_event_fn;
ZEND_API zend_async_new_timer_event_t zend_async_new_timer_event_fn;
ZEND_API zend_async_new_signal_event_t zend_async_new_signal_event_fn;
ZEND_API zend_async_new_process_event_t zend_async_new_process_event_fn;
ZEND_API zend_async_new_thread_event_t zend_async_new_thread_event_fn;
ZEND_API zend_async_new_filesystem_event_t zend_async_new_filesystem_event_fn;

/* DNS API */

ZEND_API zend_async_getnameinfo_t zend_async_getnameinfo_fn;
ZEND_API zend_async_getaddrinfo_t zend_async_getaddrinfo_fn;

/* Thread pool API */
ZEND_API bool zend_async_thread_pool_is_enabled(void);
ZEND_API zend_async_queue_task_t zend_async_queue_task_fn;

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
);

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
);

ZEND_API void zend_async_thread_pool_register(
	zend_string *module, bool allow_override, zend_async_queue_task_t queue_task_fn
);
/* Waker API */
ZEND_API zend_async_waker_t *zend_async_waker_create(zend_coroutine_t *coroutine);
ZEND_API void zend_async_waker_destroy(zend_coroutine_t *coroutine);
ZEND_API void zend_async_waker_add_event(zend_coroutine_t *coroutine, zend_async_event_t *event, zend_async_event_callback_t *callback);
ZEND_API void zend_async_waker_del_event(zend_coroutine_t *coroutine, zend_async_event_t *event);

END_EXTERN_C()

#define ZEND_ASYNC_IS_ENABLED() zend_async_is_enabled()
#define ZEND_ASYNC_SPAWN() zend_async_spawn_fn()
#define ZEND_ASYNC_NEW_COROUTINE(scope) zend_async_new_coroutine_fn(scope)
#define ZEND_ASYNC_SUSPEND(coroutine) zend_async_suspend_fn(coroutine)
#define ZEND_ASYNC_RESUME(coroutine) zend_async_resume_fn(coroutine)
#define ZEND_ASYNC_CANCEL(coroutine, error, transfer_error) zend_async_cancel_fn(coroutine, error, transfer_error)
#define ZEND_ASYNC_SHUTDOWN() zend_async_shutdown_fn()
#define ZEND_ASYNC_GET_COROUTINES() zend_async_get_coroutines_fn()
#define ZEND_ASYNC_ADD_MICROTASK(microtask) zend_async_add_microtask_fn(microtask)

#define ZEND_ASYNC_REACTOR_IS_ENABLED() zend_async_reactor_is_enabled()
#define ZEND_ASYNC_REACTOR_STARTUP() zend_async_reactor_startup_fn()
#define ZEND_ASYNC_REACTOR_SHUTDOWN() zend_async_reactor_shutdown_fn()

#define ZEND_ASYNC_REACTOR_EXECUTE(no_wait) zend_async_reactor_execute_fn(no_wait)
#define ZEND_ASYNC_REACTOR_LOOP_ALIVE() zend_async_reactor_loop_alive_fn()

#define ZEND_ASYNC_NEW_SOCKET_EVENT(socket, events) zend_async_new_socket_event_fn(socket, events)
#define ZEND_ASYNC_NEW_POLL_EVENT(fh, socket, events) zend_async_new_poll_event_fn(fh, socket, events)
#define ZEND_ASYNC_NEW_TIMER_EVENT(timeout, is_periodic) zend_async_new_timer_event_fn(timeout, is_periodic)
#define ZEND_ASYNC_NEW_SIGNAL_EVENT(signum) zend_async_new_signal_event_fn(signum)
#define ZEND_ASYNC_NEW_PROCESS_EVENT() zend_async_new_process_event_fn()
#define ZEND_ASYNC_NEW_THREAD_EVENT() zend_async_new_thread_event_fn()
#define ZEND_ASYNC_NEW_FILESYSTEM_EVENT(path, flags) zend_async_new_filesystem_event_fn(path, flags)

#define ZEND_ASYNC_QUEUE_TASK(task) zend_async_queue_task_fn(task)
