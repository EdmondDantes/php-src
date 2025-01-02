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
#include "php.h"
#include "php_async.h"

#include <php_network.h>
#include <zend_fibers.h>

#include "php_reactor.h"
#include "php_reactor.h"
#include "php_scheduler.h"
#include "php_layer/functions.h"
#include "php_layer/notifier.h"
#include "php_layer/ev_handles.h"
#include "php_layer/exceptions.h"

#ifdef PHP_SOCKETS
#include "ext/sockets/php_sockets.h"
#endif

/* Async global */
#ifdef ZTS
int async_globals_id = 0;
size_t async_globals_offset;
#else
async_globals_t* async_globals;
#endif

//===============================================================
#pragma region Startup and Shutdown
//===============================================================

static void async_globals_ctor(async_globals_t * globals)
{
	globals->is_async = false;
	globals->in_scheduler_context = false;
	globals->reactor = NULL;
	globals->exception_handler = NULL;
	globals->execute_callbacks_handler = NULL;
	globals->execute_next_fiber_handler = NULL;
	globals->execute_microtasks_handler = NULL;
}

static void async_globals_dtor(async_globals_t * globals)
{

}

/**
 * Activate the scheduler context.
 */
static void async_globals_startup(void)
{
	size_t globals_size = sizeof(async_globals_t);

#ifdef ZTS

	if (async_globals_id != 0) {
		return;
	}

	ts_allocate_fast_id(
		&async_globals_id,
		&async_globals_offset,
		globals_size,
		async_globals_ctor,
		async_globals_dtor
	);

	async_globals_t *async_globals = ts_resource(async_globals_id);
#else
	if (async_globals != NULL) {
        return;
    }

	async_globals = ecalloc(1, globals_size, 1);
	async_globals_ctor(async_globals);
#endif
}

/**
 * Activate the scheduler context.
 */
static void async_globals_shutdown(void)
{
#ifdef ZTS
	if (async_globals_id == 0) {
        return;
    }

	ts_free_id(async_globals_id);
	async_globals_id = 0;
#else
	if (async_globals == NULL) {
        return;
    }

	async_globals_dtor(async_globals);
	pefree(async_globals, 1);
#endif
}

/**
 * Async startup function.
 */
void async_module_startup(void)
{
	if (async_register_module() == FAILURE) {
		zend_error(E_CORE_WARNING, "Failed to register the 'True Asynchrony' module.");
	}

	async_globals_startup();
}

/**
 * Async shutdown function.
 */
void async_module_shutdown(void)
{
	async_globals_shutdown();
}

//===============================================================
#pragma endregion
//===============================================================

/**
 * Copy of zend_fetch_resource2
 *
 * @param resource - Resource to fetch
 * @return - Stream pointer
 */
static zend_always_inline php_stream * resource_to_stream(const zend_resource *resource)
{
	if (php_file_le_stream() == resource->type || php_file_le_pstream() == resource->type) {
		return resource->ptr;
	}

	return NULL;
}

void async_resource_to_fd(const zend_resource *resource, php_socket_t *socket, async_file_descriptor_t *file)
{
	php_stream *stream = resource_to_stream(resource);

	if (stream == NULL) {
		async_throw_error("Invalid resource type. Expected a stream of STDIO or Socket.");
		return;
	}

	if (php_stream_is(stream, PHP_STREAM_IS_SOCKET)) {

		if (php_stream_cast(stream, PHP_STREAM_AS_SOCKETD, &socket, false) == FAILURE) {
			async_throw_error("Failed to cast the stream to a socket descriptor.");
		}

	} else if (php_stream_is(stream, PHP_STREAM_IS_STDIO)) {

#ifdef PHP_WIN32
		async_throw_error("Not supported async file operation for Windows");
		return;
#else
		if (php_stream_cast(stream, PHP_STREAM_AS_SOCKETD, file, false) == FAILURE) {
			async_throw_error("Failed to cast the stream to a file descriptor.");
		}
#endif
	} else {
		async_throw_error("Invalid resource type. Expected a stream of STDIO or Socket.");
	}
}

php_socket_t async_try_extract_socket_object(zend_object * object)
{
#ifndef PHP_SOCKETS
	return 0;
#else
	if (UNEXPECTED(object->ce != socket_ce)) {
		return 0;
	}

	php_socket *socket = socket_from_obj(object);
	return (php_socket_t) socket->bsd_socket;
#endif
}

/**
 * Fetches the fiber state for the given fiber.
 */
ZEND_API async_fiber_state_t * async_find_fiber_state(const zend_fiber *fiber)
{
	if (IS_ASYNC_OFF) {
		return NULL;
	}

	zval *result = zend_hash_index_find(&ASYNC_G(fibers_state), fiber->std.handle);

	if (UNEXPECTED(result == NULL)) {
		return NULL;
	}

	async_fiber_state_t *resume = (async_fiber_state_t *) Z_PTR_P(result);
	zval_ptr_dtor(result);
	return resume;
}

static async_fiber_state_t * async_add_fiber_state(zend_fiber * fiber, async_resume_t *resume)
{
	async_fiber_state_t * state = pecalloc(1, sizeof(async_fiber_state_t), 1);

	if (state == NULL) {
        return NULL;
    }

	state->fiber = fiber;
	state->resume = resume;

	// Add reference to the fiber object and the resume object.
	GC_ADDREF(&fiber->std);

	if (resume != NULL) {
        GC_ADDREF(&resume->std);
    }

	zval zv;
	ZVAL_PTR(&zv, state);

	zend_hash_index_update(&ASYNC_G(fibers_state), fiber->std.handle, &zv);

	return state;
}

void async_resume_fiber(async_resume_t *resume, zval* result, zend_object* error)
{
	if (UNEXPECTED(EG(active_fiber) == resume->fiber)) {
		async_throw_error("Cannot resume the current Fiber.");
		return;
	}

	const bool is_pending = resume->status == ASYNC_RESUME_PENDING || resume->status == ASYNC_RESUME_NO_STATUS;

	if (Z_TYPE(resume->result) != IS_UNDEF) {
		ZVAL_PTR_DTOR(&resume->result);
	}

	ZVAL_UNDEF(&resume->result);

	if (resume->error != NULL) {
		OBJ_RELEASE(resume->error);
	}

	resume->error = NULL;

	if (EXPECTED(error == NULL)) {
		resume->status = ASYNC_RESUME_SUCCESS;

		if (result != NULL) {
			zval_copy(&resume->result, result);
		} else {
			zval_null(&resume->result);
		}
	} else {
		resume->status = ASYNC_RESUME_ERROR;
		GC_ADDREF(resume->error);
		resume->error = error;
	}

	if (EXPECTED(is_pending)) {
		if (UNEXPECTED(circular_buffer_push(&ASYNC_G(pending_fibers), resume, true) == FAILURE)) {
			async_throw_error("Failed to push the Fiber into the pending queue.");
			return;
		} else {
			GC_ADDREF(&resume->std);
		}
	}
}

void async_cancel_fiber(const zend_fiber *fiber, zend_object *error)
{
	const async_fiber_state_t *state = async_find_fiber_state(fiber);

	if (state == NULL || state->resume == NULL) {
		async_throw_error("The fiber is not waiting for asynchronous operations and cannot be terminated.");
		return;
	}

	async_resume_fiber(state->resume, NULL, error);
}

void async_transfer_throw_to_fiber(zend_fiber *fiber, zend_object *error)
{
	if (fiber == NULL) {
		return;
	}

	// Add fiber state if it does not exist.
	const async_fiber_state_t *state = async_find_fiber_state(fiber);

	if (state == NULL) {
		state = async_add_fiber_state(fiber, async_resume_new(fiber));
	}

	// Inherit exception from state-fiber if exists.
	if (state->resume->error != NULL) {
		zend_exception_set_previous(error, state->resume->error);
		zend_object_ptr_reset(state->resume->error);
	}

	// Move fiber to pending queue with exception.
	async_resume_fiber(state->resume, NULL, error);
}

/**
 * Suspend the current fiber.
 * The method puts the current Fiber into a waiting state for descriptors and events described by the Resume object.
 * Calling this method essentially returns control to the Scheduler.
 *
 * @param resume - Resume object. If NULL, a new object will be created.
 */
void async_await(async_resume_t *resume)
{
	if (UNEXPECTED(IS_ASYNC_OFF)) {
		return;
	}

	if (ASYNC_G(in_scheduler_context)) {
		async_throw_error("Cannot await in the scheduler context");
		return;
	}

	if (EG(active_fiber) == NULL) {
		zend_error(E_CORE_WARNING, "Cannot await in a non-fiber context");
        return;
    }

	bool is_owned_resume = false;

	if (resume == NULL) {
		is_owned_resume = true;
		resume = async_resume_new(NULL);
	}

	if (resume == NULL) {
		async_throw_error("Failed to create a new Resume object");
		goto finally;
	}

	if (resume->status != ASYNC_RESUME_PENDING) {
		async_throw_error("Attempt to use a Resume object that is not in the ASYNC_RESUME_PENDING state.");
		goto finally;
    }

	if (resume->fiber != EG(active_fiber)) {
		async_throw_error("Attempt to use a Resume object that is not associated with the current Fiber.");
	    goto finally;
    }

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

	if (state == NULL) {
		state = async_add_fiber_state(resume->fiber, NULL);

		if (state == NULL) {
			async_throw_error("Failed to create Fiber state");
			goto finally;
        }
	}

	if (state->resume != NULL) {
		async_throw_error("Attempt to stop a Fiber that already has a Resume object.");
		goto finally;
	}

	GC_ADDREF(&resume->std);
	state->resume = resume;

	zval *notifier;

	ZEND_HASH_FOREACH_VAL(&resume->notifiers, notifier)
		if (Z_TYPE_P(notifier) == IS_OBJECT) {

			reactor_add_handle((reactor_handle_t *) Z_OBJ_P(notifier));

			if (EG(exception) != NULL) {
				goto finally;
			}
		}
	ZEND_HASH_FOREACH_END();

	async_resume_pending(resume);

	zend_fiber_suspend(EG(active_fiber), NULL, NULL);

finally:

	if (is_owned_resume) {
        GC_DELREF(&resume->std);
    }
}

void async_build_resume_with(zend_ulong timeout, reactor_notifier_t * cancellation)
{

}

void async_await_socket()
{

}

/**
 * The method stops Fiber execution for a Zend resource.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_await.
 */
void async_await_resource(
	zend_resource * resource, const zend_ulong actions, const zend_ulong timeout, reactor_notifier_t * cancellation
)
{
	const reactor_handle_t *handle = reactor_handle_from_resource_fn(resource, actions);

	if (handle == NULL) {
		async_throw_error("I can't create an event handle from the resource.");
		return;
	}

	async_resume_t *resume = async_resume_new(NULL);

	if(resume == NULL) {
		async_throw_error("Failed to create a new Resume object");
        return;
    }

	// Add timer handle if a timeout is specified.
	if (timeout > 0) {
		async_resume_when(resume, reactor_timer_new_fn(timeout), async_resume_when_callback_cancel);
	}

	// Add cancellation handle if it is specified.
	if (cancellation != NULL) {
        async_resume_when(resume, cancellation, async_resume_when_callback_cancel);
    }

	async_await(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}

/**
 * The method stops Fiber execution for a specified signal.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_await.
 */
void async_await_signal(const zend_long sig_number, reactor_notifier_t * cancellation)
{
	async_resume_t *resume = async_resume_new(NULL);

	async_resume_when(resume, reactor_signal_new_fn(sig_number), async_resume_when_callback_resolve);

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, async_resume_when_callback_cancel);
	}

	async_await(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}

//===============================================================
#pragma region POLL2 EMULATION
//===============================================================

/**
 * The method stops Fiber execution for a specified time.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_await.
 * It's like a sleep()/usleep() function.
 */
void async_await_timeout(const zend_ulong timeout, reactor_notifier_t * cancellation)
{
	if (timeout == 0) {
		async_await(NULL);
		return;
	}

	async_resume_t *resume = async_resume_new(NULL);

	async_resume_when(resume, reactor_timer_new_fn(timeout), async_resume_when_callback_resolve);

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, async_resume_when_callback_cancel);
	}

	async_await(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}

static zend_always_inline zend_ulong poll2_events_to_async(const short events)
{
	zend_long result = 0;

	if (events & POLLIN) {
		result |= ASYNC_READABLE;
	}

	if (events & POLLOUT) {
		result |= ASYNC_WRITABLE;
	}

	if (events & POLLHUP) {
		result |= ASYNC_DISCONNECT;
	}

	if (events & POLLPRI) {
		result |= ASYNC_PRIORITIZED;
	}

	if (events & POLLERR) {
		result |= ASYNC_READABLE;
	}

	if (events & POLLNVAL) {
		result |= ASYNC_READABLE;
	}

	return result;
}

static zend_always_inline short async_events_to_poll2(const zend_ulong events)
{
	short result = 0;

	if (events & ASYNC_READABLE) {
		result |= POLLIN;
	}

	if (events & ASYNC_WRITABLE) {
		result |= POLLOUT;
	}

	if (events & ASYNC_DISCONNECT) {
		result |= POLLHUP;
	}

	if (events & ASYNC_PRIORITIZED) {
		result |= POLLPRI;
	}

	return result;
}

#define IF_EXCEPTION_GOTO_ERROR \
    if (UNEXPECTED(EG(exception) != NULL)) { \
        goto error; \
    }

int async_poll2(php_pollfd *ufds, unsigned int nfds, const int timeout)
{
	int result = 0;
	async_resume_t *resume = async_resume_new(NULL);

	if(resume == NULL) {
		errno = ENOMEM;
		return -1;
	}

	reactor_handle_t *handle = NULL;

	if (timeout > 0) {
		handle = reactor_timer_new_fn(timeout);

		if (EG(exception) || handle == NULL) {
            goto finally;
        }

		async_resume_when(resume, handle, async_resume_when_callback_timeout);
		IF_EXCEPTION_GOTO_ERROR;
		OBJ_RELEASE(&handle->std);
	}

	for (unsigned int i = 0; i < nfds; i++) {
		handle = reactor_socket_new_fn(ufds[i].fd, poll2_events_to_async(ufds[i].events));

		if (EG(exception) || handle == NULL) {
			goto finally;
		}

		async_resume_when(resume, handle, async_resume_when_callback_resolve);

		IF_EXCEPTION_GOTO_ERROR;
		OBJ_RELEASE(&handle->std);
	}

	async_await(resume);

	IF_EXCEPTION_GOTO_ERROR;

	zval *notifier;
	result = 0;

	// calculation how many descriptors are ready
	ZEND_HASH_FOREACH_VAL(resume->triggered_notifiers, notifier) {
		if (Z_TYPE_P(notifier) == IS_OBJECT && instanceof_function(Z_OBJ_P(notifier)->ce, async_ce_ev_handle)) {
			result++;

			const php_socket_t socket = reactor_extract_os_socket_handle_fn((reactor_handle_t *)Z_OBJ_P(notifier));

			// Fine the same socket in the ufds array
			for (unsigned int i = 0; i < nfds; i++) {
				if (ufds[i].fd == socket) {
					ufds[i].revents = async_events_to_poll2(Z_LVAL_P(async_ev_handle_get_triggered_events(Z_OBJ_P(notifier))));
					break;
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

finally:

	if (EXPECTED(resume != NULL)) {
		OBJ_RELEASE(&resume->std);
	}

	if (EXPECTED(handle != NULL)) {
        OBJ_RELEASE(&handle->std);
    }

	return result;

error:
	errno = EINTR;
	result = -1;

	if (EG(exception)) {
		zend_object *error = EG(exception);
		zend_clear_exception();

		if (error->ce == async_ce_cancellation_exception) {
            errno = ECANCELED;
        } else if (error->ce == async_ce_timeout_exception) {
            errno = ETIMEDOUT;
        } else {
        	zend_exception_error(error, E_WARNING);
        }
	}

	goto finally;
}

//===============================================================
#pragma endregion
//===============================================================