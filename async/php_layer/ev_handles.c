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

///
/// This module defines the behavior of handle classes, which serve as event descriptors for the Reactor `event loop`.
/// The classes in this group can be instantiated in PHP but are managed by the `Reactor API`,
/// which can be overridden through extensions
///

#include <zend_fibers.h>
#include "ev_handles.h"
#include "notifier.h"
#include "exceptions.h"
#include "../php_reactor.h"
#include "ev_handles_arginfo.h"

#define THROW_IF_REACTOR_OFF \
	if (UNEXPECTED(reactor_object_create_fn == NULL)) { \
		async_throw_error("Failed to create event handle, no Reactor API implementation available"); \
		return; \
	}

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

	zend_object *error;

	ZEND_PARSE_PARAMETERS_START(1, 2)
	Z_PARAM_OBJECT_OF_CLASS(error, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	async_cancel_fiber(fiber, error);
}

PHP_METHOD(Async_EvHandle, __construct)
{
	THROW_IF_REACTOR_OFF;

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
	THROW_IF_REACTOR_OFF;

	zend_long microseconds;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(microseconds)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(microseconds);
}

PHP_METHOD(Async_SignalHandle, __construct)
{
	THROW_IF_REACTOR_OFF;

	zend_long sig_number;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(sig_number)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(sig_number);
}

PHP_METHOD(Async_ProcessHandle, __construct)
{
	THROW_IF_REACTOR_OFF;

	zend_long process;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(process)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(process);
}

PHP_METHOD(Async_ThreadHandle, __construct)
{
	THROW_IF_REACTOR_OFF;

	zend_long thread;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(thread)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(thread);
}

PHP_METHOD(Async_FileSystemHandle, __construct)
{
	THROW_IF_REACTOR_OFF;

	zval *path;
	zend_long flags = 0;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_ZVAL(path)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(flags)
	ZEND_PARSE_PARAMETERS_END();

	CALL_INTERNAL_CTOR(path, flags);
}
/**
 * Creates and initializes a new `fiber handle` object.
 *
 * This function allocates memory for a new `async_fiber_handle_t` object and initializes
 * its base `zend_object` structure. The fiber handle type is set to `REACTOR_H_FIBER`,
 * and the constructor and destructor pointers are initialized to NULL.
 * Standard object initialization routines are called to ensure the object is properly
 * integrated into the Zend object model.
 *
 * @param class_entry A pointer to the zend_class_entry representing the fiber class.
 *
 * @return A pointer to the newly created zend_object that represents the fiber.
 *
 * @note
 * - This function uses `zend_object_alloc` to allocate memory for the object, which ensures
 *   proper alignment and memory management.
 * - `zend_object_std_init` is responsible for setting up the object with default behavior
 *   (such as reference counting and GC).
 * - `object_properties_init` initializes object properties based on the class entry.
 * - The function returns a pointer to the `zend_object` part of the structure, which is
 *   compatible with Zend Engine's internal APIs.
 */
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

/**
 * Creates a new reactor object through a Reactor API.
 *
 * The `Reactor API` handler must correctly allocate memory for a data structure
 * that is compatible with the `reactor_handle_t` type (i.e., it must extend the specified type).
 * If the `Reactor API` is not defined,
 * calling the constructor will result in an exception indicating that the API is not available.
 * However, this function successfully allocates memory
 * because the Zend Engine mechanism does not allow exceptions to be thrown at this stage.
 *
 * @param class_entry A pointer to the zend_class_entry representing the reactor class.
 *
 * @return A pointer to the newly created zend_object that represents the reactor.
 *
 * @note
 * - If `reactor_object_create_fn` is not defined, the function calls `reactor_default_object_create()`,
 *   which handles the default object instantiation.
 */
static zend_object* reactor_object_create(zend_class_entry *class_entry)
{
	if (UNEXPECTED(reactor_object_create_fn == NULL)) {
		return (zend_object *) reactor_default_object_create(class_entry);
	}

	return (zend_object *) reactor_object_create_fn(class_entry);
}

static void reactor_object_destroy(zend_object *object)
{
	reactor_handle_t* handle = (reactor_handle_t *) object;

	if (handle->dtor != NULL) {
		handle->dtor(handle);
	}
}

static zend_object_handlers reactor_object_handlers;

void async_register_handlers_ce(void)
{
	// Create common handlers for reactor classes
	reactor_object_handlers = std_object_handlers;
	reactor_object_handlers.dtor_obj = reactor_object_destroy;
	reactor_object_handlers.clone_obj = NULL;

	async_ce_ev_handle = register_class_Async_EvHandle(async_ce_notifier);
	async_ce_ev_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_ev_handle->create_object = reactor_object_create;
	async_ce_ev_handle->default_object_handlers = &reactor_object_handlers;

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
	async_ce_timer_handle->create_object = reactor_object_create;
	async_ce_timer_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_signal_handle = register_class_Async_SignalHandle(async_ce_notifier);
	async_ce_signal_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_signal_handle->create_object = reactor_object_create;
	async_ce_signal_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_process_handle = register_class_Async_ProcessHandle(async_ce_notifier);
	async_ce_process_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_process_handle->create_object = reactor_object_create;
	async_ce_process_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_thread_handle = register_class_Async_ThreadHandle(async_ce_notifier);
	async_ce_thread_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_thread_handle->create_object = reactor_object_create;
	async_ce_thread_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_file_system_handle = register_class_Async_FileSystemHandle(async_ce_notifier);
	async_ce_file_system_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_file_system_handle->create_object = reactor_object_create;
	async_ce_file_system_handle->default_object_handlers = &reactor_object_handlers;
}