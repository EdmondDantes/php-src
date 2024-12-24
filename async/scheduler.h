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

typedef void (*async_callback_handler_t)(void);
typedef void (*async_resume_next_fiber_handler_t)(void);
typedef void (*async_execute_microtasks_handler_t)(void);

ZEND_API async_callback_handler_t async_scheduler_set_callback_handler(async_callback_handler_t handler);
ZEND_API async_resume_next_fiber_handler_t async_scheduler_set_next_fiber_handler(async_resume_next_fiber_handler_t handler);
ZEND_API async_execute_microtasks_handler_t async_scheduler_set_microtasks_handler(async_execute_microtasks_handler_t handler);

zend_result async_scheduler_add_handle(const zend_object *handle);

#endif //SCHEDULER_H
