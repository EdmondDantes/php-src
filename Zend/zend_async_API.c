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
#include "zend_async_API.h"

zend_async_spawn_t zend_async_spawn_fn = NULL;
zend_async_suspend_t zend_async_suspend_fn = NULL;
zend_async_resume_t zend_async_resume_fn = NULL;
zend_async_cancel_t zend_async_cancel_fn = NULL;
zend_async_shutdown_t zend_async_shutdown_fn = NULL;
zend_async_get_coroutines_t zend_async_get_coroutines_fn = NULL;
zend_async_add_event_t zend_async_add_event_fn = NULL;
zend_async_remove_event_t zend_async_remove_event_fn = NULL;
zend_async_new_socket_event_t zend_async_new_socket_event_fn = NULL;
zend_async_new_poll_event_t zend_async_new_poll_event_fn = NULL;
zend_async_new_timer_event_t zend_async_new_timer_event_fn = NULL;
zend_async_new_signal_event_t zend_async_new_signal_event_fn = NULL;
zend_async_new_process_event_t zend_async_new_process_event_fn = NULL;
zend_async_new_thread_event_t zend_async_new_thread_event_fn = NULL;
zend_async_new_filesystem_event_t zend_async_new_filesystem_event_fn = NULL;
zend_async_queue_task_t zend_async_queue_task_fn = NULL;