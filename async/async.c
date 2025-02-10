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

ZEND_TLS HashTable * host_name_list = NULL;

//===============================================================
#pragma region Startup and Shutdown
//===============================================================

void async_host_name_list_ctor(void)
{
	host_name_list = NULL;
}

void async_host_name_list_dtor(void)
{
	if (host_name_list != NULL) {
		zend_hash_destroy(host_name_list);
		FREE_HASHTABLE(host_name_list);
		host_name_list = NULL;
	}
}

/**
 * Async globals destructor.
 */
void async_globals_dtor(zend_async_globals *async_globals)
{
	async_globals->is_async = false;
	async_globals->in_scheduler_context = false;

	ZEND_ASSERT(async_globals->exit_exception == NULL && "Exit exception must be NULL.");

	circular_buffer_dtor(&async_globals->microtasks);
	circular_buffer_dtor(&async_globals->deferred_resumes);
	zend_hash_destroy(&async_globals->fibers_state);
	zend_hash_destroy(&async_globals->defer_callbacks);
}

static void async_fiber_state_dtor(zval *zval)
{
	pefree(Z_PTR_P(zval), false);
}

/**
 * Async globals constructor.
 */
void async_globals_ctor(zend_async_globals *async_globals)
{
	async_globals->is_async = false;
	async_globals->in_scheduler_context = false;

	circular_buffer_ctor(&async_globals->microtasks, 32, sizeof(zval), &zend_std_persistent_allocator);
	circular_buffer_ctor(&async_globals->deferred_resumes, 128, sizeof(async_resume_t *), &zend_std_persistent_allocator);
	zend_hash_init(&async_globals->fibers_state, 128, NULL, async_fiber_state_dtor, 1);
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

	if (state == NULL) {
		return;
	}

	ZEND_ASSERT(state->resume == NULL && "Fiber state must not have a resume object.");
	state->resume = NULL;

	if (zend_hash_index_del(&ASYNC_G(fibers_state), fiber->std.handle) == FAILURE) {
		async_warning("Failed to remove the fiber state from the hash table.");
	}
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
 * @param transfer_fiber A flag indicating whether the fiber should be transferred Refcounts.
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
static async_fiber_state_t * async_add_fiber_state(zend_fiber * fiber, async_resume_t *resume, const bool transfer_fiber)
{
	async_fiber_state_t * state = pecalloc(1, sizeof(async_fiber_state_t), false);

	if (state == NULL) {
        return NULL;
    }

	state->fiber = fiber;
	state->resume = resume;

	zval zv;
	ZVAL_PTR(&zv, state);

	if (zend_hash_index_update(&ASYNC_G(fibers_state), fiber->std.handle, &zv) != NULL) {
		//
		// From this point, the Fiber belongs to the Scheduler.
		// The reference count will be decremented in the execute_next_fiber method
		// when the Fiber completes its execution.
		//
		if (false == transfer_fiber) {
			GC_ADDREF(&fiber->std);
		}
    }

	return state;
}

void async_start_fiber(zend_fiber * fiber, zval *params, const uint32_t params_count, HashTable * named_params)
{
	if (UNEXPECTED(ASYNC_G(graceful_shutdown))) {
		zend_error(E_CORE_WARNING, "Cannot start a new fiber during a graceful shutdown");
		return;
	}

	async_resume_t * resume = async_resume_new(fiber);

	if (resume == NULL) {
		return;
	}

	zend_fiber_params_ctor(fiber, params, params_count, named_params);

	async_fiber_state_t *state = async_find_fiber_state(fiber);

	if (state == NULL) {
		state = async_add_fiber_state(resume->fiber, NULL, true);
	}

	resume->status = ASYNC_RESUME_SUCCESS;
	ZVAL_NULL(&resume->result);
	state->resume = resume;

	async_push_fiber_to_deferred_resume(resume, true);
}

void async_resume_fiber(async_resume_t *resume, zval* result, zend_object* error)
{
	if (UNEXPECTED(EG(active_fiber) == resume->fiber)) {
		async_throw_error("Cannot resume the current Fiber.");
		return;
	}

	const bool was_waiting = resume->status == ASYNC_RESUME_WAITING;

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
		resume->error = error;
		GC_ADDREF(error);
	}

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

	if (state == NULL) {
		async_add_fiber_state(resume->fiber, resume, false);
	} else {
		state->resume = resume;
	}

	if (was_waiting) {
		async_push_fiber_to_deferred_resume(resume, false);
	}
}

void async_cancel_fiber(const zend_fiber *fiber, zend_object *error, const bool transfer_error)
{
	const async_fiber_state_t *state = async_find_fiber_state(fiber);

	if (state == NULL || state->resume == NULL) {
		async_throw_error("The fiber is not waiting for asynchronous operations and cannot be terminated.");

		if (transfer_error) {
			OBJ_RELEASE(error);
		}

		return;
	}

	async_resume_fiber(state->resume, NULL, error);

	if (true == transfer_error) {
		// because async_resume_fiber increments the reference count of the error object
		GC_DELREF(error);
	}
}

void async_transfer_throw_to_fiber(zend_fiber *fiber, zend_object *error)
{
	if (fiber == NULL) {
		return;
	}

	// Add fiber state if it does not exist.
	const async_fiber_state_t *state = async_find_fiber_state(fiber);

	if (state == NULL) {
		// TODO: async_resume_new???
		state = async_add_fiber_state(fiber, async_resume_new(fiber), false);
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
void async_wait(async_resume_t *resume)
{
	if (UNEXPECTED(IS_ASYNC_OFF)) {
		return;
	}

	/*
	if (ASYNC_G(in_scheduler_context)) {
		async_throw_error("Cannot await in the scheduler context");
		return;
	}
	*/

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
		state = async_add_fiber_state(resume->fiber, NULL, false);

		if (UNEXPECTED(state == NULL)) {
			async_throw_error("Failed to create Fiber state");
			goto finally;
        }
	}

	// Save the current filename and line number.
	zend_apply_current_filename_and_line(&resume->filename, &resume->lineno);

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

		ZEND_ASSERT(Z_TYPE_P(notifier) == IS_PTR && "Invalid notifier in the resume->notifiers");

		if (EXPECTED(Z_TYPE_P(notifier) == IS_PTR)) {

			reactor_add_handle(((async_resume_notifier_t *) Z_PTR_P(notifier))->notifier);

			if (EG(exception) != NULL) {
				goto finally;
			}
		}
	ZEND_HASH_FOREACH_END();

	async_resume_waiting(resume);
	state->resume = resume;

	if (UNEXPECTED(zend_hash_num_elements(&resume->notifiers) == 0)) {
		// Put resume into the deferred queue if there are no notifiers.
		resume->status = ASYNC_RESUME_SUCCESS;
		ZVAL_NULL(&resume->result);
		state->resume = resume;
		async_push_fiber_to_deferred_resume(resume, false);
    }

	zend_fiber_suspend(EG(active_fiber), NULL, NULL);

finally:

	if (UNEXPECTED(EG(exception) != NULL)) {
		zend_exception_save();
	}

	ZEND_HASH_FOREACH_VAL(&resume->notifiers, notifier)

		ZEND_ASSERT(Z_TYPE_P(notifier) == IS_PTR && "Invalid notifier in the resume->notifiers");

		if (EXPECTED(Z_TYPE_P(notifier) == IS_PTR)) {

			reactor_remove_handle_fn(((async_resume_notifier_t *) Z_PTR_P(notifier))->notifier);

			if (EG(exception) != NULL) {
				zend_exception_save();
			}
		}
	ZEND_HASH_FOREACH_END();

	if (UNEXPECTED(EG(prev_exception) != NULL)) {
		zend_exception_restore();
	}

	if (is_owned_resume) {
		ZEND_ASSERT(GC_REFCOUNT(&resume->std) == 1 && "Resume object has references more than 1");
        OBJ_RELEASE(&resume->std);
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
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_wait.
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

	async_wait(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}

/**
 * The method stops Fiber execution for a specified signal.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_wait.
 */
void async_await_signal(const zend_long sig_number, reactor_notifier_t * cancellation)
{
	async_resume_t *resume = async_resume_new(NULL);

	async_resume_when(resume, reactor_signal_new_fn(sig_number), true, async_resume_when_callback_resolve);

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, false, async_resume_when_callback_cancel);
	}

	async_wait(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}

async_resume_t * async_new_resume_with_timeout(
	zend_fiber * fiber, const zend_ulong timeout, reactor_notifier_t *cancellation
)
{
	async_resume_t *resume = async_resume_new(fiber);

	if (timeout > 0) {
		async_resume_when(
			resume, reactor_timer_new_fn(timeout, false), true, async_resume_when_callback_resolve
		);
	}

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, false, async_resume_when_callback_cancel);
	}

	return resume;
}

/**
 * The method stops Fiber execution for a specified time.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_wait.
 * It's like a sleep()/usleep() function.
 */
void async_wait_timeout(const zend_ulong timeout, reactor_notifier_t * cancellation)
{
	if (UNEXPECTED(timeout == 0 && cancellation == NULL)) {
		async_wait(NULL);
		return;
	}

	async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, cancellation);

	async_wait(resume);

	ZEND_ASSERT(GC_REFCOUNT(&resume->std) == 1 && "Resume object has references more than 1");

	// Release the reference to the resume object.
	OBJ_RELEASE(&resume->std);
}

void async_wait_socket(php_socket_t socket, const zend_ulong events, const zend_ulong timeout, reactor_notifier_t * cancellation)
{
	reactor_handle_t * handle = reactor_socket_new_fn(socket, events);

	if (UNEXPECTED(EG(exception) != NULL)) {
        return;
    }

	async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, cancellation);

	if (UNEXPECTED(EG(exception) != NULL)) {
		return;
	}

	async_resume_when(resume, handle, true, async_resume_when_callback_resolve);

	async_wait(resume);

	ZEND_ASSERT(GC_REFCOUNT(&resume->std) == 1 && "Resume object has references more than 1");
	OBJ_RELEASE(&resume->std);
}

void async_wait_fd(async_file_descriptor_t fd, const zend_ulong events, const zend_ulong timeout, reactor_notifier_t * cancellation)
{
	reactor_handle_t * handle = reactor_file_new_fn(fd, events);

	if (UNEXPECTED(EG(exception) != NULL)) {
		return;
	}

	async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, cancellation);

	if (UNEXPECTED(EG(exception) != NULL)) {
		return;
	}

	async_resume_when(resume, handle, true, async_resume_when_callback_resolve);

	async_wait(resume);

	ZEND_ASSERT(GC_REFCOUNT(&resume->std) == 1 && "Resume object has references more than 1");
	OBJ_RELEASE(&resume->std);
}

//===============================================================
#pragma region POLL2 EMULATION
//===============================================================

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

	async_wait(resume);

	IF_EXCEPTION_GOTO_ERROR;

	if (resume->triggered_notifiers == NULL) {
		result = 0;
		goto finally;
	}

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

//===============================================================
#pragma region SELECT EMULATION
//===============================================================
PHPAPI int async_select(php_socket_t max_fd, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *tv)
{
	int result = 0;
	fd_set aread, awrite, aexcept;

	/* As max_fd is unsigned, non socket might overflow. */
	if (max_fd > (php_socket_t)FD_SETSIZE) {
		return -1;
	}

	async_resume_t *resume = async_resume_new(NULL);

	if(resume == NULL) {
		errno = ENOMEM;
		return -1;
	}

	ULONGLONG ms_total;

	/* calculate how long we need to wait in milliseconds */
	if (tv == NULL) {
		ms_total = INFINITE;
	} else {
		ms_total = tv->tv_sec * 1000;
		ms_total += tv->tv_usec / 1000;
	}

	if (ms_total != INFINITE) {
		async_resume_when(
			resume,
			reactor_timer_new_fn(ms_total, false),
			true,
			async_resume_when_callback_timeout
		);
		IF_EXCEPTION_GOTO_ERROR;
	}

#define SAFE_FD_ISSET(fd, set)	(set != NULL && FD_ISSET(fd, set))

	/* build an array of handles for non-sockets */
	for (int i = 0; (uint32_t)i < max_fd; i++) {

		bool is_socket = false;
		async_file_descriptor_t fh = 0;

#ifdef PHP_WIN32
		// For windows, we work with sockets only
		is_socket = true;
#else
		fh = (async_file_descriptor_t) i;
#endif

		int events = 0;
		reactor_handle_t *handle = NULL;

		if (SAFE_FD_ISSET(i, rfds)) {
			events |= ASYNC_READABLE;
		}

		if (SAFE_FD_ISSET(i, wfds)) {
			events |= ASYNC_WRITABLE;
		}

		if (SAFE_FD_ISSET(i, efds)) {
			events |= ASYNC_PRIORITIZED;
		}

		if (is_socket) {
			handle = reactor_socket_new_fn(i, events);
		} else {
			handle = reactor_file_new_fn(fh, events);
		}

		async_resume_when(
			resume,
			handle,
			true,
			async_resume_when_callback_resolve
		);

		IF_EXCEPTION_GOTO_ERROR;
	}

	FD_ZERO(&aread);
	FD_ZERO(&awrite);
	FD_ZERO(&aexcept);

	async_wait(resume);

	IF_EXCEPTION_GOTO_ERROR;

	if (resume->triggered_notifiers == NULL) {
		result = 0;
		goto finally;
	}

	zval *notifier;
	result = 0;

	// Calculation state of aread, awrite, aexcept:
	ZEND_HASH_FOREACH_VAL(resume->triggered_notifiers, notifier) {
		if (Z_TYPE_P(notifier) == IS_OBJECT && instanceof_function(Z_OBJ_P(notifier)->ce, async_ce_poll_handle)) {
			result++;

			const reactor_poll_t *poll = (reactor_poll_t *)Z_OBJ_P(notifier);

			const zend_long events = Z_LVAL(poll->triggered_events);

			if (events & ASYNC_READABLE) {
				FD_SET(poll->socket, &aread);
			}

			if (events & ASYNC_WRITABLE) {
				FD_SET(poll->socket, &awrite);
			}

			if (events & ASYNC_DISCONNECT || events & ASYNC_PRIORITIZED) {
				FD_SET(poll->socket, &aexcept);
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

//===============================================================
#pragma region DNS
//===============================================================

int async_network_get_addresses(const char *host, int socktype, struct sockaddr ***sal, zend_string **error_string)
{
	if (host == NULL) {
		return 0;
	}

	struct addrinfo hints = {0};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = socktype;

	async_resume_t *resume = async_resume_new(NULL);

	if(resume == NULL) {
		errno = ENOMEM;
		return -1;
	}

	zend_string * z_host = zend_string_init(host, strlen(host), 0);

	reactor_handle_t * dns_info = reactor_dns_info_new_fn(z_host, NULL, NULL, &hints);

	zend_string_release(z_host);

	if (UNEXPECTED(EG(exception) != NULL || dns_info == NULL)) {
		OBJ_RELEASE(&dns_info->std);
		return -1;
	}

	async_resume_when(resume, dns_info, false, async_resume_when_callback_resolve);
	async_wait(resume);
	OBJ_RELEASE(&resume->std);

	if (UNEXPECTED(EG(exception) != NULL)) {

		reactor_dns_info_cancel_fn(dns_info);
		OBJ_RELEASE(&dns_info->std);

		zend_string * message = zend_current_exception_get_message(true);
		bool is_owned_message = false;

		if (message == NULL) {
			is_owned_message = true;
			message = zend_string_init("Unknown error", sizeof("Unknown error") - 1, 0);
		}

		if (error_string) {

			/* free error string received during previous iteration (if any) */
			if (*error_string) {
				zend_string_release_ex(*error_string, 0);
			}

			*error_string = strpprintf(
				0, "async getaddrinfo for %s failed: %s", host, ZSTR_VAL(message)
			);
		} else {
			php_error_docref(NULL, E_WARNING, "async getaddrinfo for %s failed: %s", host, ZSTR_VAL(message));
		}

		if (is_owned_message) {
			zend_string_release(message);
		}

		return -1;
	}

	struct sockaddr **sap;
	struct addrinfo *sai;
	int n;

	sai = ((reactor_dns_info_t *) dns_info)->addr_info;

	for (n = 1; (sai = sai->ai_next) != NULL; n++)
		;

	*sal = safe_emalloc((n + 1), sizeof(*sal), 0);
	sai = ((reactor_dns_info_t *) dns_info)->addr_info;
	sap = *sal;

	do {
		*sap = emalloc(sai->ai_addrlen);
		memcpy(*sap, sai->ai_addr, sai->ai_addrlen);
		sap++;
	} while ((sai = sai->ai_next) != NULL);

	*sap = NULL;

	OBJ_RELEASE(&dns_info->std);

	return n;
}

static struct hostent *addr_info_to_hostent(const struct addrinfo *addr_info)
{
	if (addr_info == NULL || addr_info->ai_family != AF_INET) {
		return NULL;
	}

	struct hostent *result = (struct hostent *)emalloc(sizeof(struct hostent));

	if (result == NULL) {
		return NULL;
	}

	memset(result, 0, sizeof(struct hostent));

	char **addr_list = (char **)emalloc(2 * sizeof(char *));

	if (addr_list == NULL) {
		efree(result);
		return NULL;
	}

	memset(addr_list, 0, 2 * sizeof(char *));

	struct sockaddr_in *addr_in = (struct sockaddr_in *)addr_info->ai_addr;
	addr_list[0] = (char *)emalloc(sizeof(struct in_addr));

	if (addr_list[0] == NULL) {
		efree(addr_list);
		efree(result);
		return NULL;
	}

	memcpy(addr_list[0], &addr_in->sin_addr, sizeof(struct in_addr));

	result->h_name = addr_info->ai_canonname ? estrdup(addr_info->ai_canonname) : NULL;
	result->h_aliases = NULL;
	result->h_addrtype = AF_INET;
	result->h_length = sizeof(struct in_addr);
	result->h_addr_list = addr_list;

	return result;
}

static void hostent_dtor(zval *pDest)
{
	struct hostent *host = Z_PTR_P(pDest);

	if (host == NULL) {
		return;
	}

	// Free the canonical name if it was allocated
	if (host->h_name) {
		efree(host->h_name);
	}

	// Free the address list
	if (host->h_addr_list) {
		if (host->h_addr_list[0]) {
			efree(host->h_addr_list[0]);
		}

		efree(host->h_addr_list);
	}

	// Free the hostent structure itself
	efree(host);
}

zend_always_inline struct hostent* find_host_by_name(const char *name)
{
	if (host_name_list == NULL) {
        return NULL;
    }

	zval *entry = zend_hash_str_find(host_name_list, name, strlen(name));

	if (entry == NULL) {
        return NULL;
    }

	return (struct hostent *)Z_PTR_P(entry);
}

zend_always_inline void store_host_by_name(const char *name, struct hostent *host)
{
    if (host_name_list == NULL) {
        ALLOC_HASHTABLE(host_name_list);
        zend_hash_init(host_name_list, 8, NULL, hostent_dtor, 0);
    }

    zval zv;
    ZVAL_PTR(&zv, host);

    zend_hash_str_update(host_name_list, name, strlen(name), &zv);
}

PHPAPI struct hostent* async_network_get_host_by_name(const char *name)
{
	if (name == NULL) {
        return NULL;
    }

	struct hostent* result = find_host_by_name(name);

	if (result != NULL) {
        return result;
    }

	struct addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;

	async_resume_t *resume = async_resume_new(NULL);

	if(resume == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	zend_string * z_host = zend_string_init(name, strlen(name), 0);
	reactor_handle_t * dns_info = reactor_dns_info_new_fn(z_host, NULL, NULL, &hints);
	zend_string_release(z_host);

	if (UNEXPECTED(EG(exception) != NULL || dns_info == NULL)) {
		OBJ_RELEASE(&dns_info->std);
		errno = ENOMEM;
		return NULL;
	}

	async_resume_when(resume, dns_info, false, async_resume_when_callback_resolve);
	async_wait(resume);
	OBJ_RELEASE(&resume->std);

	if (UNEXPECTED(EG(exception) != NULL)) {
		zend_exception_to_warning("async_network_get_host_by_name error: %s", true);
		reactor_dns_info_cancel_fn(dns_info);
		OBJ_RELEASE(&dns_info->std);
		return NULL;
	}

	result = addr_info_to_hostent(((reactor_dns_info_t *) dns_info)->addr_info);

	store_host_by_name(name, result);
	
	OBJ_RELEASE(&dns_info->std);

	return result;
}

zend_string* async_get_host_by_addr(const char * ip)
{
	if (ip == NULL) {
        return NULL;
    }

	async_resume_t *resume = async_resume_new(NULL);

	if(resume == NULL) {
		zend_error(E_CORE_WARNING, "Failed to create a new Resume object");
		return NULL;
	}

	zend_string * z_address = zend_string_init(ip, strlen(ip), 0);

	reactor_handle_t * dns_info = reactor_dns_info_new_fn(NULL, NULL, z_address, NULL);

	if (UNEXPECTED(EG(exception) != NULL || dns_info == NULL)) {
		OBJ_RELEASE(&dns_info->std);
		errno = ENOMEM;
		return NULL;
	}

	async_resume_when(resume, dns_info, true, async_resume_when_callback_resolve);
	async_wait(resume);
	OBJ_RELEASE(&resume->std);

	if (UNEXPECTED(EG(exception) != NULL)) {
		zend_exception_to_warning("async_get_host_by_addr error: %s", true);
		reactor_dns_info_cancel_fn(dns_info);
		OBJ_RELEASE(&dns_info->std);
	}

	zend_string * result = zend_string_copy(Z_STR(((reactor_dns_info_t *) dns_info)->host));
	OBJ_RELEASE(&dns_info->std);

	return result;
}

void async_get_addr_info(zend_string *host, zend_string *service, struct addrinfo *hints, struct addrinfo **res)
{
	reactor_handle_t * dns_info = reactor_dns_info_new_fn(host, service, NULL, hints);

	if (UNEXPECTED(EG(exception) != NULL || dns_info == NULL)) {
		OBJ_RELEASE(&dns_info->std);
		return;
	}

	async_resume_t *resume = async_resume_new(NULL);

	async_resume_when(resume, dns_info, false, async_resume_when_callback_resolve);
	async_wait(resume);
	OBJ_RELEASE(&resume->std);

	if (UNEXPECTED(EG(exception) != NULL)) {
		reactor_dns_info_cancel_fn(dns_info);
		OBJ_RELEASE(&dns_info->std);
		return;
	}

	res = &((reactor_dns_info_t *) dns_info)->addr_info;
}

void async_free_addr_info(struct addrinfo *addr_info)
{
	// Calc from addinfo reactor_dns_info_t
	reactor_dns_info_t * dns_info = (reactor_dns_info_t *)((char *)addr_info - XtOffsetOf(reactor_dns_info_t, addr_info));
	OBJ_RELEASE(&dns_info->handle.std);
}

//===============================================================
#pragma endregion
//===============================================================

bool async_ensure_socket_nonblocking(php_socket_t socket)
{
#ifdef PHP_WIN32
	/* Set the socket to nonblocking mode */
	DWORD yes = 1;
	if (ioctlsocket(socket, FIONBIO, &yes) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		char *buffer = php_win32_error_to_msg(error);

		if (!buffer[0]) {
			async_warning("Unable to set socket to non-blocking mode [0x%08lx]", error);
		} else {
			async_warning("Unable to set socket to non-blocking mode [0x%08lx]: %s", error, buffer);
		}

		php_win32_error_msg_free(buffer);

		return false;
	}
#else
	int flags = fcntl(socket, F_GETFL);

	if (flags == -1) {
		async_warning("Unable to obtain blocking state");
		return false;
	}

	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
		async_warning("Unable to set socket to non-blocking mode");
		return false;
	}
#endif

	return true;
}

ZEND_API zend_long async_wait_process(async_process_t process_h, const zend_ulong timeout)
{
	reactor_handle_t * handle = reactor_process_new_fn(process_h, 0);

	if (UNEXPECTED(EG(exception) != NULL)) {
		return -1;
	}

	reactor_process_t *process = (reactor_process_t *) handle;

	async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, NULL);

	if (UNEXPECTED(EG(exception) != NULL)) {
		return -1;
	}

	async_resume_when(resume, handle, true, async_resume_when_callback_resolve);

	async_wait(resume);

	ZEND_ASSERT(GC_REFCOUNT(&resume->std) == 1 && "Resume object has references more than 1");
	OBJ_RELEASE(&resume->std);

	return Z_TYPE(process->exit_code) == IS_LONG ? Z_LVAL(process->exit_code) : -1;
}

ZEND_API pid_t async_waitpid(pid_t pid, int *status, int options)
{
	zend_long process_status = async_wait_process((async_process_t) pid, 0);

	*status = (int) process_status;

	if (EG(exception) != NULL) {
		return -1;
	}

	return pid;
}