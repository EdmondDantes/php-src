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
#ifndef PHP_SCHEDULER_H
#define PHP_SCHEDULER_H

#include "php.h"

typedef struct _async_microtask_s async_microtask_t;

typedef void (*async_microtask_handler_t)(async_microtask_t *microtask);

struct _async_microtask_s {
	bool is_fci;
	bool is_cancelled;
	int ref_count;
};

typedef struct _async_internal_microtask_s {
	async_microtask_t task;
	async_microtask_handler_t handler;
} async_internal_microtask_t;

typedef struct _async_function_microtask_s {
	async_microtask_t task;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
} async_function_microtask_t;

/**
 * async_run_callbacks_handler_t - Function pointer type for running pending IO-callbacks/timer-callbacks.
 * This function processes and completes all queued callbacks.
 * The function returns TRUE if event loop has descriptors to process.
 *
 * @param no_wait The parameter indicates whether it is necessary to switch to kernel mode to wait for I/O events.
 */
typedef zend_bool (*async_callbacks_handler_t)(zend_bool no_wait);

/**
 * async_resume_next_fiber_handler_t - Function pointer type for resuming the next fiber in the queue.
 * This function continues execution of the next scheduled fiber.
 */
typedef bool (*async_next_fiber_handler_t)(void);

/**
 * async_execute_microtasks_handler_t - Function pointer type for executing pending microtasks.
 * This function processes and completes all queued microtasks.
 */
typedef void (*async_microtasks_handler_t)(void);

/**
 * async_exception_handler_t - Function pointer type for handling exceptions in fibers/microtasks.
 * This function catches and processes exceptions that occur during fiber execution.
 */
typedef void (*async_exception_handler_t)(void);

BEGIN_EXTERN_C()

typedef struct _zend_async_globals zend_async_globals;

/**
 * The method activates the Scheduler in the specified thread.
 * The method can only be called in the main Fiber (i.e., when there is no Fiber) of the thread.
 * The method does not return control while the event loop is active.
 */
ZEND_API void async_scheduler_launch(void);
ZEND_API async_callbacks_handler_t async_scheduler_set_callbacks_handler(async_callbacks_handler_t handler);
ZEND_API async_next_fiber_handler_t async_scheduler_set_next_fiber_handler(async_next_fiber_handler_t handler);
ZEND_API async_microtasks_handler_t async_scheduler_set_microtasks_handler(async_microtasks_handler_t handler);
ZEND_API async_exception_handler_t async_scheduler_set_exception_handler(async_exception_handler_t handler);
ZEND_API void async_scheduler_add_microtask(zval *microtask);
ZEND_API void async_scheduler_add_microtask_ex(async_microtask_t *microtask);
ZEND_API void async_scheduler_add_microtask_handler(async_microtask_handler_t handler);
ZEND_API async_microtask_t * async_scheduler_create_microtask(zval * microtask);
ZEND_API void async_scheduler_microtask_dtor(async_microtask_t *microtask);

zend_result async_scheduler_add_handle(const zend_object *handle);

END_EXTERN_C()

#endif //PHP_SCHEDULER_H
