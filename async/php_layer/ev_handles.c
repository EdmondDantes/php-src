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
#include "ev_handles.h"
#include "notifier.h"
#include "../reactor.h"
#include "ev_handles_arginfo.h"
#include "exceptions.h"

#define CALL_INTERNAL_CTOR(...) \
		if (UNEXPECTED(((reactor_handle_t*)Z_OBJ_P(ZEND_THIS))->ctor == NULL)) { \
			async_throw_error("Failed to create event handle, class does not have a internal constructor"); \
			return; \
		} \
		((reactor_handle_t*)Z_OBJ_P(ZEND_THIS))->ctor((reactor_handle_t*)Z_OBJ_P(ZEND_THIS), ##__VA_ARGS__);

PHP_METHOD(Async_FiberHandle, __construct)
{
	async_throw_error("Fiber handle cannot be created directly");
}

PHP_METHOD(Async_EvHandle, __construct)
{
	zval *handle;
	zend_long actions = 0;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_ZVAL(handle)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(actions)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(handle, actions);
}

PHP_METHOD(Async_TimerHandle, __construct)
{
	zend_long microseconds;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(microseconds)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(microseconds);
}

PHP_METHOD(Async_SignalHandle, __construct)
{
	zend_long sig_number;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(sig_number)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(sig_number);
}

PHP_METHOD(Async_ProcessHandle, __construct)
{
	zend_long process;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(process)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(process);
}

PHP_METHOD(Async_ThreadHandle, __construct)
{
	zend_long thread;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(thread)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(thread);
}

PHP_METHOD(Async_FileSystemHandle, __construct)
{
	zval *path;
	zend_long flags = 0;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_ZVAL(path)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(flags)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(path, flags);
}

void async_register_handlers_ce(void)
{
	async_ce_ev_handle = register_class_Async_EvHandle(async_ce_notifier);
	async_ce_ev_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_fiber_handle = register_class_Async_FiberHandle(async_ce_notifier);
	async_ce_fiber_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_file_handle = register_class_Async_FileHandle(async_ce_ev_handle);
	async_ce_file_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_socket_handle = register_class_Async_SocketHandle(async_ce_ev_handle);
	async_ce_socket_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_pipe_handle = register_class_Async_PipeHandle(async_ce_ev_handle);
	async_ce_pipe_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_tty_handle = register_class_Async_TtyHandle(async_ce_ev_handle);
	async_ce_tty_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_timer_handle = register_class_Async_TimerHandle(async_ce_notifier);
	async_ce_timer_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_signal_handle = register_class_Async_SignalHandle(async_ce_notifier);
	async_ce_signal_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_process_handle = register_class_Async_ProcessHandle(async_ce_notifier);
	async_ce_process_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_thread_handle = register_class_Async_ThreadHandle(async_ce_notifier);
	async_ce_thread_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_file_system_handle = register_class_Async_FileSystemHandle(async_ce_notifier);
	async_ce_file_system_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
}