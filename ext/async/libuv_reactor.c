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
#include <Zend/zend_async_API.h>

#include "exceptions.h"
#include "php_async.h"
#include "zend_common.h"

typedef struct
{
	uv_loop_t loop;
#ifdef PHP_WIN32
	uv_thread_t * watcherThread;
	HANDLE ioCompletionPort;
	unsigned int countWaitingDescriptors;
	bool isRunning;
	uv_async_t * uvloop_wakeup;
	/* Circular buffer of libuv_process_t ptr */
	circular_buffer_t * pid_queue;
#endif
} libuv_reactor_t;

static void libuv_reactor_stop_with_exception(void);

#define UVLOOP ((uv_loop_t *) ASYNC_G(reactor))
#define LIBUV_REACTOR ((libuv_reactor_t *) ASYNC_G(reactor))
#define WATCHER ((libuv_reactor_t *) ASYNC_G(reactor))->watcherThread
#define IF_EXCEPTION_STOP_REACTOR if (UNEXPECTED(EG(exception) != NULL)) { libuv_reactor_stop_with_exception(); }

/* {{{ libuv_reactor_startup */
void libuv_reactor_startup(void)
{
	if (ASYNC_G(reactor) != NULL) {
		return;
	}

	ASYNC_G(reactor) = pecalloc(1, sizeof(libuv_reactor_t), 1);
	const int result = uv_loop_init(ASYNC_G(reactor));

	if (result != 0) {
		async_throw_error("Failed to initialize loop: %s", uv_strerror(result));
		return;
	}

	uv_loop_set_data(ASYNC_G(reactor), ASYNC_G(reactor));
}
/* }}} */

/* {{{ libuv_reactor_stop_with_exception */
static void libuv_reactor_stop_with_exception(void)
{
	// TODO: implement libuv_reactor_stop_with_exception
}
/* }}} */

/* {{{ libuv_reactor_shutdown */
void libuv_reactor_shutdown(void)
{
	if (EXPECTED(ASYNC_G(reactor) != NULL)) {

		if (uv_loop_alive(UVLOOP) != 0) {
			// need to finish handlers
			uv_run(UVLOOP, UV_RUN_ONCE);
		}

		uv_loop_close(ASYNC_G(reactor));
		pefree(ASYNC_G(reactor), 1);
		ASYNC_G(reactor) = NULL;
	}
}
/* }}} */

/* {{{ libuv_reactor_execute */
bool libuv_reactor_execute(bool no_wait)
{
	const bool has_handles = uv_run(UVLOOP, no_wait ? UV_RUN_NOWAIT : UV_RUN_ONCE);

	if (UNEXPECTED(has_handles == false && ASYNC_G(active_event_count) > 0)) {
		async_warning("event_handle_count %d is greater than 0 but no handles are available", ASYNC_G(active_event_count));
		return false;
	}

	return ASYNC_G(active_event_count) > 0 && has_handles;
}
/* }}} */

/* {{{ libuv_reactor_loop_alive */
bool libuv_reactor_loop_alive(void)
{
	if (UVLOOP == NULL) {
		return false;
	}

	return ASYNC_G(active_event_count) > 0 && uv_loop_alive(UVLOOP) != 0;
}
/* }}} */

/* {{{ libuv_close_handle_cb */
static void libuv_close_handle_cb(uv_handle_t *handle)
{
	pefree(handle->data, 0);
}
/* }}} */

/* {{{ libuv_add_callback */
static void libuv_add_callback(zend_async_event_t *event, zend_async_event_callback_t *callback)
{
	zend_async_callbacks_push(event, callback);
}
/* }}} */

/* {{{ libuv_remove_callback */
static void libuv_remove_callback(zend_async_event_t *event, zend_async_event_callback_t *callback)
{
	zend_async_callbacks_remove(event, callback);
}
/* }}} */

/////////////////////////////////////////////////////////////////////////////
/// Poll API
//////////////////////////////////////////////////////////////////////////////

/* {{{ on_poll_event */
static void on_poll_event(const uv_poll_t* handle, const int status, const int events)
{
	async_poll_event_t *poll = handle->data;
	zend_object *exception = NULL;

	if (status < 0) {
		exception = async_new_exception(
			async_ce_input_output_exception, "Input output error: %s", uv_strerror(status)
		);
	}

	poll->event.triggered_events = events;

	zend_async_callbacks_notify(&poll->event.base, NULL, exception);

	if (exception != NULL) {
		zend_object_release(exception);
	}

	IF_EXCEPTION_STOP_REACTOR;
}
/* }}} */

/* {{{ libuv_poll_start */
static void libuv_poll_start(zend_async_event_t *event)
{
	if (event->loop_ref_count > 0) {
		event->loop_ref_count++;
		return;
	}

    async_poll_event_t *poll = (async_poll_event_t *)(event);

    const int error = uv_poll_start(&poll->uv_handle, poll->event.events, on_poll_event);

    if (error < 0) {
        async_throw_error("Failed to start poll handle: %s", uv_strerror(error));
    	return;
    }

	event->loop_ref_count++;
    ASYNC_G(active_event_count)++;
}
/* }}} */

/* {{{ libuv_poll_stop */
static void libuv_poll_stop(zend_async_event_t *event)
{
	if (event->loop_ref_count > 1) {
		event->loop_ref_count--;
		return;
	}

	async_poll_event_t *poll = (async_poll_event_t *)(event);

	const int error = uv_poll_stop(&poll->uv_handle);

	event->loop_ref_count = 0;
	ASYNC_G(active_event_count)--;

	if (error < 0) {
		async_throw_error("Failed to stop poll handle: %s", uv_strerror(error));
		return;
	}
}
/* }}} */

/* {{{ libuv_poll_dispose */
static void libuv_poll_dispose(zend_async_event_t *event)
{
	if (event->ref_count > 1) {
		event->ref_count--;
		return;
	}

	if (event->loop_ref_count > 0) {
		event->loop_ref_count = 1;
		libuv_poll_stop(event);
	}

	zend_async_callbacks_free(event);

	async_poll_event_t *poll = (async_poll_event_t *)(event);

	uv_close((uv_handle_t *)&poll->uv_handle, libuv_close_handle_cb);

	pefree(event, 0);
}
/* }}} */

/* {{{ libuv_new_poll_event */
zend_async_poll_event_t* libuv_new_poll_event(zend_file_descriptor_t fh, zend_socket_t socket, async_poll_event events)
{
	async_poll_event_t *poll = pecalloc(1, sizeof(async_poll_event_t), 0);

	int error = 0;

	if (socket != 0) {
		error = uv_poll_init_socket(UVLOOP, &poll->uv_handle, socket);
		poll->event.is_socket = true;
		poll->event.socket = socket;
	} else if (fh != NULL) {
		error = uv_poll_init(UVLOOP, &poll->uv_handle, (int) fh);
		poll->event.is_socket = false;
		poll->event.file = fh;
	} else {

	}

	if (error < 0) {
		async_throw_error("Failed to initialize poll handle: %s", uv_strerror(error));
		pefree(poll, 0);
		return NULL;
	}

	// Link the handle to the loop.
	poll->uv_handle.data = poll;
	poll->event.events = events;

	// Initialize the event methods
	poll->event.base.add_callback = libuv_add_callback;
	poll->event.base.del_callback = libuv_remove_callback;
	poll->event.base.start = libuv_poll_start;
	poll->event.base.stop = libuv_poll_stop;
	poll->event.base.dispose = libuv_poll_dispose;

	return &poll->event;
}
/* }}} */

/* {{{ libuv_new_socket_event */
zend_async_poll_event_t* libuv_new_socket_event(zend_socket_t socket, async_poll_event events)
{
	return libuv_new_poll_event(NULL, socket, events);
}
/* }}} */

/////////////////////////////////////////////////////////////////////////////////
/// Timer API
/////////////////////////////////////////////////////////////////////////////////

/* {{{ on_timer_event */
static void on_timer_event(uv_timer_t *handle)
{
	async_timer_event_t *poll = handle->data;

	zend_async_callbacks_notify(&poll->event.base, NULL, NULL);

	IF_EXCEPTION_STOP_REACTOR;
}
/* }}} */

/* {{{ libuv_timer_start */
static void libuv_timer_start(zend_async_event_t *event)
{
	if (event->loop_ref_count > 0) {
		event->loop_ref_count++;
		return;
	}

	async_timer_event_t *timer = (async_timer_event_t *)(event);

	const int error = uv_timer_start(
		&timer->uv_handle,
		on_timer_event,
		timer->event.timeout,
		timer->event.is_periodic ? timer->event.timeout : 0
	);

	if (error < 0) {
		async_throw_error("Failed to start timer handle: %s", uv_strerror(error));
		return;
	}

	event->loop_ref_count++;
	ASYNC_G(active_event_count)++;
}
/* }}} */

/* {{{ libuv_timer_stop */
static void libuv_timer_stop(zend_async_event_t *event)
{
	if (event->loop_ref_count > 1) {
		event->loop_ref_count--;
		return;
	}

	async_timer_event_t *timer = (async_timer_event_t *)(event);

	const int error = uv_timer_stop(&timer->uv_handle);

	event->loop_ref_count = 0;
	ASYNC_G(active_event_count)--;

	if (error < 0) {
		async_throw_error("Failed to stop timer handle: %s", uv_strerror(error));
		return;
	}
}
/* }}} */

/* {{{ libuv_timer_dispose */
static void libuv_timer_dispose(zend_async_event_t *event)
{
	if (event->ref_count > 1) {
		event->ref_count--;
		return;
	}

	if (event->loop_ref_count > 0) {
		event->loop_ref_count = 1;
		libuv_timer_stop(event);
	}

	zend_async_callbacks_free(event);

	async_timer_event_t *timer = (async_timer_event_t *)(event);

	uv_close((uv_handle_t *)&timer->uv_handle, libuv_close_handle_cb);

	pefree(event, 0);
}
/* }}} */

/* {{{ libuv_new_timer_event */
zend_async_timer_event_t* libuv_new_timer_event(const zend_ulong timeout, const bool is_periodic)
{
	if (UNEXPECTED(timeout < 0)) {
		zend_throw_exception(zend_ce_type_error, "Invalid timeout", 0);
		return NULL;
	}

	async_timer_event_t *event = pecalloc(1, sizeof(async_timer_event_t), 0);

	const int error = uv_timer_init(UVLOOP, &event->uv_handle);

	if (error < 0) {
		async_throw_error("Failed to initialize timer handle: %s", uv_strerror(error));
		pefree(event, 0);
		return NULL;
	}

	event->uv_handle.data = event;
	event->event.timeout = timeout;
	event->event.is_periodic = is_periodic;

	event->event.base.add_callback = libuv_add_callback;
	event->event.base.del_callback = libuv_remove_callback;
	event->event.base.start = libuv_timer_start;
	event->event.base.stop = libuv_timer_stop;
	event->event.base.dispose = libuv_timer_dispose;

	return &event->event;
}
/* }}} */

/////////////////////////////////////////////////////////////////////////////////
///// Signal API
////////////////////////////////////////////////////////////////////////////////

/* {{{ on_signal_event */
static void on_signal_event(uv_signal_t *handle, int signum)
{
    async_signal_event_t *signal = handle->data;

    zend_async_callbacks_notify(&signal->event.base, &signum, NULL);

    IF_EXCEPTION_STOP_REACTOR;
}
/* }}} */

/* {{{ libuv_signal_start */
static void libuv_signal_start(zend_async_event_t *event)
{
    if (event->loop_ref_count > 0) {
        event->loop_ref_count++;
        return;
    }

    async_signal_event_t *signal = (async_signal_event_t *)(event);

    const int error = uv_signal_start(
        &signal->uv_handle,
        on_signal_event,
        signal->event.signal
    );

    if (error < 0) {
        async_throw_error("Failed to start signal handle: %s", uv_strerror(error));
        return;
    }

    event->loop_ref_count++;
    ASYNC_G(active_event_count)++;
}
/* }}} */

/* {{{ libuv_signal_stop */
static void libuv_signal_stop(zend_async_event_t *event)
{
    if (event->loop_ref_count > 1) {
        event->loop_ref_count--;
        return;
    }

    async_signal_event_t *signal = (async_signal_event_t *)(event);

    const int error = uv_signal_stop(&signal->uv_handle);

    event->loop_ref_count = 0;
    ASYNC_G(active_event_count)--;

    if (error < 0) {
        async_throw_error("Failed to stop signal handle: %s", uv_strerror(error));
        return;
    }
}
/* }}} */

/* {{{ libuv_signal_dispose */
static void libuv_signal_dispose(zend_async_event_t *event)
{
    if (event->ref_count > 1) {
        event->ref_count--;
        return;
    }

    if (event->loop_ref_count > 0) {
        event->loop_ref_count = 1;
        libuv_signal_stop(event);
    }

    zend_async_callbacks_free(event);

    async_signal_event_t *signal = (async_signal_event_t *)(event);

    uv_close((uv_handle_t *)&signal->uv_handle, libuv_close_handle_cb);

    pefree(event, 0);
}
/* }}} */

/* {{{ libuv_new_signal_event */
zend_async_signal_event_t* libuv_new_signal_event(int signum)
{
    async_signal_event_t *signal = pecalloc(1, sizeof(async_signal_event_t), 0);

    const int error = uv_signal_init(UVLOOP, &signal->uv_handle);

    if (error < 0) {
        async_throw_error("Failed to initialize signal handle: %s", uv_strerror(error));
        pefree(signal, 0);
        return NULL;
    }

    signal->uv_handle.data = signal;
    signal->event.signal = signum;

    signal->event.base.add_callback = libuv_add_callback;
    signal->event.base.del_callback = libuv_remove_callback;
    signal->event.base.start = libuv_signal_start;
    signal->event.base.stop = libuv_signal_stop;
    signal->event.base.dispose = libuv_signal_dispose;

    return &signal->event;
}
/* }}} */

/* {{{ libuv_new_process_event */
void libuv_new_process_event(zend_async_process_event_t *process_event)
{
    //TODO: libuv_new_process_event
}
/* }}} */

/* {{{ libuv_new_thread_event */
void libuv_new_thread_event(zend_async_thread_event_t *thread_event)
{
    //TODO: libuv_new_thread_event
}
/* }}} */

/* {{{ libuv_new_filesystem_event */
void libuv_new_filesystem_event(zend_async_filesystem_event_t *filesystem_event)
{
    //TODO: libuv_new_filesystem_event
}
/* }}} */

void async_libuv_reactor_register(void)
{
	zend_string * module_name = zend_string_init(LIBUV_REACTOR_NAME, sizeof(LIBUV_REACTOR_NAME) - 1, 0);

	zend_async_reactor_register(
		module_name,
		false,
		libuv_reactor_startup,
		libuv_reactor_shutdown,
		libuv_reactor_execute,
		libuv_reactor_loop_alive,
		libuv_new_socket_event,
		libuv_new_poll_event,
		libuv_new_timer_event,
		libuv_new_signal_event,
		libuv_new_process_event,
		libuv_new_thread_event,
		libuv_new_filesystem_event
	);

	zend_string_release(module_name);
}