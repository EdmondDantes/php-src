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
#include <php_network.h>
#include <ext/standard/proc_open.h>
#include "scheduler.h"
#include "internal/circular_buffer.h"
#include "php_layer/resume.h"

#define ASYNC_READABLE 1
#define ASYNC_WRITABLE 2
#define ASYNC_DISCONNECT 4
#define ASYNC_PRIORITIZED 8

/**
 * Global asynchronous context.
 */
typedef struct _async_globals_s async_globals_t;
/**
 * Fiber state structure.
 * The structure describes the relationship between Fiber and the resume state.
 * The resume state can be NULL, in which case the Fiber is considered active.
*/
typedef struct _async_fiber_state_s async_fiber_state_t;

struct _async_globals_s {
	/* Equal TRUE if the asynchronous context is enabled */
	bool is_async;
	/* Equal TRUE if the scheduler is running */
	bool in_scheduler_context;
	// Microtask and fiber queues
	circular_buffer_t microtasks;
	/* Queue of resume objects: async_resume_t */
	circular_buffer_t pending_fibers;
	/* List of async_fiber_state_t  */
	HashTable fibers_state;
	/* Handlers for the scheduler */
	async_callbacks_handler_t execute_callbacks_handler;
	async_next_fiber_handler_t execute_next_fiber_handler;
	async_microtasks_handler_t execute_microtasks_handler;
	async_exception_handler_t exception_handler;
#ifdef PHP_ASYNC_TRACK_HANDLES
	/* List of linked handles to fibers */
	HashTable linked_handles;
#endif
	/* Extension of async_fiber_state_t */
	char extend[];
};

struct _async_fiber_state_s {
	zend_fiber *fiber;
	/* Fiber resume object. Can be NULL */
	async_resume_t *resume;
};

/* Async global */
#ifdef ZTS
ZEND_API int async_globals_id = 0;
ZEND_API size_t async_globals_offset;
# define ASYNC_G(v) ZEND_TSRMG_FAST(async_globals_id, async_globals_t *, v)
#else
# define ASYNC_G(v) (async_globals->v)
ZEND_API async_globals_t* async_globals;
#endif

#define IS_ASYNC_ON (ASYNC_G(is_async) == true)
#define IS_ASYNC_OFF (ASYNC_G(is_async) == false)

void async_module_startup(void);
void async_module_shutdown(void);
ZEND_API void async_resource_to_fd(const zend_resource *resource, php_socket_t *socket, php_file_descriptor_t *file);

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
ZEND_API void async_resume_fiber(async_resume_t *resume, const zval* result, zend_object* error);

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
 *
 * @note
 * - If the fiber is not in a cancelable state, no action is taken other than throwing an error.
 * - This function relies on the internal state of the fiber to determine if cancellation is possible.
 * - The error object passed will have its reference count managed by the resumption process.
 */
ZEND_API void async_cancel_fiber(const zend_fiber *fiber, zend_object *error);

#endif //ASYNC_H
