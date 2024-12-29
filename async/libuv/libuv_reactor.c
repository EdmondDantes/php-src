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
#include "libuv_reactor.h"

#include <zend_exceptions.h>
#include <async/php_async.h>
#include <async/php_reactor.h>
#include <async/php_scheduler.h>
#include <async/php_layer/zend_common.h>
#include <ext/spl/spl_exceptions.h>

#include "../php_layer/exceptions.h"

#define UVLOOP ((uv_loop_t *) ASYNC_G(extend))

static async_microtasks_handler_t microtask_handler = NULL;
static async_next_fiber_handler_t next_fiber_handler = NULL;

static void libuv_close_cb(uv_handle_t *handle)
{
	pefree(handle, 1);
}

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

static zend_always_inline void libuv_poll_alloc(libuv_poll_t * poll, const int actions, int handle)
{
	poll->uv_handle = pecalloc(1, sizeof(uv_poll_t), 1);

	int error = uv_poll_init(poll->uv_handle->loop, poll->uv_handle, handle);

	if (error < 0) {
		async_throw_poll("Failed to initialize poll handle: %s", uv_strerror(error));
		pefree(poll->uv_handle, 1);
		poll->uv_handle = NULL;
		return;
	}

	error = uv_poll_start(poll->uv_handle, actions, on_poll_event);

	if (error < 0) {
		async_throw_poll("Failed to start poll handle: %s", uv_strerror(error));
		pefree(poll->uv_handle, 1);
		poll->uv_handle = NULL;
	}
}

static void libuv_poll_ctor(reactor_handle_t *handle, ...)
{
	libuv_poll_t *poll = (libuv_poll_t *)handle;

	va_list args;
	va_start(args, handle);
	const zval *raw_handle = va_arg(args, zval *);
	const zend_long actions = va_arg(args, zend_long);
	va_end(args);

	if (raw_handle == NULL) {
		zend_throw_exception(spl_ce_InvalidArgumentException, "Invalid poll handle", 0);
		return;
	}

	if (actions == 0) {
		zend_throw_exception(spl_ce_InvalidArgumentException, "Invalid poll actions", 0);
		return;
	}

	php_socket_t socket = 0;
	php_file_descriptor_t file;

	if (Z_TYPE_P(raw_handle) == IS_RESOURCE) {
		async_resource_to_fd(Z_RES_P(raw_handle), &socket, &file);
		IF_THROW_RETURN_VOID;
	} else if (Z_TYPE_P(raw_handle) == IS_OBJECT) {
		socket = async_try_extract_socket_object(Z_OBJ_P(raw_handle));
	} else {
        zend_throw_exception(
        	spl_ce_InvalidArgumentException,
        	"Invalid poll handle. The handle must be a socket or I/O resource.",
        	0
        	);

        return;
	}

	if (UNEXPECTED(poll->handle.type == REACTOR_H_SOCKET && socket == 0)) {
        zend_throw_exception(spl_ce_InvalidArgumentException, "Invalid resource for socket descriptor", 0);
		return;
    }

	libuv_poll_alloc(poll, libuv_events_from_php(actions), socket > 0 ? socket : file);
}

static void libuv_poll_dtor(reactor_handle_t *handle)
{
	libuv_poll_t *poll = (libuv_poll_t *)handle;

	const int error = uv_poll_stop(poll->uv_handle);

	if (error < 0) {
		zend_error(E_WARNING, "Failed to stop poll handle: %s", uv_strerror(error));
    }

	uv_close((uv_handle_t *)poll->uv_handle, libuv_close_cb);
}

libuv_poll_t *libuv_poll_new()
{
	libuv_poll_t *poll = pecalloc(1, sizeof(libuv_poll_t), 1);
	poll->handle.ctor = libuv_poll_ctor;
	poll->handle.dtor = libuv_poll_dtor;
	poll->uv_handle->data = poll;

	return poll;
}

static libuv_poll_t* libuv_poll_new(const int fd, const REACTOR_HANDLE_TYPE type, const zend_long events)
{
	libuv_poll_t *poll_handle = libuv_poll_new();

	if (poll_handle == NULL) {
		return NULL;
	}

	poll_handle->handle.type = type;

	int res = uv_poll_init(UVLOOP, poll_handle->uv_handle, fd);

	if (res < 0) {
		async_throw_poll("Failed to initialize poll handle: %s", uv_strerror(res));
		pefree(poll_handle, 1);
		return NULL;
	}

	res = uv_poll_start(poll_handle->uv_handle, libuv_events_from_php(events), on_poll_event);

	if (res < 0) {
		async_throw_poll("Failed to start poll handle: %s", uv_strerror(res));
		uv_close((uv_handle_t*)poll_handle, libuv_close_cb);
		pefree(poll_handle, 1);
		return NULL;
	}

	return poll_handle;
}

static void handle_callbacks(void)
{
	uv_run(UVLOOP, UV_RUN_ONCE);
}

static void libuv_startup(void)
{

}

static void libuv_shutdown(void)
{

}

static void libuv_add_handle_ex(reactor_handle_t *handle)
{

}

static void libuv_remove_handle(reactor_handle_t *handle)
{

}

/**
 * The prepare callback is executed immediately before the event loop enters
 * the blocking phase to wait for new I/O events. This allows for the execution
 * of pending tasks or preparations right before the event loop goes idle.
 *
 * @param handle Pointer to the uv_prepare_t handle.
 */
static void prepare_cb(uv_prepare_t *handle)
{
	if (microtask_handler != NULL) {
		microtask_handler();
	}

	if (EG(exception) != NULL) {
        uv_stop(UVLOOP);
    }
}

/**
 * The check callback is executed after all I/O callbacks have been invoked
 * but before the next iteration of the event loop and before timers like setImmediate.
 * This allows for tasks to run immediately after I/O processing.
 *
 * @param handle Pointer to the uv_check_t handle.
 */
static void check_cb(uv_check_t *handle)
{
	if (microtask_handler != NULL) {
		microtask_handler();
	}

	if (EG(exception) != NULL) {
		uv_stop(UVLOOP);
	}

	// Execute the next fiber task if scheduled, after microtasks have been processed.
	if (next_fiber_handler != NULL) {
		next_fiber_handler();
	}

	if (EG(exception) != NULL) {
		uv_stop(UVLOOP);
	}
}

static zend_bool execute_callbacks(const zend_bool no_wait)
{
	return uv_run(UVLOOP, no_wait ? UV_RUN_NOWAIT : UV_RUN_ONCE);
}

static void libuv_loop_stop(void)
{
	uv_stop(UVLOOP);
}

static zend_bool libuv_loop_alive(void)
{
    return uv_loop_alive(UVLOOP);
}

static async_microtasks_handler_t libuv_set_microtask_handler(const async_microtasks_handler_t handler)
{
	const async_microtasks_handler_t old = microtask_handler;
	microtask_handler = handler;
	return old;
}

static async_next_fiber_handler_t libuv_set_next_fiber_handler(const async_next_fiber_handler_t handler)
{
    const async_next_fiber_handler_t old = next_fiber_handler;
    next_fiber_handler = handler;
    return old;
}


static size_t async_ex_globals_handler(const async_globals_t* async_globals, size_t current_size, const zend_bool is_destroy)
{
	if (async_globals == NULL) {
		return sizeof(uv_loop_t);
	}

	uv_loop_t *loop = UVLOOP;

	if (is_destroy) {

		if (loop->data != NULL) {
			uv_loop_close(loop);
			loop->data = NULL;
		}
	} else {

		const int result = uv_loop_init(loop);

		if (result != 0) {
			async_throw_error("Failed to initialize loop: %s", uv_strerror(result));
		}

		uv_loop_set_data(loop, (void *) async_globals);
	}

	return 0;
}

static reactor_handle_t* libuv_object_create(zend_class_entry *class_entry)
{
	// This is function call from zend_API.c
	// ZEND_API zend_result object_and_properties_init(zval *arg, zend_class_entry *class_type, HashTable *properties)
	// => _object_and_properties_init
	//
	// This function is responsible for:
	// * Allocating memory
	// * Initializing properties
	//
	// It is inherited by all child objects. Therefore, you must take this into account!
	//

	libuv_handle_t *object;

	if (class_entry == async_ce_socket_handle) {

		object = zend_object_alloc(sizeof(libuv_poll_t), class_entry);
		object->handle.type = REACTOR_H_SOCKET;
		object->handle.ctor = libuv_poll_ctor;
		object->handle.dtor = libuv_poll_dtor;
		object->uv_handle = pecalloc(1, sizeof(uv_poll_t), 1);

	} else if (class_entry == async_ce_file_handle) {

		object = zend_object_alloc(sizeof(libuv_poll_t), class_entry);
		object->handle.type = REACTOR_H_FILE;
		object->handle.ctor = libuv_poll_ctor;
		object->handle.dtor = libuv_poll_dtor;
		object->uv_handle = pecalloc(1, sizeof(uv_poll_t), 1);

	} else if (class_entry == async_ce_pipe_handle) {

		object = zend_object_alloc(sizeof(libuv_poll_t), class_entry);
		object->handle.type = REACTOR_H_PIPE;
		object->handle.ctor = libuv_poll_ctor;
		object->handle.dtor = libuv_poll_dtor;
		object->uv_handle = pecalloc(1, sizeof(uv_poll_t), 1);

	} else if (class_entry == async_ce_tty_handle) {

		object = zend_object_alloc(sizeof(libuv_poll_t), class_entry);
		object->handle.type = REACTOR_H_TTY;
		object->handle.ctor = libuv_poll_ctor;
		object->handle.dtor = libuv_poll_dtor;
		object->uv_handle = pecalloc(1, sizeof(uv_poll_t), 1);

	} else if (class_entry == async_ce_timer_handle) {

		object = zend_object_alloc(sizeof(libuv_timer_t), class_entry);
		object->handle.type = REACTOR_H_TIMER;
		object->uv_handle = pecalloc(1, sizeof(uv_timer_t), 1);
		object->handle.ctor = libuv_timer_ctor;
		object->handle.dtor = libuv_timer_dtor;

	} else if (class_entry == async_ce_signal_handle) {

		object = zend_object_alloc(sizeof(libuv_signal_t), class_entry);
		object->handle.type = REACTOR_H_SIGNAL;
		object->uv_handle = pecalloc(1, sizeof(uv_signal_t), 1);
		object->handle.ctor = libuv_signal_ctor;
		object->handle.dtor = libuv_signal_dtor;

	} else if (class_entry == async_ce_process_handle) {

		object = zend_object_alloc(sizeof(libuv_signal_t), class_entry);
		object->handle.type = REACTOR_H_PROCESS;
		object->uv_handle = pecalloc(1, sizeof(uv_signal_t), 1);
		object->handle.ctor = libuv_process_ctor;
		object->handle.dtor = libuv_process_dtor;

	} else if (class_entry == async_ce_thread_handle) {

		object = zend_object_alloc(sizeof(libuv_thread_cb_t), class_entry);
		object->handle.type = REACTOR_H_THREAD;
		object->uv_handle = pecalloc(1, sizeof(uv_async_t), 1);
		object->handle.ctor = libuv_thread_ctor;
		object->handle.dtor = libuv_thread_dtor;

	} else if (class_entry == async_ce_file_system_handle) {

		object = zend_object_alloc(sizeof(libuv_fs_event_t), class_entry);
		object->handle.type = REACTOR_H_FILE_SYSTEM;
		object->uv_handle = pecalloc(1, sizeof(uv_fs_event_t), 1);
		object->handle.ctor = libuv_file_system_ctor;
		object->handle.dtor = libuv_file_system_dtor;

	} else {
		return reactor_default_object_create(class_entry);
	}

	// Link the uv handle to the PHP object.
	object->uv_handle->data = object;
	object->uv_handle->loop = UVLOOP;

	zend_object_std_init(&object->handle.std, class_entry);
	object_properties_init(&object->handle.std, class_entry);

	return &object->handle;
}

/**
 * Previous handlers.
 */
static reactor_startup_t prev_reactor_startup_fn = NULL;
static reactor_shutdown_t prev_reactor_shutdown_fn = NULL;

static reactor_object_create_t prev_reactor_object_create_fn = NULL;

static reactor_handle_method_t prev_reactor_add_handle_ex_fn = NULL;
static reactor_handle_method_t prev_reactor_remove_handle_fn = NULL;

static reactor_stop_t prev_reactor_loop_stop_fn = NULL;
static reactor_loop_alive_t prev_reactor_loop_alive_fn = NULL;

static reactor_handle_from_resource_t prev_reactor_handle_from_resource_fn = NULL;
static reactor_file_new_t prev_reactor_file_new_fn = NULL;
static reactor_socket_new_t prev_reactor_socket_new_fn = NULL;
static reactor_pipe_new_t prev_reactor_pipe_new_fn = NULL;
static reactor_tty_new_t prev_reactor_tty_new_fn = NULL;

static reactor_timer_new_t prev_reactor_timer_new_fn = NULL;
static reactor_signal_new_t prev_reactor_signal_new_fn = NULL;
static reactor_process_new_t prev_reactor_process_new_fn = NULL;
static reactor_thread_new_t prev_reactor_thread_new_fn = NULL;

static reactor_file_system_new_t prev_reactor_file_system_new_fn = NULL;

static reactor_handle_t* libuv_handle_from_resource(zend_resource *resource, zend_ulong actions)
{

}

static reactor_handle_t* libuv_file_new(const php_file_descriptor_t fd, const zend_ulong events)
{
	zval object;

	if (object_init_ex(&object, async_ce_file_handle) == FAILURE) {
		return NULL;
	}

	libuv_poll_t * poll = (libuv_poll_t *) Z_OBJ_P(&object);

	libuv_poll_alloc(poll, (int) events, (int) fd);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&poll->handle.std);
		return NULL;
	}

	poll->handle.type = REACTOR_H_FILE;

	return (reactor_handle_t *) poll;
}

static reactor_handle_t* libuv_socket_new(const php_socket_t socket, const zend_ulong events)
{
	zval object;

	//
	// Create a new object of the class Async\SocketHandle without calling the constructor.
	//
	if (object_init_ex(&object, async_ce_socket_handle) == FAILURE) {
		return NULL;
	}

	libuv_poll_t * poll = (libuv_poll_t *) Z_OBJ_P(&object);

	libuv_poll_alloc(poll, (int) events, (int) socket);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&poll->handle.std);
		return NULL;
	}

	poll->handle.type = REACTOR_H_SOCKET;

	return (reactor_handle_t *) poll;
}

static reactor_handle_t* libuv_pipe_new(const php_file_descriptor_t fd, const zend_ulong events)
{
	zval object;

	if (object_init_ex(&object, async_ce_pipe_handle) == FAILURE) {
		return NULL;
	}

	libuv_poll_t * poll = (libuv_poll_t *) Z_OBJ_P(&object);

	libuv_poll_alloc(poll, (int) events, (int) fd);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&poll->handle.std);
		return NULL;
	}

	poll->handle.type = REACTOR_H_PIPE;

	return (reactor_handle_t *) poll;
}

static reactor_handle_t* libuv_tty_new(const php_file_descriptor_t fd, const zend_ulong events)
{
	zval object;

	if (object_init_ex(&object, async_ce_tty_handle) == FAILURE) {
		return NULL;
	}

	libuv_poll_t * poll = (libuv_poll_t *) Z_OBJ_P(&object);

	libuv_poll_alloc(poll, (int) events, (int) fd);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&poll->handle.std);
		return NULL;
	}

	poll->handle.type = REACTOR_H_TTY;

	return (reactor_handle_t *) poll;
}

static reactor_handle_t* libuv_timer_new(const zend_ulong timeout)
{
	zval object;
	zval params[1];

	ZVAL_LONG(&params[0], timeout);

	if (object_init_with_constructor(&object, async_ce_timer_handle, 1, params, NULL) == FAILURE) {
		return NULL;
	}

	return (reactor_handle_t *) Z_OBJ_P(&object);
}

static reactor_handle_t* libuv_signal_new(const zend_long sig_number)
{
	zval object;
	zval params[1];

	ZVAL_LONG(&params[0], sig_number);

	if (object_init_with_constructor(&object, async_ce_signal_handle, 1, params, NULL) == FAILURE) {
		return NULL;
	}

	return (reactor_handle_t *) Z_OBJ_P(&object);
}

static reactor_handle_t* libuv_process_new(const php_process_id_t pid, const zend_ulong events)
{
	zval object;
	zval params[2];

	ZVAL_LONG(&params[0], pid);
	ZVAL_LONG(&params[0], events);

	if (object_init_with_constructor(&object, async_ce_process_handle, 2, params, NULL) == FAILURE) {
		return NULL;
	}

	return (reactor_handle_t *) Z_OBJ_P(&object);
}

static reactor_handle_t* libuv_thread_new(const THREAD_T tid, const zend_ulong events)
{
	zval object;
	zval params[2];

	ZVAL_LONG(&params[0], tid);
	ZVAL_LONG(&params[0], events);

	if (object_init_with_constructor(&object, async_ce_thread_handle, 2, params, NULL) == FAILURE) {
		return NULL;
	}

	return (reactor_handle_t *) Z_OBJ_P(&object);
}

static reactor_handle_t* libuv_file_system_new(const char *path, const size_t length, const zend_ulong events)
{
	zval object;
	zval params[2];

	zend_string *z_path = zend_string_init(path, length, 0);

	ZVAL_STR(&params[0], z_path);
	ZVAL_LONG(&params[0], events);

	if (object_init_with_constructor(&object, async_ce_thread_handle, 2, params, NULL) == FAILURE) {
		zval_dtor(&params[0]);
		return NULL;
	}

	zval_dtor(&params[0]);
	return (reactor_handle_t *) Z_OBJ_P(&object);
}

static void setup_handlers(void)
{
	async_set_ex_globals_handler(async_ex_globals_handler);
	async_scheduler_set_callbacks_handler(execute_callbacks);

	prev_reactor_startup_fn = reactor_startup_fn;
	reactor_startup_fn = NULL;

	prev_reactor_shutdown_fn = reactor_shutdown_fn;
	reactor_shutdown_fn = NULL;

	prev_reactor_object_create_fn = reactor_object_create_fn;
	reactor_object_create_fn = libuv_object_create;

	prev_reactor_add_handle_ex_fn = reactor_add_handle_ex_fn;
	reactor_add_handle_ex_fn = NULL;

	prev_reactor_remove_handle_fn = reactor_remove_handle_fn;
	reactor_remove_handle_fn = NULL;

	prev_reactor_loop_stop_fn = reactor_stop_fn;
	reactor_stop_fn = libuv_loop_stop;

	prev_reactor_loop_alive_fn = reactor_loop_alive_fn;
	reactor_loop_alive_fn = libuv_loop_alive;

	prev_reactor_handle_from_resource_fn = reactor_handle_from_resource_fn;
	reactor_handle_from_resource_fn = libuv_handle_from_resource;

	prev_reactor_file_new_fn = reactor_file_new_fn;
	reactor_file_new_fn = libuv_file_new;

	prev_reactor_socket_new_fn = reactor_socket_new_fn;
	reactor_socket_new_fn = libuv_socket_new;

	prev_reactor_pipe_new_fn = reactor_pipe_new_fn;
	reactor_pipe_new_fn = libuv_pipe_new;

	prev_reactor_tty_new_fn = reactor_tty_new_fn;
	reactor_tty_new_fn = libuv_tty_new;

	prev_reactor_timer_new_fn = reactor_timer_new_fn;
	reactor_timer_new_fn = libuv_timer_new;

	prev_reactor_signal_new_fn = reactor_signal_new_fn;
	reactor_signal_new_fn = libuv_signal_new;

	prev_reactor_process_new_fn = reactor_process_new_fn;
	reactor_process_new_fn = libuv_process_new;

	prev_reactor_thread_new_fn = reactor_thread_new_fn;
	reactor_thread_new_fn = libuv_thread_new;

	prev_reactor_file_system_new_fn = reactor_file_system_new_fn;
	reactor_file_system_new_fn = libuv_file_system_new;
}

static void restore_handlers(void)
{
	async_set_ex_globals_handler(NULL);

	reactor_startup_fn = prev_reactor_startup_fn;
	reactor_shutdown_fn = prev_reactor_shutdown_fn;

	reactor_object_create_fn = prev_reactor_object_create_fn;

	reactor_add_handle_ex_fn = prev_reactor_add_handle_ex_fn;
	reactor_remove_handle_fn = prev_reactor_remove_handle_fn;

	reactor_stop_fn = prev_reactor_loop_stop_fn;
	reactor_loop_alive_fn = prev_reactor_loop_alive_fn;

	reactor_handle_from_resource_fn = prev_reactor_handle_from_resource_fn;
	reactor_file_new_fn = prev_reactor_file_new_fn;
	reactor_socket_new_fn = prev_reactor_socket_new_fn;
	reactor_pipe_new_fn = prev_reactor_pipe_new_fn;
	reactor_tty_new_fn = prev_reactor_tty_new_fn;

	reactor_timer_new_fn = prev_reactor_timer_new_fn;
	reactor_signal_new_fn = prev_reactor_signal_new_fn;
	reactor_process_new_fn = prev_reactor_process_new_fn;
	reactor_thread_new_fn = prev_reactor_thread_new_fn;
	reactor_file_system_new_fn = prev_reactor_file_system_new_fn;
}

void async_libuv_startup(void)
{
	setup_handlers();
}

void async_libuv_shutdown(void)
{
	restore_handlers();
}