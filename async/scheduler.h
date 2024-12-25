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
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "php.h"
#include "zend_exceptions.h"
#include "zend_smart_str.h"
#include "zend_interfaces.h"
#include "async.h"

typedef void (*async_callback_handler_t)(void);
typedef void (*async_resume_next_fiber_handler_t)(void);
typedef void (*async_execute_microtasks_handler_t)(void);
typedef void (*async_fiber_exception_handler_t)(void);

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
ZEND_API async_callback_handler_t async_scheduler_set_callback_handler(async_callback_handler_t handler);
ZEND_API async_resume_next_fiber_handler_t async_scheduler_set_next_fiber_handler(async_resume_next_fiber_handler_t handler);
ZEND_API async_execute_microtasks_handler_t async_scheduler_set_microtasks_handler(async_execute_microtasks_handler_t handler);
ZEND_API async_fiber_exception_handler_t async_scheduler_set_exception_handler(async_fiber_exception_handler_t handler);

ZEND_API async_ex_globals_fn async_set_ex_globals_handler(async_ex_globals_fn handler);

zend_result async_scheduler_add_handle(const zend_object *handle);


#endif //SCHEDULER_H
