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
#ifndef PHP_ASYNC_H
#define PHP_ASYNC_H

#include "php.h"
#include "php_network.h"
#include "async/php_scheduler.h"
#include "async/internal/circular_buffer.h"
#include "async/php_layer/resume.h"
#include "php_layer/exceptions.h"
#include "zend_fibers.h"

#define ASYNC_READABLE 1
#define ASYNC_WRITABLE 2
#define ASYNC_DISCONNECT 4
#define ASYNC_PRIORITIZED 8

/**
 * Global asynchronous context.
 */
ZEND_BEGIN_MODULE_GLOBALS(async)
	/* Equal TRUE if the asynchronous context is enabled */
	bool is_async;
	/* Equal TRUE if the scheduler is running */
	bool in_scheduler_context;
	/* Equal TRUE if the reactor is in the process of shutting down */
	bool graceful_shutdown;
	/* Number of active event handles */
	unsigned int event_handle_count;
	/* Exit exception object */
	zend_object * exit_exception;
	// Microtask and fiber queues
	circular_buffer_t microtasks;
	/* Queue of resume objects: async_resume_t */
	circular_buffer_t deferred_resumes;
	/* List of async_fiber_state_t  */
	HashTable fibers_state;
	/* List of deferred callbacks */
	HashTable defer_callbacks;
	/* Special fiber for the microtasks */
	zend_fiber * microtask_fiber;
	/* Special fiber for the callbacks */
	zend_fiber * callbacks_fiber;
	/* Link to the reactor structure */
	void * reactor;
	/* Handlers for the scheduler */
	async_callbacks_handler_t execute_callbacks_handler;
	async_next_fiber_handler_t execute_next_fiber_handler;
	async_microtasks_handler_t execute_microtasks_handler;
	async_exception_handler_t exception_handler;
#ifdef PHP_ASYNC_TRACK_HANDLES
	/* List of linked handles to fibers */
	HashTable linked_handles;
#endif
ZEND_END_MODULE_GLOBALS(async)

/**
 * Fiber state structure.
 * The structure describes the relationship between Fiber and the resume state.
 * The resume state can be NULL, in which case the Fiber is considered active.
*/
typedef struct _async_fiber_state_s async_fiber_state_t;

struct _async_fiber_state_s {
	zend_fiber *fiber;
	/* Fiber resume object. Can be NULL */
	async_resume_t *resume;
};

/* Async global */
ZEND_EXTERN_MODULE_GLOBALS(async)
#define ASYNC_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(async, v)
#define ASYNC_GLOBAL ZEND_MODULE_GLOBALS_BULK(async)

#define IS_ASYNC_ON (ASYNC_G(is_async) == true)
#define IS_ASYNC_OFF (ASYNC_G(is_async) == false)
#define IS_ASYNC_HAS_DEFER_FIBER circular_buffer_is_not_empty(&ASYNC_G(deferred_resumes))

#define IN_ASYNC_CONTEXT EG(active_fiber) != NULL && IS_ASYNC_ON

//
// Definitions compatibles with proc_open()
//
#ifdef PHP_WIN32
typedef HANDLE async_file_descriptor_t;
typedef DWORD async_process_id_t;
typedef HANDLE async_process_t;
#else
typedef int async_file_descriptor_t;
typedef pid_t async_process_id_t;
typedef pid_t async_process_t;
#endif

zend_always_inline void async_push_fiber_to_deferred_resume(async_resume_t *resume, const bool transfer_resume)
{
	if (UNEXPECTED(circular_buffer_push(&ASYNC_G(deferred_resumes), &resume, true) == FAILURE)) {
		async_throw_error("Failed to push the Fiber into the pending queue.");
	} else {

		//
		// When the resume object enters the deferred queue,
		// its reference count is incremented, and after extraction, it is decremented.
		// However, the Fiber does not modify its reference count in any way.
		//

		if (false == transfer_resume) {
			GC_ADDREF(&resume->std);
		}
	}
}

zend_always_inline async_resume_t * async_next_deferred_resume(void)
{
	if (circular_buffer_is_empty(&ASYNC_G(deferred_resumes))) {
		return NULL;
	}

	async_resume_t *resume;

	if (UNEXPECTED(circular_buffer_pop(&ASYNC_G(deferred_resumes), &resume) == FAILURE)) {
		ZEND_ASSERT("Failed to pop the Fiber from the pending queue.");
		return NULL;
	}

	return resume;
}


BEGIN_EXTERN_C()

void async_module_startup(void);
void async_module_shutdown(void);
void async_host_name_list_ctor(void);
void async_host_name_list_dtor(void);
void async_globals_ctor(zend_async_globals *async_globals);
void async_globals_dtor(zend_async_globals *async_globals);
void async_fiber_shutdown_callback(zend_fiber *fiber);
ZEND_API void async_resource_cast(const zend_resource *resource, php_socket_t *socket, async_file_descriptor_t *file);
ZEND_API php_socket_t async_try_extract_socket_object(zend_object * object);

/**
 * @brief Suspends the current fiber and waits for the specified events.
 *
 * @param resume  Pointer to an async_resume_t object describing the events to wait for.
 *                If NULL, a new resume object will be created internally.
 *
 * @details This function suspends the execution of the current fiber, placing it into a
 *          waiting state until the specified descriptors or events trigger a resumption.
 *          Control is returned to the scheduler until the awaited events complete.
 *
 *          If the fiber is not in a valid context (such as outside of a fiber or within the
 *          scheduler), the function raises an error and returns immediately.
 *
 *          If the resume object is not in the ASYNC_RESUME_PENDING state, or is not associated
 *          with the current fiber, an error is thrown. A fiber state is created if one does not exist.
 *
 * @note This function cannot be called from within the scheduler context or from outside of a fiber.
 *       Attempting to await on an invalid resume object or a mismatched fiber will raise errors.
 *
 * @return void   No return value. Errors are signaled through thrown exceptions or error messages.
 *
 * @throws Throws errors in the following cases:
 *         - Waiting in a non-fiber context.
 *         - Waiting in the scheduler context.
 *         - Resume object is in an invalid state.
 *         - Resume object is not linked to the active fiber.
 *         - Failed to create fiber state or resume object.
 */
ZEND_API void async_wait(async_resume_t *resume);

/**
 * Finds the state of a given fiber.
 *
 * This function retrieves the state associated with the specified fiber by looking it up
 * in the global fibers state hash table. If asynchronous operations are disabled or the
 * fiber state cannot be found, the function returns NULL.
 *
 * @param fiber  A constant pointer to the zend_fiber structure representing the fiber whose state is being queried.
 *
 * @return A pointer to the async_fiber_state_t structure representing the fiber's state,
 *         or NULL if the fiber state is not found or if asynchronous operations are disabled.
 *
 * @note
 * - If the ASYNC subsystem is turned off (IS_ASYNC_OFF), the function immediately returns NULL.
 * - The function performs a lookup by the fiber's handle in the global fibers state table.
 * - The returned fiber state is cast from the zval pointer retrieved from the hash table.
 * - The zval retrieved from the hash table is decremented with `zval_ptr_dtor()` to reduce its reference count.
 * - This function does not allocate new memory but returns a pointer to the existing fiber state.
 */
ZEND_API async_fiber_state_t * async_find_fiber_state(const zend_fiber *fiber);

/**
 * @brief Starts an asynchronous fiber and schedules it for execution.
 *
 * This function initializes and starts the provided zend_fiber, ensuring that it
 * is properly managed within the fiber system. If the fiber state is already present,
 * the existing resume handle is released and replaced by a new one. The fiber is
 * then added to the pending queue for execution.
 *
 * @param fiber A pointer to the zend_fiber that needs to be started asynchronously.
 *
 * @note If memory allocation for the resume handle fails, the function returns early without starting the fiber.
 * @note If pushing the fiber to the pending queue fails, an error is thrown, and the fiber is not executed.
 *
 * @throws Throws an error if the fiber cannot be added to the pending queue.
 */
ZEND_API void async_start_fiber(zend_fiber * fiber, zval *params, const uint32_t params_count, HashTable * named_params);

/**
 * Resumes an asynchronous fiber by enqueuing it into the pending queue.
 *
 * This function manages the result or error associated with the fiber resume operation.
 * It ensures that any previously set result or error is properly released and deallocated.
 * If a new result is provided, it is copied into the resume structure. If an error is provided,
 * its reference count is incremented to prevent premature deallocation.
 *
 * If the fiber is not already in a pending state, it is pushed into the global pending fibers queue.
 * If the queue push fails, an error is thrown, and the fiber is not resumed.
 *
 * @param resume  A pointer to the async_resume_t structure representing the fiber to be resumed.
 * @param result  A constant zval pointer representing the result to be set for the fiber, or NULL if no result.
 * @param error   A zend_object pointer representing an error, or NULL if no error occurred.
 *
 * @note This function safely handles memory management for both zval and zend_object.
 *       - The zval* result is copied and managed internally.
 *       - The zend_object* error is reference-counted to ensure it is not prematurely destroyed.
 *       - If the fiber is already in a pending state, no additional action is taken.
 *       - If the circular buffer operation fails, an exception is thrown.
 */
ZEND_API void async_resume_fiber(async_resume_t *resume, zval* result, zend_object* error);

/**
 * Cancels an asynchronous fiber by resuming it with an error.
 *
 * This function attempts to cancel a running or suspended fiber by finding its state
 * and resuming it with the provided error. If the fiber is not in a state where it
 * can be canceled (e.g., not waiting for asynchronous operations), an error is thrown.
 *
 * If the fiber can be resumed, it is resumed with no result (NULL) and the specified error.
 *
 * @param fiber  A constant pointer to the zend_fiber structure representing the fiber to cancel.
 * @param error  A zend_object pointer representing the error to pass to the fiber upon cancellation.
 * @param transfer_error Indicates that the error object is transferred to the ownership of the function.
 *
 * @note
 * - If the fiber is not in a cancelable state, no action is taken other than throwing an error.
 * - This function relies on the internal state of the fiber to determine if cancellation is possible.
 * - The error object passed will have its reference count managed by the resumption process.
 */
ZEND_API void async_cancel_fiber(const zend_fiber *fiber, zend_object *error, const bool transfer_error);

ZEND_API async_resume_t * async_new_resume_with_timeout(
	zend_fiber * fiber, const zend_ulong timeout, reactor_notifier_t *cancellation
);

ZEND_API void async_wait_timeout(const zend_ulong timeout, reactor_notifier_t * cancellation);

ZEND_API void async_wait_socket(php_socket_t socket, const zend_ulong events, const zend_ulong timeout, reactor_notifier_t * cancellation);

ZEND_API void async_wait_fd(async_file_descriptor_t fd, const zend_ulong events, const zend_ulong timeout, reactor_notifier_t * cancellation);

/**
 * Transfers an exception to the specified fiber and schedules it for resumption.
 *
 * This function ensures that the given fiber has a valid state. If the state does not exist,
 * it creates a new one and associates it with the fiber. If the fiber's state already has
 * an existing exception, it is linked as the previous exception to maintain the chain.
 * Finally, the fiber is moved to the pending queue with the new exception, triggering
 * its resumption with the error.
 *
 * @param fiber  The target fiber to which the exception is transferred.
 * @param error The exception object to be thrown in the fiber context.
 */
ZEND_API void async_transfer_throw_to_fiber(zend_fiber *fiber, zend_object *error);

/**
 * @brief Asynchronously polls multiple file descriptors.
 *
 * @param ufds     Pointer to an array of php_pollfd structures representing the file descriptors to poll.
 * @param nfds     Number of file descriptors in the ufds array.
 * @param timeout  Timeout value in milliseconds. A value of 0 returns immediately.
 *                 A negative value waits indefinitely.
 *
 * @return int     On success, returns the number of file descriptors ready for the requested events.
 *                 On failure, returns -1 and sets errno to one of the following:
 *                 - ENOMEM: Memory allocation failed.
 *                 - EINTR: Operation was interrupted by an exception.
 *                 - ECANCELED: Operation was cancelled.
 *                 - ETIMEDOUT: Operation timed out.
 *
 * @details This function performs asynchronous polling by registering file descriptors and waiting
 *          for specified events within a timeout period. If a timeout is specified, a timer is
 *          created and linked to the polling process. For each file descriptor, an event handle
 *          is generated, and callbacks are attached to resume execution upon completion.
 *
 * @note This function relies on the PHP asynchronous reactor system and exception handling
 *       mechanisms. It assumes the existence of relevant exception classes and internal
 *       reactor structures.
 */
ZEND_API int async_poll2(php_pollfd *ufds, unsigned int nfds, const int timeout);

ZEND_API int async_select(php_socket_t max_fd, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *tv);

ZEND_API int async_network_get_addresses(const char *host, int socktype, struct sockaddr ***sal, zend_string **error_string);

ZEND_API struct hostent* async_network_get_host_by_name(const char *name);

ZEND_API zend_string* async_get_host_by_addr(const char* ip);

ZEND_API void async_get_addr_info(zend_string *host, zend_string *service, struct addrinfo *hints, struct addrinfo **res);

ZEND_API void async_free_addr_info(struct addrinfo *addr_info);

ZEND_API bool async_ensure_socket_nonblocking(php_socket_t socket);

ZEND_API zend_long async_wait_process(async_process_t pid, const zend_ulong timeout);

ZEND_API pid_t async_waitpid(pid_t pid, int *status, int options);


END_EXTERN_C()

#endif //ASYNC_H
