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
#ifndef HANDLES_H
#define HANDLES_H

#include "php.h"
#include "zend_types.h"
#include "notifier.h"

typedef struct _async_fiber_handle_s async_fiber_handle_t;

struct _async_fiber_handle_s {
	reactor_handle_t handle;
	zend_fiber *fiber;
};

ZEND_API zend_class_entry *async_ce_ev_handle;

ZEND_API zend_class_entry *async_ce_fiber_handle;

ZEND_API zend_class_entry *async_ce_file_handle;
ZEND_API zend_class_entry *async_ce_socket_handle;
ZEND_API zend_class_entry *async_ce_pipe_handle;
ZEND_API zend_class_entry *async_ce_tty_handle;

ZEND_API zend_class_entry *async_ce_timer_handle;

ZEND_API zend_class_entry *async_ce_signal_handle;
ZEND_API zend_class_entry *async_ce_process_handle;
ZEND_API zend_class_entry *async_ce_thread_handle;

ZEND_API zend_class_entry *async_ce_file_system_handle;

void async_register_handlers_ce(void);

#endif //HANDLES_H
