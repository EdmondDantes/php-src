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

#include <zend_fibers.h>

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

#define GET_FIBER_FROM_HANDLE() ((async_fiber_handle_t*)Z_OBJ_P(ZEND_THIS))->fiber
#define RETURN_IF_FIBER_INTERNAL_ERROR(fiber) if (UNEXPECTED((fiber) == NULL)) { \
		async_throw_error("Internal FiberHandle initialization error."); \
		return; \
	}

PHP_METHOD(Async_FiberHandle, __construct)
{
	async_throw_error("Fiber handles cannot be created directly");
}

PHP_METHOD(Async_FiberHandle, isStarted)
{
	const zend_fiber *fiber = GET_FIBER_FROM_HANDLE();

	RETURN_IF_FIBER_INTERNAL_ERROR(fiber);

	// @see zend_fiber::isStarted
	RETURN_BOOL(fiber->context.status == ZEND_FIBER_STATUS_RUNNING || fiber->caller != NULL);
}

PHP_METHOD(Async_FiberHandle, isSuspended)
{
	const zend_fiber *fiber = GET_FIBER_FROM_HANDLE();

	RETURN_IF_FIBER_INTERNAL_ERROR(fiber);

	// @see zend_fiber::isSuspended
	RETURN_BOOL(fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED && fiber->caller == NULL);
}

PHP_METHOD(Async_FiberHandle, isRunning)
{
	const zend_fiber *fiber = GET_FIBER_FROM_HANDLE();

	RETURN_IF_FIBER_INTERNAL_ERROR(fiber);

	RETURN_BOOL(fiber->context.status == ZEND_FIBER_STATUS_RUNNING || fiber->caller != NULL);
}

PHP_METHOD(Async_FiberHandle, isTerminated)
{
	const zend_fiber *fiber = GET_FIBER_FROM_HANDLE();

	RETURN_IF_FIBER_INTERNAL_ERROR(fiber);

	RETURN_BOOL(fiber->context.status == ZEND_FIBER_STATUS_DEAD);
}

PHP_METHOD(Async_FiberHandle, getContext)
{
	zend_fiber *fiber = GET_FIBER_FROM_HANDLE();

	RETURN_IF_FIBER_INTERNAL_ERROR(fiber);

	if (fiber->user_local_storage) {
		RETURN_ARR(fiber->user_local_storage);
	} else {
		ALLOC_HASHTABLE(fiber->user_local_storage);
		zend_hash_init(fiber->user_local_storage, 0, NULL, ZVAL_PTR_DTOR, 0);
		RETURN_ARR(fiber->user_local_storage);
	}
}

PHP_METHOD(Async_FiberHandle, cancelWith)
{
	const zend_fiber *fiber = GET_FIBER_FROM_HANDLE();

	RETURN_IF_FIBER_INTERNAL_ERROR(fiber);

	const zend_object *error;

	ZEND_PARSE_PARAMETERS_START(1, 2)
	Z_PARAM_OBJECT_OF_CLASS(error, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	reactor_cancel_fiber(fiber, error);
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

static zend_object* async_fiber_object_create(zend_class_entry *class_entry)
{
	async_fiber_handle_t * object = zend_object_alloc(sizeof(async_fiber_handle_t), class_entry);
	object->handle.type = REACTOR_H_FIBER;
	object->handle.ctor = NULL;
	object->handle.dtor = NULL;
	object->fiber = NULL;

	zend_object_std_init(&object->handle.std, class_entry);
	object_properties_init(&object->handle.std, class_entry);

	return &object->handle.std;
}

void async_register_handlers_ce(void)
{
	async_ce_ev_handle = register_class_Async_EvHandle(async_ce_notifier);
	async_ce_ev_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_fiber_handle = register_class_Async_FiberHandle(async_ce_notifier);
	async_ce_fiber_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_fiber_handle->create_object = async_fiber_object_create;

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