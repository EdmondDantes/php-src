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

#include "../php_layer/exceptions.h"

typedef struct
{
	uv_loop_t loop;
#ifdef PHP_WIN32
	uv_thread_t * watcherThread;
	HANDLE ioCompletionPort;
	HANDLE hWakeUpEvent;
	unsigned int countWaitingDescriptors;
	bool isRunning;
	uv_async_t uvloop_wakeup;
	/* Circular buffer of libuv_process_t ptr */
	circular_buffer_t * pid_queue;
#endif
} libuv_reactor_t;

#define UVLOOP ((uv_loop_t *) ASYNC_G(reactor))
#define LIBUV_REACTOR ((libuv_reactor_t *) ASYNC_G(reactor))
#define WATCHER ((libuv_reactor_t *) ASYNC_G(reactor))->watcherThread
#define IF_EXCEPTION_STOP if (UNEXPECTED(EG(exception) != NULL)) { reactor_stop_fn; }

void libuv_startup(void);

#define STARTUP_REACTOR_IF_NEED if (UNEXPECTED(UVLOOP == NULL)) {						\
		libuv_startup();																\
		if(UNEXPECTED(EG(exception) != NULL)) {											\
			return NULL;																\
		}																				\
    }

static async_microtasks_handler_t microtask_handler = NULL;
static async_next_fiber_handler_t next_fiber_handler = NULL;
static zend_object_handlers libuv_object_handlers;

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

static zend_always_inline libuv_poll_t * libuv_poll_new(
	zend_class_entry * class_entry,
	async_file_descriptor_t file,
	php_socket_t socket,
	zend_ulong events
	);

static reactor_handle_t* libuv_handle_from_resource(const zend_resource *resource, const zend_ulong actions, const REACTOR_HANDLE_TYPE expected_type)
{
	php_socket_t socket;
	async_file_descriptor_t file;

	async_resource_cast(resource, &socket, &file);

	if (socket == 0 && file == NULL) {
        async_throw_error("Expected a file descriptor or socket resource");
        return NULL;
    }

	if (expected_type == REACTOR_H_FILE && file == NULL) {
		async_throw_error("Expected a file descriptor resource");
		return NULL;
	} else if (expected_type == REACTOR_H_SOCKET && socket == 0) {
        async_throw_error("Expected a socket resource");
		return NULL;
    }

	// TODO: Support Win32 for async file operations
#ifdef PHP_WIN32
	if (file != NULL) {
		async_throw_error("Not supported async file operation for Windows");
		return NULL;
	}
#endif

	if (file != NULL) {
        return (reactor_handle_t *) libuv_poll_new(async_ce_file_handle, file, 0, actions);
    } else {
	    return (reactor_handle_t *) libuv_poll_new(async_ce_socket_handle, 0, socket, actions);
    }
}

//=============================================================
#pragma region Poll Handle
//=============================================================
static void on_poll_event(const uv_poll_t* handle, const int status, const int events)
{
	libuv_poll_t *poll = handle->data;
	zval error;
	ZVAL_NULL(&error);

	if (status < 0) {
		zend_object *exception = async_new_exception(
			async_ce_input_output_exception, "Input output error: %s", uv_strerror(status)
		);

		ZVAL_OBJ(&error, exception);
	}

	ZVAL_LONG(&poll->poll.triggered_events, libuv_events_to_php(events));

	async_notifier_notify(&poll->handle, &poll->poll.triggered_events, &error);
	zval_ptr_dtor(&error);
	IF_EXCEPTION_STOP;
}

static zend_always_inline void libuv_poll_init(libuv_poll_t * poll)
{
	poll->uv_handle = pecalloc(1, sizeof(uv_poll_t), 1);
	poll->reference_count = 0;

#ifdef PHP_WIN32
	if (poll->std.ce == async_ce_file_handle) {
		async_throw_error("File descriptor polling is not supported on Windows");
		pefree(poll->uv_handle, 1);
		poll->uv_handle = NULL;
		return;
	}

	int error = uv_poll_init_socket(UVLOOP, poll->uv_handle, poll->poll.socket);
#else
	int error = uv_poll_init(UVLOOP, poll->uv_handle, poll->poll.file);
#endif

	if (error < 0) {
		async_throw_error("Failed to initialize poll handle: %s", uv_strerror(error));
		pefree(poll->uv_handle, 1);
		poll->uv_handle = NULL;
		return;
	}

	// Link the handle to the loop.
	poll->uv_handle->data = poll;
}

static zend_always_inline libuv_poll_t * libuv_poll_new(
	zend_class_entry * class_entry,
	const async_file_descriptor_t file,
	const php_socket_t socket,
	const zend_ulong events
	)
{
	DEFINE_ZEND_INTERNAL_OBJECT(libuv_poll_t, object, class_entry);
	async_notifier_object_init(&object->handle);

	if (UNEXPECTED(object == NULL)) {
		return NULL;
	}

	if (file) {
		object->poll.file = file;
	} else {
		object->poll.socket = socket;
	}

	object->poll.events = (int) events;

	libuv_poll_init(object);

	if (UNEXPECTED(EG(exception))) {
		OBJ_RELEASE(&object->std);
		return NULL;
	}

	object->std.handlers = &libuv_object_handlers;

	return object;
}

static reactor_handle_t* libuv_file_new(const async_file_descriptor_t file, const zend_ulong events)
{
	return (reactor_handle_t *) libuv_poll_new(async_ce_file_handle, file, 0, events);
}

static reactor_handle_t* libuv_socket_new(const php_socket_t socket, const zend_ulong events)
{
	return (reactor_handle_t *) libuv_poll_new(async_ce_socket_handle, 0, socket, events);
}

static reactor_handle_t* libuv_pipe_new(const async_file_descriptor_t file, const zend_ulong events)
{
	return (reactor_handle_t *) libuv_poll_new(async_ce_pipe_handle, file, 0, events);
}

static reactor_handle_t* libuv_tty_new(const async_file_descriptor_t file, const zend_ulong events)
{
	return (reactor_handle_t *) libuv_poll_new(async_ce_tty_handle, file, 0, events);
}

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region Timer
//=============================================================

static void on_timer_event(uv_timer_t *handle)
{
	libuv_timer_t *timer = handle->data;

	zval error;
	ZVAL_NULL(&error);

	zval events;
	ZVAL_NULL(&events);

	async_notifier_notify(&timer->handle, &events, &error);
	IF_EXCEPTION_STOP;
}

static reactor_handle_t* libuv_timer_new(const zend_ulong timeout, const zend_bool is_periodic)
{
	STARTUP_REACTOR_IF_NEED

	if (timeout < 0) {
		zend_throw_exception(zend_ce_type_error, "Invalid timeout", 0);
		return NULL;
	}

	DEFINE_ZEND_INTERNAL_OBJECT(libuv_timer_t, object, async_ce_timer_handle);
	async_notifier_object_init(&object->handle);

	if (UNEXPECTED(object == NULL)) {
		return NULL;
	}

	object->uv_handle = pecalloc(1, sizeof(uv_timer_t), 1);
	object->reference_count = 0;

	if (UNEXPECTED(object->uv_handle == NULL)) {
		OBJ_RELEASE(&object->std);
		zend_throw_exception(zend_ce_type_error, "Failed to initialize timer handle", 0);
		return NULL;
	}

	int error = uv_timer_init(UVLOOP, object->uv_handle);

	if (error < 0) {
		async_throw_error("Failed to initialize timer handle: %s", uv_strerror(error));
		pefree(object->uv_handle, 1);
		OBJ_RELEASE(&object->std);
		return NULL;
	}

	object->uv_handle->data = object;

	ZVAL_LONG(&object->timer.microseconds, timeout);
	ZVAL_BOOL(&object->timer.is_periodic, is_periodic);

	object->std.handlers = &libuv_object_handlers;

	return (reactor_handle_t *) object;
}

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region Signal
//=============================================================

static void on_signal_event(uv_signal_t *handle, int sig_number)
{
	libuv_signal_t *signal = handle->data;

	zval error;
	ZVAL_NULL(&error);

	zval sig;
	ZVAL_LONG(&sig, sig_number);

	async_notifier_notify(&signal->handle, &sig, &error);
	IF_EXCEPTION_STOP;
}

static reactor_handle_t* libuv_signal_new(const zend_long sig_number)
{
	STARTUP_REACTOR_IF_NEED

	if (sig_number < 0) {
		zend_throw_exception(zend_ce_type_error, "Invalid signal number", 0);
		return NULL;
	}

	DEFINE_ZEND_INTERNAL_OBJECT(libuv_signal_t, object, async_ce_signal_handle);
	async_notifier_object_init(&object->handle);

	if (UNEXPECTED(object == NULL)) {
		return NULL;
	}

	object->uv_handle = pecalloc(1, sizeof(uv_signal_t), 1);
	object->reference_count = 0;

	if (UNEXPECTED(object->uv_handle == NULL)) {
		OBJ_RELEASE(&object->std);
		zend_throw_exception(zend_ce_type_error, "Failed to initialize signal handle", 0);
		return NULL;
	}

	int error = uv_signal_init(UVLOOP, object->uv_handle);

	if (error < 0) {
		async_throw_error("Failed to initialize signal handle: %s", uv_strerror(error));
		pefree(object->uv_handle, 1);
		OBJ_RELEASE(&object->std);
		return NULL;
	}

	ZVAL_LONG(&object->signal.number, sig_number);

	object->std.handlers = &libuv_object_handlers;

	return (reactor_handle_t *) object;
}

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region File System Events
//=============================================================

static void on_fs_event(uv_fs_event_t *handle, const char *filename, int events, int status)
{
	libuv_fs_event_t *fs_event = handle->data;

	zval error;
	ZVAL_NULL(&error);

	zval event;
	ZVAL_STRING(&event, filename);

	zval php_events;
	ZVAL_LONG(&php_events, events);

	async_notifier_notify(&fs_event->handle, &event, &error);
	IF_EXCEPTION_STOP;
}

static reactor_handle_t* libuv_file_system_new(const char *path, const size_t length, const zend_ulong flags)
{
	STARTUP_REACTOR_IF_NEED

	if (length == 0) {
		zend_throw_exception(zend_ce_type_error, "Invalid path", 0);
		return NULL;
	}

	DEFINE_ZEND_INTERNAL_OBJECT(libuv_fs_event_t, object, async_ce_file_system_handle);
	async_notifier_object_init(&object->handle);

	if (UNEXPECTED(object == NULL)) {
		return NULL;
	}

	object->uv_handle = pecalloc(1, sizeof(uv_fs_event_t), 1);
	object->reference_count = 0;

	if (UNEXPECTED(object->uv_handle == NULL)) {
		OBJ_RELEASE(&object->std);
		zend_throw_exception(zend_ce_type_error, "Failed to initialize file system event handle", 0);
		return NULL;
	}

	int error = uv_fs_event_init(UVLOOP, object->uv_handle);

	if (error < 0) {
		async_throw_error("Failed to initialize file system event handle: %s", uv_strerror(error));
		pefree(object->uv_handle, 1);
		OBJ_RELEASE(&object->std);
		return NULL;
	}

	ZVAL_STRINGL(&object->fs_event.path, path, length);
	ZVAL_LONG(&object->fs_event.flags, flags);

	object->std.handlers = &libuv_object_handlers;

	return (reactor_handle_t *) object;
}

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region Process handle
//=============================================================

static reactor_handle_t* libuv_process_new(const async_process_t process_h, const zend_ulong events)
{
	DEFINE_ZEND_INTERNAL_OBJECT(libuv_process_t, object, async_ce_process_handle);
	async_notifier_object_init(&object->handle);

	if (UNEXPECTED(object == NULL)) {
		return NULL;
	}

	object->hProcess = process_h;

	return (reactor_handle_t*) object;
}

#ifdef PHP_WIN32

static void process_watcher_thread(void * args)
{
	libuv_reactor_t *reactor = (libuv_reactor_t *) args;

	ULONG_PTR completionKey;

	while (reactor->isRunning && reactor->ioCompletionPort != NULL) {

		if (false == GetQueuedCompletionStatus(reactor->ioCompletionPort, NULL, &completionKey, NULL, INFINITE)) {
			break;
		}

		if (completionKey == (ULONG_PTR) reactor->hWakeUpEvent) {
			continue;
		}

		if (reactor->isRunning == false) {
            break;
        }

		libuv_process_t * process = (libuv_process_t *) completionKey;

		if (UNEXPECTED(circular_buffer_is_full(reactor->pid_queue))) {

			uv_async_send(&reactor->uvloop_wakeup);

			unsigned int delay = 1;

			while (reactor->isRunning && circular_buffer_is_full(reactor->pid_queue)) {
				usleep(delay);
				delay = MIN(delay << 1, 1000);
			}

			if (false == reactor->isRunning) {
				break;
			}
        }

		circular_buffer_push(reactor->pid_queue, &process, false);
		uv_async_send(&reactor->uvloop_wakeup);
	}
}

static void libuv_start_process_watcher(void);
static void libuv_stop_process_watcher(void);

static void on_process_event(uv_async_t *handle)
{
	libuv_reactor_t * reactor = LIBUV_REACTOR;

	if (circular_buffer_is_empty(reactor->pid_queue)) {
		return;
	}

	libuv_process_t * process;

	while (circular_buffer_is_not_empty(reactor->pid_queue)) {
		circular_buffer_pop(reactor->pid_queue, &process);

		zval event, error;
		ZVAL_NULL(&event);
		ZVAL_UNDEF(&error);

		if (reactor->countWaitingDescriptors > 0) {
			reactor->countWaitingDescriptors--;

			if (reactor->countWaitingDescriptors == 0) {
				libuv_stop_process_watcher();
			}
        }

		async_notifier_notify(&process->handle, &event, &error);
		IF_EXCEPTION_STOP;
	}
}

static void libuv_start_process_watcher(void)
{
	if (WATCHER != NULL) {
		return;
	}

	uv_thread_t *thread = pecalloc(1, sizeof(uv_thread_t), 0);

	if (thread == NULL) {
		return;
	}

	libuv_reactor_t * reactor = LIBUV_REACTOR;

	// Create IoCompletionPort
	reactor->ioCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);

	if (reactor->ioCompletionPort == NULL) {
		char * error_msg = php_win32_error_to_msg((HRESULT) GetLastError());
		php_error_docref(NULL, E_CORE_ERROR, "Failed to create IO completion port: %s", error_msg);
		php_win32_error_msg_free(error_msg);
		return;
	}

	// Wake up event
	reactor->hWakeUpEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (reactor->hWakeUpEvent == NULL) {
		char * error_msg = php_win32_error_to_msg((HRESULT) GetLastError());
		php_error_docref(NULL, E_CORE_ERROR, "Failed to create wake up event: %s", error_msg);
		php_win32_error_msg_free(error_msg);
		return;
	}

	if (CreateIoCompletionPort(
		reactor->hWakeUpEvent, reactor->ioCompletionPort, (ULONG_PTR)reactor->hWakeUpEvent, 0
	) == NULL) {
		char * error_msg = php_win32_error_to_msg((HRESULT) GetLastError());
		php_error_docref(NULL, E_CORE_ERROR, "Failed to create IO completion port for wake up event: %s", error_msg);
		php_win32_error_msg_free(error_msg);
		return;
	}

	reactor->isRunning = true;
	reactor->countWaitingDescriptors = 0;

	int error = uv_thread_create(thread, process_watcher_thread, reactor);

	if (error < 0) {
		uv_thread_detach(thread);
		pefree(thread, 0);
		reactor->isRunning = false;
		php_error_docref(NULL, E_CORE_ERROR, "Failed to create process watcher thread: %s", uv_strerror(error));
		return;
	}

	WATCHER = thread;

	error = uv_async_init(UVLOOP, &reactor->uvloop_wakeup, on_process_event);
	circular_buffer_ctor(reactor->pid_queue, 64, sizeof(libuv_process_t *), &zend_std_allocator);

	if (error < 0) {
		uv_thread_detach(thread);
		reactor->isRunning = false;
		pefree(thread, 0);
		WATCHER = NULL;
		php_error_docref(NULL, E_CORE_ERROR, "Failed to initialize async handle: %s", uv_strerror(error));
	}
}

static void libuv_stop_process_watcher(void)
{
	if (WATCHER == NULL) {
		return;
	}

	libuv_reactor_t * reactor = LIBUV_REACTOR;

	reactor->isRunning = false;

	// send wake up event to stop the thread
	PostQueuedCompletionStatus(reactor->ioCompletionPort, 0, (ULONG_PTR)reactor->hWakeUpEvent, NULL);
	uv_thread_detach(WATCHER);
	pefree(WATCHER, 0);
	WATCHER = NULL;

	// Stop IO completion port
	CloseHandle(reactor->ioCompletionPort);
	reactor->ioCompletionPort = NULL;

	// Stop wake up event
	CloseHandle(reactor->hWakeUpEvent);
	reactor->hWakeUpEvent = NULL;

	// Stop circular buffer
	circular_buffer_destroy(reactor->pid_queue);
	reactor->pid_queue = NULL;
}

static void libuv_add_process_handle(reactor_handle_t *handle)
{
	libuv_process_t *process = (libuv_process_t *) handle;

	if (process->hJob != NULL) {
		return;
	}

	if (WATCHER == NULL) {
		libuv_start_process_watcher();
	}

	process->hJob = CreateJobObject(NULL, NULL);

	if (AssignProcessToJobObject(process->hJob, process->hProcess) == 0) {
		char * error_msg = php_win32_error_to_msg((HRESULT) GetLastError());
		async_throw_error("Failed to assign process to job object: %s", error_msg);
		php_win32_error_msg_free(error_msg);
		return;
	}

	if (CreateIoCompletionPort(
		process->hJob, LIBUV_REACTOR->ioCompletionPort, (ULONG_PTR)process, 0
	) == NULL) {
		CloseHandle(process->hJob);
		char * error_msg = php_win32_error_to_msg((HRESULT) GetLastError());
		async_throw_error("Failed to create IO completion port for process: %s", error_msg);
		php_win32_error_msg_free(error_msg);
	}

	LIBUV_REACTOR->countWaitingDescriptors++;
}

static void libuv_remove_process_handle(reactor_handle_t *handle)
{
	libuv_process_t *process = (libuv_process_t *) handle;

	if (process->hJob != NULL) {
		CloseHandle(process->hJob);
		process->hJob = NULL;
	}
}

#else

// Unix process handle

static void libuv_add_process_handle(reactor_handle_t *handle)
{
	libuv_process_t *process = (libuv_process_t *) handle;

}

static void libuv_remove_process_handle(reactor_handle_t *handle)
{
	libuv_process_t *process = (libuv_process_t *) handle;

}

#endif

static void libuv_close_process_handle(reactor_handle_t *handle)
{

}

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region Thread handle
//=============================================================

static reactor_handle_t* libuv_thread_new(const THREAD_T tid, const zend_ulong events)
{
	return NULL;
}

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region Handle add/remove/destroy API
//=============================================================

static void libuv_add_handle(reactor_handle_t *handle)
{
	zend_object * object = &handle->std;

	if (object->ce == async_ce_file_handle
		|| object->ce == async_ce_socket_handle
		|| object->ce == async_ce_pipe_handle
		|| object->ce == async_ce_tty_handle) {

		libuv_poll_t *poll = (libuv_poll_t *)object;

		if (poll->uv_handle == NULL || poll->reference_count > 0) {
			poll->reference_count++;
            return;
        }

		const int error = uv_poll_start(poll->uv_handle, poll->poll.events, on_poll_event);

		if (error < 0) {
			async_throw_error("Failed to start poll handle: %s", uv_strerror(error));
		}

		poll->reference_count = 1;

    } else if (object->ce == async_ce_timer_handle) {

    	libuv_timer_t *timer = (libuv_timer_t *)object;

    	if (timer->uv_handle == NULL || timer->reference_count > 0) {
			timer->reference_count++;
    		return;
    	}

    	const int error = uv_timer_start(
    		timer->uv_handle,
    		on_timer_event,
    		Z_LVAL(timer->timer.microseconds),
    		Z_TYPE(timer->timer.is_periodic) == IS_TRUE ? Z_LVAL(timer->timer.microseconds) : 0
		);

    	if (error < 0) {
    		async_throw_error("Failed to start timer handle: %s", uv_strerror(error));
    	}

    	timer->reference_count = 1;

    } else if (object->ce == async_ce_signal_handle) {

		libuv_signal_t *signal = (libuv_signal_t *)object;

        if (signal->uv_handle == NULL || signal->reference_count > 0) {
			signal->reference_count++;
        	return;
        }

    	const int error = uv_signal_start(signal->uv_handle, on_signal_event, (int) Z_LVAL(signal->signal.number));

    	if (error < 0) {
    		async_throw_error("Failed to start signal handle: %s", uv_strerror(error));
    	}

    	signal->reference_count = 1;

    } else if (object->ce == async_ce_file_system_handle) {

		libuv_fs_event_t *fs_event = (libuv_fs_event_t *)object;

        if (fs_event->uv_handle == NULL || fs_event->reference_count > 0) {
        	fs_event->reference_count++;
        	return;
        }

    	const int error = uv_fs_event_start(
    		fs_event->uv_handle, on_fs_event, Z_STRVAL(fs_event->fs_event.path), Z_LVAL(fs_event->fs_event.flags)
		);

    	if (error < 0) {
    		async_throw_error("Failed to start file system event handle: %s", uv_strerror(error));
    	}

    	fs_event->reference_count = 1;

    } else if (object->ce == async_ce_process_handle) {
    	libuv_add_process_handle(handle);
    } else if (object->ce == async_ce_thread_handle) {

    }
}

static void libuv_remove_handle(reactor_handle_t *handle)
{
	zend_object *object = &handle->std;

	if (object->ce == async_ce_file_handle
		|| object->ce == async_ce_socket_handle
		|| object->ce == async_ce_pipe_handle
		|| object->ce == async_ce_tty_handle) {

		libuv_poll_t *poll = (libuv_poll_t *)object;

		if (poll->uv_handle == NULL || poll->reference_count > 1) {
			poll->reference_count--;
            return;
        }

		const int error = uv_poll_stop(poll->uv_handle);

		if (error < 0) {
			async_warning("Failed to stop poll handle: %s", uv_strerror(error));
		}

		poll->reference_count = 0;

    } else if (object->ce == async_ce_timer_handle) {

    	libuv_timer_t *timer = (libuv_timer_t *)object;

    	if (timer->uv_handle == NULL || timer->reference_count > 1) {
    		timer->reference_count--;
    		return;
    	}

		const int error = uv_timer_stop(timer->uv_handle);

		if (error < 0) {
			async_warning("Failed to stop timer handle: %s", uv_strerror(error));
		}

    	timer->reference_count = 0;

    } else if (object->ce == async_ce_signal_handle) {

		libuv_signal_t *signal = (libuv_signal_t *)object;

        if (signal->uv_handle == NULL || signal->reference_count > 1) {
        	signal->reference_count--;
        	return;
        }

		const int error = uv_signal_stop(signal->uv_handle);

        if (error < 0) {
            async_warning("Failed to stop signal handle: %s", uv_strerror(error));
        }

    	signal->reference_count = 0;

    } else if (object->ce == async_ce_file_system_handle) {

		libuv_fs_event_t *fs_event = (libuv_fs_event_t *)object;

        if (fs_event->uv_handle == NULL || fs_event->reference_count > 1) {
        	return;
        }

		const int error = uv_fs_event_stop(fs_event->uv_handle);

		if (error < 0) {
            async_warning("Failed to stop file system event handle: %s", uv_strerror(error));
        }

    	fs_event->reference_count = 0;

    } else if (object->ce == async_ce_process_handle) {
		libuv_remove_process_handle(handle);
    } else if (object->ce == async_ce_thread_handle) {

    }
}

static bool libuv_is_listening(reactor_handle_t *handle)
{
	const zend_object *object = &handle->std;

	if (object->ce == async_ce_file_handle
		|| object->ce == async_ce_socket_handle
		|| object->ce == async_ce_pipe_handle
		|| object->ce == async_ce_tty_handle)
	{
		return ((libuv_poll_t *) handle)->reference_count;
	} else if (object->ce == async_ce_signal_handle) {
		return ((libuv_signal_t *) handle)->reference_count;
	} else if (object->ce == async_ce_file_system_handle) {
		return ((libuv_fs_event_t *) handle)->reference_count;
	}

	return false;
}

static void libuv_close_handle(reactor_handle_t *handle)
{
	zend_object *object = &handle->std;
	uv_handle_t * uv_handle = NULL;

	if (object->ce == async_ce_file_handle
		|| object->ce == async_ce_socket_handle
		|| object->ce == async_ce_pipe_handle
		|| object->ce == async_ce_tty_handle) {

		libuv_poll_t *poll = (libuv_poll_t *)object;

		if (poll->uv_handle == NULL) {
			return;
		}

		uv_handle = (uv_handle_t *) poll->uv_handle;
		poll->uv_handle = NULL;

	} else if (object->ce == async_ce_timer_handle) {

		libuv_timer_t *timer = (libuv_timer_t *)object;

		if (timer->uv_handle == NULL) {
			return;
		}

		uv_handle = (uv_handle_t *) timer->uv_handle;

	} else if (object->ce == async_ce_signal_handle) {

		libuv_signal_t *signal = (libuv_signal_t *)object;

		if (signal->uv_handle == NULL) {
			return;
		}

		uv_handle = (uv_handle_t *) signal->uv_handle;

	} else if (object->ce == async_ce_file_system_handle) {

		libuv_fs_event_t *fs_event = (libuv_fs_event_t *)object;

		if (fs_event->uv_handle == NULL) {
			return;
		}

		uv_handle = (uv_handle_t *) fs_event->uv_handle;

	} else if (object->ce == async_ce_process_handle) {
		libuv_close_process_handle(handle);
	} else if (object->ce == async_ce_thread_handle) {

	} else if (object->ce == async_ce_dns_info) {

		libuv_dns_info_t *dns_info = (libuv_dns_info_t *)object;

		if (dns_info->addr_info != NULL) {
			if (dns_info->is_addr_info) {
				uv_freeaddrinfo(dns_info->addr_info->addrinfo);
			} else {

			}

			efree(dns_info->addr_info);
		}

		return;
    }

	if (uv_handle != NULL) {
		uv_close(uv_handle, libuv_close_cb);
	}
}

static void libuv_object_destroy(zend_object *object)
{
	libuv_remove_handle((reactor_handle_t *) object);
	libuv_close_handle((reactor_handle_t *) object);
	async_ce_notifier->default_object_handlers->dtor_obj(object);
}

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region Reactor API
//=============================================================

static void libuv_startup(void)
{
	zend_async_globals *async_globals = ASYNC_GLOBAL;

	if (async_globals->reactor != NULL) {
		return;
	}

	async_globals->reactor = pecalloc(1, sizeof(libuv_reactor_t), 1);
	const int result = uv_loop_init(async_globals->reactor);

	if (result != 0) {
		async_throw_error("Failed to initialize loop: %s", uv_strerror(result));
		return;
	}

	uv_loop_set_data(async_globals->reactor, ASYNC_GLOBAL);
}

static void libuv_shutdown(void)
{
	zend_async_globals *async_globals = ASYNC_GLOBAL;

	if (EXPECTED(async_globals->reactor != NULL)) {
		uv_loop_close(async_globals->reactor);
		pefree(async_globals->reactor, 1);
		async_globals->reactor = NULL;
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
	if (UVLOOP == NULL) {
		return false;
	}

    return uv_loop_alive(UVLOOP);
}

//=============================================================
#pragma region Handle API
//=============================================================

/**
 * Previous handlers.
 */
static reactor_startup_t prev_reactor_startup_fn = NULL;
static reactor_shutdown_t prev_reactor_shutdown_fn = NULL;

static reactor_is_active_method_t prev_reactor_is_active_fn = NULL;
static reactor_handle_method_t prev_reactor_add_handle_ex_fn = NULL;
static reactor_handle_method_t prev_reactor_remove_handle_fn = NULL;
static reactor_is_listening_method_t prev_reactor_is_listening_fn = NULL;

static reactor_stop_t prev_reactor_loop_stop_fn = NULL;
static reactor_loop_alive_t prev_reactor_loop_alive_fn = NULL;

static reactor_handle_from_resource_t prev_reactor_handle_from_resource_fn = NULL;
static reactor_file_new_t prev_reactor_file_new_fn = NULL;
static reactor_socket_new_t prev_reactor_socket_new_fn = NULL;
static reactor_pipe_new_t prev_reactor_pipe_new_fn = NULL;
static reactor_tty_new_t prev_reactor_tty_new_fn = NULL;

static reactor_timer_new_t prev_reactor_timer_new_fn = NULL;
static reactor_signal_new_t prev_reactor_signal_new_fn = NULL;
static reactor_file_system_new_t prev_reactor_file_system_new_fn = NULL;

static reactor_process_new_t prev_reactor_process_new_fn = NULL;
static reactor_thread_new_t prev_reactor_thread_new_fn = NULL;
static reactor_dns_info_new_t prev_reactor_dns_info_new_fn = NULL;
static reactor_dns_info_cancel_t prev_reactor_dns_info_cancel_fn = NULL;

//=============================================================
#pragma endregion
//=============================================================

//=============================================================
#pragma region Getaddrinfo
//=============================================================
static void addr_on_resolved(uv_getaddrinfo_t *req, const int status, struct addrinfo *res)
{
	libuv_dns_info_t * dns_handle = req->data;

	zval error;
	ZVAL_NULL(&error);

	if (Z_TYPE(dns_handle->dns_info.address) == IS_STRING) {
        zend_string_release(Z_STR(dns_handle->dns_info.address));
    }

	ZVAL_NULL(&dns_handle->dns_info.address);

	if (status == UV_ECANCELED) {
		// No need to call async_notifier_notify if the request is canceled.
		uv_freeaddrinfo(res);
		OBJ_RELEASE(&dns_handle->handle.std);
		return;
	}

	if (status < 0) {
		zend_object *exception = async_new_exception(
			async_ce_input_output_exception, "async getaddrinfo error: %s", uv_strerror(status)
		);

		ZVAL_OBJ(&error, exception);
	} else {
		dns_handle->dns_info.addr_info = res;
	}

	zval z_null;
	ZVAL_NULL(&z_null);
	async_notifier_notify(&dns_handle->handle, &z_null, &error);
	zval_ptr_dtor(&error);
	OBJ_RELEASE(&dns_handle->handle.std);
}

static void host_on_resolved(const uv_getnameinfo_t* req, const int status, const char* hostname, const char* service)
{
	libuv_dns_info_t * dns_handle = req->data;

	zval error;
	ZVAL_NULL(&error);

	if (status == UV_ECANCELED) {
		// No need to call async_notifier_notify if the request is canceled.
		OBJ_RELEASE(&dns_handle->handle.std);
		return;
	}

	if (status < 0) {
		zend_object *exception = async_new_exception(
			async_ce_input_output_exception, "async getnameinfo error: %s", uv_strerror(status)
		);

		ZVAL_OBJ(&error, exception);
	} else {
	    ZVAL_STRING(&dns_handle->dns_info.host, hostname);
	}

	zval z_null;
	ZVAL_NULL(&z_null);
	async_notifier_notify(&dns_handle->handle, &z_null, &error);
	zval_ptr_dtor(&error);
	OBJ_RELEASE(&dns_handle->handle.std);
}

static reactor_handle_t * libuv_dns_info_new(
	zend_string * host,
	zend_string * service,
	zend_string * address,
	struct addrinfo * hints
)
{
	struct sockaddr_storage addr_storage;

	if (address != NULL) {
		if (inet_pton(AF_INET, ZSTR_VAL(address), &((struct sockaddr_in*)&addr_storage)->sin_addr) == 1) {
			uv_ip4_addr(ZSTR_VAL(address), 0, (struct sockaddr_in*)&addr_storage);
		} else if (inet_pton(AF_INET6, ZSTR_VAL(address), &((struct sockaddr_in6*)&addr_storage)->sin6_addr) == 1) {
			uv_ip6_addr(ZSTR_VAL(address), 0, (struct sockaddr_in6*)&addr_storage);
		} else {
			async_throw_error("Invalid IP address format: %s", ZSTR_VAL(address));
			return NULL;
		}
	}

	DEFINE_ZEND_INTERNAL_OBJECT(libuv_dns_info_t, dns_handle, async_ce_dns_info);
	async_notifier_object_init(&dns_handle->handle);
	dns_handle->std.handlers = &libuv_object_handlers;

	bool hints_owned = false;

	if (host != NULL) {
		ZVAL_STR(&dns_handle->dns_info.host, zend_string_copy(host));
	}

	if (address != NULL) {
		ZVAL_STR(&dns_handle->dns_info.address, zend_string_copy(address));
	}

	if (hints == NULL) {
		hints_owned = true;
		hints = emalloc(sizeof(struct addrinfo));
		memset(hints, 0, sizeof(struct addrinfo));
		hints->ai_family = AF_UNSPEC;
	}

	const char * c_host = host != NULL ? ZSTR_VAL(host) : NULL;
	const char * c_service = service != NULL ? ZSTR_VAL(service) : NULL;

	int result = 0;

	// Increase the reference count to prevent the object from being destroyed before the callback is executed.
	// The object will be released in the callback function.
	// Please see https://docs.libuv.org/en/v1.x/request.html#c.uv_cancel
	GC_ADDREF(&dns_handle->handle.std);

	if (address != NULL) {
		dns_handle->is_addr_info = false;
		dns_handle->name_info = emalloc(sizeof(uv_getnameinfo_t));
		dns_handle->name_info->data = dns_handle;

		result = uv_getnameinfo(
			UVLOOP, dns_handle->name_info, host_on_resolved, (const struct sockaddr*) &addr_storage, 0
		);
	} else {
		dns_handle->is_addr_info = true;
		dns_handle->addr_info = emalloc(sizeof(uv_getaddrinfo_t));
		dns_handle->addr_info->data = dns_handle;

		result = uv_getaddrinfo(UVLOOP, dns_handle->addr_info, addr_on_resolved, c_host, c_service, hints);
	}

	if (hints_owned) {
		efree(hints);
	}

	if (result) {
		async_throw_error("Dns info error: %s", uv_strerror(result));
		// Release the reference count if the request failed after the GC_ADDREF call.
		GC_DELREF(&dns_handle->handle.std);
		// Release the object if the request failed.
		OBJ_RELEASE(&dns_handle->handle.std);
	}

	return (reactor_handle_t *) dns_handle;
}

static void libuv_dns_info_cancel(reactor_handle_t *handle)
{
	libuv_dns_info_t *dns_handle = (libuv_dns_info_t *)handle;

	if (dns_handle->is_cancelled) {
		return;
	}

	dns_handle->is_cancelled = true;

	if (dns_handle->is_addr_info) {
		uv_cancel((uv_req_t *)dns_handle->addr_info);
	} else {
		uv_cancel((uv_req_t *)dns_handle->name_info);
	}
}

//=============================================================
#pragma endregion
//=============================================================

static void setup_handlers(void)
{
	async_scheduler_set_callbacks_handler(execute_callbacks);

	prev_reactor_startup_fn = reactor_startup_fn;
	reactor_startup_fn = libuv_startup;

	prev_reactor_shutdown_fn = reactor_shutdown_fn;
	reactor_shutdown_fn = libuv_shutdown;

	prev_reactor_loop_stop_fn = reactor_stop_fn;
	reactor_stop_fn = libuv_loop_stop;

	prev_reactor_loop_alive_fn = reactor_loop_alive_fn;
	reactor_loop_alive_fn = libuv_loop_alive;

	prev_reactor_is_active_fn = reactor_is_active_fn;
	reactor_is_active_fn = libuv_loop_alive;

	prev_reactor_add_handle_ex_fn = reactor_add_handle_ex_fn;
	reactor_add_handle_ex_fn = libuv_add_handle;

	prev_reactor_remove_handle_fn = reactor_remove_handle_fn;
	reactor_remove_handle_fn = libuv_remove_handle;

	prev_reactor_is_listening_fn = reactor_is_listening_fn;
	reactor_is_listening_fn = libuv_is_listening;

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

	prev_reactor_dns_info_new_fn = reactor_dns_info_new_fn;
	reactor_dns_info_new_fn = libuv_dns_info_new;

	prev_reactor_dns_info_cancel_fn = reactor_dns_info_cancel_fn;
	reactor_dns_info_cancel_fn = libuv_dns_info_cancel;

	prev_reactor_file_system_new_fn = reactor_file_system_new_fn;
	reactor_file_system_new_fn = libuv_file_system_new;
}

static void restore_handlers(void)
{
	reactor_startup_fn = prev_reactor_startup_fn;
	reactor_shutdown_fn = prev_reactor_shutdown_fn;

	reactor_is_active_fn = prev_reactor_is_active_fn;
	reactor_add_handle_ex_fn = prev_reactor_add_handle_ex_fn;
	reactor_remove_handle_fn = prev_reactor_remove_handle_fn;
	reactor_is_listening_fn = prev_reactor_is_listening_fn;

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
	reactor_dns_info_new_fn = prev_reactor_dns_info_new_fn;
	reactor_dns_info_cancel_fn = prev_reactor_dns_info_cancel_fn;

	reactor_file_system_new_fn = prev_reactor_file_system_new_fn;
}

void async_libuv_startup(void)
{
	setup_handlers();

	libuv_object_handlers = *async_ce_notifier->default_object_handlers;
	libuv_object_handlers.dtor_obj = libuv_object_destroy;
}

void async_libuv_shutdown(void)
{
	restore_handlers();
}

//=============================================================
#pragma endregion
//=============================================================
