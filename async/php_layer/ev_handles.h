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

ZEND_API zend_class_entry *async_ce_fiber;

ZEND_API zend_class_entry *async_ce_file;
ZEND_API zend_class_entry *async_ce_socket;
ZEND_API zend_class_entry *async_ce_pipe;
ZEND_API zend_class_entry *async_ce_tty;

ZEND_API zend_class_entry *async_ce_timer;

ZEND_API zend_class_entry *async_ce_signal;
ZEND_API zend_class_entry *async_ce_process;
ZEND_API zend_class_entry *async_ce_thread;

ZEND_API zend_class_entry *async_ce_file_system;

reactor_handle_t* async_resource_to_handle(zend_resource *resource, zend_ulong actions);
reactor_handle_t* async_timeout_new(zend_ulong timeout);
reactor_handle_t* async_signal_new(zend_long sig_number);

#endif //HANDLES_H
