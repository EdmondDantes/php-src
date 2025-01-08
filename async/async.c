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
#include <zlib.h>

#include "php_reactor.h"
#include "php_scheduler.h"
#include "php_layer/module_entry.h"
#include "php_layer/notifier.h"
#include "php_layer/reactor_handles.h"
#include "php_layer/exceptions.h"

#ifdef PHP_ASYNC_LIBUV
#include "libuv/libuv_reactor.h"
#endif


#ifdef PHP_SOCKETS
#include "ext/sockets/php_sockets.h"
#endif

/* Async global */
#ifdef ZTS
ZEND_DECLARE_MODULE_GLOBALS(async)
#else
async_globals* async_globals;
#endif

//===============================================================
#pragma region Startup and Shutdown
//===============================================================

/**
 * Async globals destructor.
 */
void async_globals_dtor(zend_async_globals *async_globals)
{
	async_globals->is_async = false;
	async_globals->in_scheduler_context = false;

	circular_buffer_dtor(&async_globals->microtasks);
	circular_buffer_dtor(&async_globals->pending_fibers);
	zend_hash_destroy(&async_globals->fibers_state);
	zend_hash_destroy(&async_globals->defer_callbacks);
}

/**
 * Async globals constructor.
 */
void async_globals_ctor(zend_async_globals *async_globals)
{
	async_globals->is_async = false;
	async_globals->in_scheduler_context = false;

	circular_buffer_ctor(&async_globals->microtasks, 32, sizeof(zval), &zend_std_persistent_allocator);
	circular_buffer_ctor(&async_globals->pending_fibers, 128, sizeof(async_resume_t *), &zend_std_persistent_allocator);
	zend_hash_init(&async_globals->fibers_state, 128, NULL, NULL, 1);
	zend_hash_init(&async_globals->defer_callbacks, 8, NULL, ZVAL_PTR_DTOR, 1);

	async_globals->execute_callbacks_handler = NULL;
	async_globals->exception_handler = NULL;
	async_globals->execute_next_fiber_handler = NULL;
	async_globals->execute_microtasks_handler = NULL;

	if (EG(exception) != NULL) {
		async_globals_dtor(async_globals);
		return;
	}

	if (EG(exception) != NULL) {
		async_globals_dtor(async_globals);
	}
}

/**
 * Async startup function.
 */
void async_module_startup(void)
{
	if (async_register_module() == FAILURE) {
		zend_error(E_CORE_WARNING, "Failed to register the 'True Asynchrony' module.");
	}
}

/**
 * Async shutdown function.
 */
void async_module_shutdown(void)
{
}

void async_fiber_shutdown_callback(zend_fiber *fiber)
{
	async_fiber_state_t *state = async_find_fiber_state(fiber);

	if (state == NULL || state->resume == NULL) {
		return;
	}

	state->resume->status = ASYNC_RESUME_NO_STATUS;
	OBJ_RELEASE(&state->resume->std);
	state->resume = NULL;

	if (zend_hash_index_del(&ASYNC_G(fibers_state), fiber->std.handle) == FAILURE) {
		zend_error(E_WARNING, "Failed to remove the fiber state from the hash table.");
	}
}

//===============================================================
#pragma endregion
//===============================================================

zend_always_inline void async_push_fiber_to_pending(async_resume_t *resume, const bool transfer_resume)
{
	if (UNEXPECTED(circular_buffer_push(&ASYNC_G(pending_fibers), &resume, true) == FAILURE)) {
		async_throw_error("Failed to push the Fiber into the pending queue.");
	} else {
		if (false == transfer_resume) {
			GC_ADDREF(&resume->std);
		}

		GC_ADDREF(&resume->fiber->std);
	}
}

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

void async_resource_cast(const zend_resource *resource, php_socket_t *socket, async_file_descriptor_t *file)
{
	socket = NULL;
	file = NULL;

	php_stream *stream = resource_to_stream(resource);

	if (stream == NULL) {
		return;
	}

	if (php_stream_is(stream, PHP_STREAM_IS_SOCKET)) {

		if (php_stream_cast(stream, PHP_STREAM_AS_SOCKETD, &socket, false) == FAILURE) {
			socket = NULL;
			return;
		}

	} else if (php_stream_is(stream, PHP_STREAM_IS_STDIO)) {
		if (php_stream_cast(stream, PHP_STREAM_AS_STDIO, file, false) == FAILURE) {
			file = NULL;
			return;
		}
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

/**
 * @brief Creates and registers a new fiber state associated with a given fiber and resume object.
 *
 * This function allocates and initializes an async_fiber_state_t structure, linking it to the specified
 * zend_fiber and async_resume_t. The state is then stored in the global fibers state table.
 *
 * @param fiber A pointer to the zend_fiber to associate with the newly created state.
 * @param resume A pointer to the async_resume_t that manages the resumption logic for the fiber.
 *
 * @return A pointer to the newly allocated async_fiber_state_t structure, or NULL if memory allocation fails.
 *
 * @note The Fiber State structure does not increment the reference count of the Fiber or Resume object,
 * as their lifecycle is managed through an internal mechanism. The fiber must notify the Async State
 * upon its destruction to ensure proper cleanup.
 *
 * @warning The function assumes the fiber is valid and has a handle. If the fiber is destroyed without
 * notifying the Async State, it may lead to undefined behavior.
 */
static async_fiber_state_t * async_add_fiber_state(zend_fiber * fiber, async_resume_t *resume)
{
	async_fiber_state_t * state = pecalloc(1, sizeof(async_fiber_state_t), 1);

	if (state == NULL) {
        return NULL;
    }

	state->fiber = fiber;
	state->resume = resume;

	zval zv;
	ZVAL_PTR(&zv, state);

	zend_hash_index_update(&ASYNC_G(fibers_state), fiber->std.handle, &zv);

	return state;
}

void async_start_fiber(zend_fiber * fiber)
{
	async_resume_t * resume = async_resume_new(fiber);

	if (resume == NULL) {
		return;
	}

	async_fiber_state_t *state = async_find_fiber_state(fiber);

	if (state == NULL) {
		async_add_fiber_state(resume->fiber, resume);
	} else {

		if (state->resume != NULL) {
			state->resume->status = ASYNC_RESUME_NO_STATUS;
			OBJ_RELEASE(&state->resume->std);
		}

		state->resume = resume;
	}

	resume->status = ASYNC_RESUME_SUCCESS;
	ZVAL_NULL(&resume->result);

	async_push_fiber_to_pending(resume, true);
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

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

	if (state == NULL) {
		async_add_fiber_state(resume->fiber, resume);
	}

	async_push_fiber_to_pending(resume, false);
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

	if (UNEXPECTED(resume == NULL)) {
		async_throw_error("Failed to create a new Resume object");
		goto finally;
	}

	if (UNEXPECTED(resume->status != ASYNC_RESUME_NO_STATUS)) {
		async_throw_error("Attempt to use a Resume object that is not in the ASYNC_RESUME_NO_STATUS state.");
		goto finally;
    }

	if (UNEXPECTED(resume->fiber != EG(active_fiber))) {
		async_throw_error("Attempt to use a Resume object that is not associated with the current Fiber.");
	    goto finally;
    }

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

	if (state == NULL) {
		state = async_add_fiber_state(resume->fiber, NULL);

		if (UNEXPECTED(state == NULL)) {
			async_throw_error("Failed to create Fiber state");
			goto finally;
        }
	}

	state->resume = resume;

	//
	// Add all notifiers to the event loop.
	// The operation of adding a handle to the event loop is idempotent,
	// so there is no need to worry about the handle being listened to twice.
	// If the Resume object has already been used
	// for waiting on events, but someone added new handles to it or removed old ones,
	// this loop will add new handles to the event loop.
	//

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
	const reactor_handle_t *handle = reactor_handle_from_resource_fn(resource, actions, 0);

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
		async_resume_when(resume, reactor_timer_new_fn(timeout, false), true, async_resume_when_callback_cancel);
	}

	// Add cancellation handle if it is specified.
	if (cancellation != NULL) {
        async_resume_when(resume, cancellation, false, async_resume_when_callback_cancel);
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

	async_resume_when(resume, reactor_signal_new_fn(sig_number), true, async_resume_when_callback_resolve);

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, false, async_resume_when_callback_cancel);
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

	async_resume_when(resume, reactor_timer_new_fn(timeout, false), true, async_resume_when_callback_resolve);

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, false, async_resume_when_callback_cancel);
	}

	async_await(resume);

	// Release the reference to the resume object.
	OBJ_RELEASE(&resume->std);
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

	if (timeout > 0) {
		async_resume_when(
			resume,
			reactor_timer_new_fn(timeout, false),
			true,
			async_resume_when_callback_timeout
		);
		IF_EXCEPTION_GOTO_ERROR;
	}

	for (unsigned int i = 0; i < nfds; i++) {
		async_resume_when(
			resume,
			reactor_socket_new_fn(ufds[i].fd, poll2_events_to_async(ufds[i].events)),
			true,
			async_resume_when_callback_resolve
		);

		IF_EXCEPTION_GOTO_ERROR;
	}

	async_await(resume);

	IF_EXCEPTION_GOTO_ERROR;

	zval *notifier;
	result = 0;

	// calculation how many descriptors are ready
	ZEND_HASH_FOREACH_VAL(resume->triggered_notifiers, notifier) {
		if (Z_TYPE_P(notifier) == IS_OBJECT && instanceof_function(Z_OBJ_P(notifier)->ce, async_ce_poll_handle)) {
			result++;

			const reactor_poll_t *poll = (reactor_poll_t *)Z_OBJ_P(notifier);

			// Fine the same socket in the ufds array
			for (unsigned int i = 0; i < nfds; i++) {
				if (ufds[i].fd == poll->socket) {
					ufds[i].revents = async_events_to_poll2(Z_LVAL(poll->triggered_events));
					break;
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

finally:

	if (EXPECTED(resume != NULL)) {
		OBJ_RELEASE(&resume->std);
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