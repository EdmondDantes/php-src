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
#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <php.h>
#include "async.h"
#include <Zend/zend_types.h>
#include "php_layer/ev_handles.h"

typedef void (*async_ev_startup_t)(void);
typedef void (*async_ev_shutdown_t)(void);

typedef void (*async_ev_handle_method_t)(async_ev_handle_t *handle);

typedef async_ev_handle_t* (*async_ev_handle_from_resource_t)(zend_resource *resource, zend_ulong actions);
typedef async_ev_handle_t* (*async_ev_file_new_t)(zend_ulong fd, zend_ulong events);
typedef async_ev_handle_t* (*async_ev_socket_new_t)(zend_ulong fd, zend_ulong events);
typedef async_ev_handle_t* (*async_ev_timeout_new_t)(zend_ulong timeout);
typedef async_ev_handle_t* (*async_ev_signal_new_t)(zend_long sig_number);
typedef async_ev_handle_t* (*async_ev_pipe_new_t)(zend_ulong fd, zend_ulong events);
typedef async_ev_handle_t* (*async_ev_tty_new_t)(zend_ulong fd, zend_ulong events);
typedef async_ev_handle_t* (*async_ev_file_system_new_t)(zend_ulong fd, zend_ulong events);
typedef async_ev_handle_t* (*async_ev_process_new_t)(zend_ulong pid, zend_ulong events);
typedef async_ev_handle_t* (*async_ev_thread_new_t)(zend_ulong thread_id, zend_ulong events);
typedef async_ev_handle_t* (*async_ev_idle_new_t)(void);

ZEND_API zend_bool async_ev_is_enabled(void);
void async_ev_add_handle(async_ev_handle_t *handle);

ZEND_API async_ev_startup_t async_ev_global_ctor_fn;
ZEND_API async_ev_startup_t async_ev_startup_fn;
ZEND_API async_ev_shutdown_t async_ev_shutdown_fn;

ZEND_API async_ev_handle_method_t async_ev_add_handle_ex_fn;
ZEND_API async_ev_handle_method_t async_ev_remove_handle_fn;

ZEND_API async_ev_handle_from_resource_t async_ev_handle_from_resource_fn;
ZEND_API async_ev_file_new_t async_ev_file_new_fn;
ZEND_API async_ev_socket_new_t async_ev_socket_new_fn;
ZEND_API async_ev_timeout_new_t async_ev_timeout_new_fn;
ZEND_API async_ev_signal_new_t async_ev_signal_new_fn;
ZEND_API async_ev_pipe_new_t async_ev_pipe_new_fn;
ZEND_API async_ev_tty_new_t async_ev_tty_new_fn;
ZEND_API async_ev_file_system_new_t async_ev_file_system_new_fn;
ZEND_API async_ev_process_new_t async_ev_process_new_fn;
ZEND_API async_ev_thread_new_t async_ev_thread_new_fn;
ZEND_API async_ev_idle_new_t async_ev_idle_new_fn;

#endif //EVENT_LOOP_H
