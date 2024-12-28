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
#include "zend_exceptions.h"
#include "zend_smart_str.h"
#include "zend_interfaces.h"
#include "php_async.h"

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
typedef void (*async_next_fiber_handler_t)(void);

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

/**
 * Async globals Ex-constructor.
 * The method is called in three cases:
 * 1. When it is necessary to return the size of the extended memory.
 * 2. When it is necessary to create and initialize the global Async structure.
 * 3. When the destructor needs to be called.
 */
typedef size_t (* async_ex_globals_fn)(async_globals_t *async_globals, size_t current_size, zend_bool is_destroy);

/**
 * The method activates the Scheduler in the specified thread.
 * The method can only be called in the main Fiber (i.e., when there is no Fiber) of the thread.
 * The method does not return control while the event loop is active.
 */
ZEND_API void async_scheduler_run(void);
ZEND_API async_callbacks_handler_t async_scheduler_set_callbacks_handler(async_callbacks_handler_t handler);
ZEND_API async_next_fiber_handler_t async_scheduler_set_next_fiber_handler(async_next_fiber_handler_t handler);
ZEND_API async_microtasks_handler_t async_scheduler_set_microtasks_handler(async_microtasks_handler_t handler);
ZEND_API async_exception_handler_t async_scheduler_set_exception_handler(async_exception_handler_t handler);

ZEND_API async_ex_globals_fn async_set_ex_globals_handler(async_ex_globals_fn handler);

zend_result async_scheduler_add_handle(const zend_object *handle);


#endif //PHP_SCHEDULER_H
