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
#include "event_loop.h"

void async_ev_startup(void)
{
}

void async_ev_shutdown(void)
{
}

async_handle_t* async_ev_handle_from_resource(zend_resource *resource, zend_ulong actions)
{

}

async_handle_t* async_ev_file_new(zend_ulong fd, zend_ulong events)
{

}

async_handle_t* async_ev_socket_new(zend_ulong fd, zend_ulong events)
{

}

async_handle_t* async_ev_timeout_new(zend_ulong timeout)
{

}
async_handle_t* async_ev_signal_new(zend_long sig_number)
{

}
async_handle_t* async_ev_pipe_new(zend_ulong fd, zend_ulong events)
{

}
async_handle_t* async_ev_tty_new(zend_ulong fd, zend_ulong events)
{

}
async_handle_t* async_ev_file_system_new(zend_ulong fd, zend_ulong events)
{

}
async_handle_t* async_ev_process_new(zend_ulong pid, zend_ulong events)
{

}
async_handle_t* async_ev_thread_new(zend_ulong tread_id, zend_ulong events)
{

}
async_handle_t* async_ev_idle_new(void)
{

}


