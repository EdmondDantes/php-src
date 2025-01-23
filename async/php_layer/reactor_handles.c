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

#include "zend_fibers.h"
#include "reactor_handles.h"
#include "notifier.h"
#include "zend_exceptions.h"
#include "exceptions.h"
#include "../php_reactor.h"
#include "reactor_handles_arginfo.h"
#include "zend_common.h"

#define THROW_IF_REACTOR_DISABLED \
	if (UNEXPECTED(reactor_is_enabled == false)) { \
		async_throw_error("Failed to create event handle, no Reactor API implementation available"); \
		return; \
	}

#define GET_FIBER_FROM_HANDLE() ((reactor_fiber_handle_t*)Z_OBJ_P(ZEND_THIS))->fiber
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

	if (fiber->fiber_storage == NULL) {
		fiber->fiber_storage = zend_fiber_storage_new();
	}

	RETURN_OBJ_COPY(fiber->fiber_storage);
}

PHP_METHOD(Async_FiberHandle, cancelWith)
{
	const zend_fiber *fiber = GET_FIBER_FROM_HANDLE();

	RETURN_IF_FIBER_INTERNAL_ERROR(fiber);

	zval *error;

	ZEND_PARSE_PARAMETERS_START(1, 2)
	Z_PARAM_OBJECT_OF_CLASS(error, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	async_cancel_fiber(fiber, Z_OBJ_P(error));
}

PHP_METHOD(Async_PollHandle, __construct) {}

PHP_METHOD(Async_PollHandle, isListening)
{
	RETURN_BOOL(reactor_is_listening_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS)));
}

PHP_METHOD(Async_PollHandle, stop)
{
	reactor_remove_handle_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS));
}

PHP_METHOD(Async_FileHandle, fromResource)
{
	THROW_IF_REACTOR_DISABLED;

	zval *zval_handle;
	zend_long actions = ASYNC_READABLE | ASYNC_WRITABLE;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_RESOURCE(zval_handle)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(actions)
	ZEND_PARSE_PARAMETERS_END();

	reactor_handle_t * handle = reactor_handle_from_resource_fn(Z_RES_P(zval_handle), actions, REACTOR_H_FILE);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&handle->std);
}

PHP_METHOD(Async_SocketHandle, fromResource)
{
	THROW_IF_REACTOR_DISABLED;

	zval *zval_handle;
	zend_long actions = ASYNC_READABLE | ASYNC_WRITABLE;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_RESOURCE(zval_handle)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(actions)
	ZEND_PARSE_PARAMETERS_END();

	reactor_handle_t * handle = reactor_handle_from_resource_fn(Z_RES_P(zval_handle), actions, REACTOR_H_SOCKET);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&handle->std);
}

PHP_METHOD(Async_SocketHandle, fromSocket)
{
	THROW_IF_REACTOR_DISABLED;

	zval *zval_handle;
	zend_long actions = ASYNC_READABLE | ASYNC_WRITABLE;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_OBJECT(zval_handle)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(actions)
	ZEND_PARSE_PARAMETERS_END();

	// TODO: integration with Socket LIB

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	//RETURN_OBJ(&handle->std);
}

PHP_METHOD(Async_TimerHandle, __construct) {}

PHP_METHOD(Async_TimerHandle, isListening)
{
	RETURN_BOOL(reactor_is_listening_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS)));
}

PHP_METHOD(Async_TimerHandle, stop)
{
	reactor_remove_handle_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS));
}

PHP_METHOD(Async_TimerHandle, newTimeout)
{
	THROW_IF_REACTOR_DISABLED;

	zend_long microseconds;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(microseconds)
	ZEND_PARSE_PARAMETERS_END();

	reactor_handle_t * handle = reactor_timer_new_fn(microseconds, false);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&handle->std);
}

PHP_METHOD(Async_TimerHandle, newInterval)
{
	THROW_IF_REACTOR_DISABLED;

	zend_long microseconds;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(microseconds)
	ZEND_PARSE_PARAMETERS_END();

	reactor_handle_t * handle = reactor_timer_new_fn(microseconds, true);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&handle->std);
}

PHP_METHOD(Async_SignalHandle, __construct) {}

PHP_METHOD(Async_SignalHandle, isListening)
{
	RETURN_BOOL(reactor_is_listening_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS)));
}

PHP_METHOD(Async_SignalHandle, stop)
{
	reactor_remove_handle_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS));
}

PHP_METHOD(Async_SignalHandle, new)
{
	THROW_IF_REACTOR_DISABLED;

	zend_long sig_number;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(sig_number)
	ZEND_PARSE_PARAMETERS_END();

	reactor_handle_t * handle = reactor_signal_new_fn(sig_number);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&handle->std);
}

PHP_METHOD(Async_ProcessHandle, __construct)
{
	async_throw_error("The object of Async\\ProcessHandle class cannot be instantiated from PHP");
}

PHP_METHOD(Async_ThreadHandle, __construct)
{
	async_throw_error("The object of Async\\ThreadHandle class cannot be instantiated from PHP");
}

PHP_METHOD(Async_FileSystemHandle, __construct)
{
}

PHP_METHOD(Async_FileSystemHandle, isListening)
{
	RETURN_BOOL(reactor_is_listening_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS)));
}

PHP_METHOD(Async_FileSystemHandle, stop)
{
	reactor_remove_handle_fn((reactor_handle_t *) Z_OBJ_P(ZEND_THIS));
}

PHP_METHOD(Async_DnsInfoHandle, resolveHost)
{
	THROW_IF_REACTOR_DISABLED;

	zend_string *host;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(host)
	ZEND_PARSE_PARAMETERS_END();

	reactor_dns_info_t * dns_info = (reactor_dns_info_t *) reactor_dns_info_new_fn(host, NULL, NULL, NULL);

	if (dns_info == NULL) {
        RETURN_THROWS();
    }

	ZVAL_NULL(&dns_info->address);
	ZVAL_STR(&dns_info->host, zend_string_copy(host));

	RETURN_OBJ(&dns_info->handle.std);
}

PHP_METHOD(Async_DnsInfoHandle, resolveAddress)
{
	THROW_IF_REACTOR_DISABLED;

	zend_string *address;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(address)
	ZEND_PARSE_PARAMETERS_END();

	reactor_dns_info_t * dns_info = (reactor_dns_info_t *) reactor_dns_info_new_fn(NULL, NULL, address, NULL);

	if (dns_info == NULL) {
		RETURN_THROWS();
	}

	ZVAL_STR(&dns_info->address, zend_string_copy(address));
	ZVAL_NULL(&dns_info->host);

	RETURN_OBJ(&dns_info->handle.std);
}

PHP_METHOD(Async_DnsInfoHandle, __construct) {}

PHP_METHOD(Async_DnsInfoHandle, __toString)
{
	const reactor_dns_info_t * dns_info = (reactor_dns_info_t *) Z_OBJ_P(ZEND_THIS);

	if (Z_TYPE(dns_info->address) == IS_STRING) {

		zend_string *prefix = zend_string_init("Dns address: ", sizeof("Dns address: ") - 1, 0);

		zend_string *result = zend_string_concat2(
		ZSTR_VAL(prefix), ZSTR_LEN(prefix),
		ZSTR_VAL(Z_STR(dns_info->address)), ZSTR_LEN(Z_STR(dns_info->address))
		);

		zend_string_free(prefix);

        RETURN_STR(result);
    } else if (Z_TYPE(dns_info->host) == IS_STRING) {

    	zend_string *prefix = zend_string_init("Dns host: ", sizeof("Dns host: ") - 1, 0);

	    zend_string *result = zend_string_concat2(
	    ZSTR_VAL(prefix), ZSTR_LEN(prefix),
	    ZSTR_VAL(Z_STR(dns_info->host)), ZSTR_LEN(Z_STR(dns_info->host))
        );

    	zend_string_free(prefix);

        RETURN_STR(result);
    } else {
        RETURN_STR(zend_string_init("Dns info", sizeof("Dns info") - 1, 0));
    }
}

PHP_METHOD(Async_FileSystemHandle, fromPath)
{
	THROW_IF_REACTOR_DISABLED;

	zval *path;
	zend_long flags = 0;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_ZVAL(path)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(flags)
	ZEND_PARSE_PARAMETERS_END();

	reactor_handle_t * handle = reactor_file_system_new_fn(Z_STRVAL_P(path), Z_STRLEN_P(path), flags);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&handle->std);
}

/**
 * Creates and initializes a new `fiber handle` object.
 *
 * This function allocates memory for a new `reactor_fiber_handle_t` object and initializes
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
	reactor_fiber_handle_t * object = zend_object_alloc_ex(sizeof(reactor_fiber_handle_t), class_entry);
	object->fiber = NULL;

	zend_object_std_init(&object->handle.std, class_entry);
	object_properties_init(&object->handle.std, class_entry);
	async_notifier_object_init(&object->handle);

	return &object->handle.std;
}

static void reactor_fiber_handle_destroy(zend_object* object)
{
	reactor_fiber_handle_t * handle = (reactor_fiber_handle_t *) object;

	if (handle->fiber != NULL) {
		zend_fiber_remove_defer(handle->fiber, handle->callback_index);
		OBJ_RELEASE(&handle->fiber->std);
	}

	handle->fiber = NULL;

	// call parent dtor function
	async_ce_notifier->default_object_handlers->dtor_obj(object);
}

static zend_object_handlers reactor_object_handlers;
static zend_object_handlers reactor_fiber_handle_handlers;

void async_register_handlers_ce(void)
{
	// Create common handlers for reactor classes
	// Copy the notifier object handlers and replace the clone_obj method with NULL
	reactor_object_handlers = *async_ce_notifier->default_object_handlers;
	reactor_object_handlers.clone_obj = NULL;

	async_ce_poll_handle = register_class_Async_PollHandle(async_ce_notifier);
	async_ce_poll_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_poll_handle->create_object = NULL;
	async_ce_poll_handle->default_object_handlers = &reactor_object_handlers;

	reactor_fiber_handle_handlers = *async_ce_notifier->default_object_handlers;
	reactor_fiber_handle_handlers.clone_obj = NULL;
	reactor_fiber_handle_handlers.dtor_obj = reactor_fiber_handle_destroy;

	async_ce_fiber_handle = register_class_Async_FiberHandle(async_ce_notifier);
	async_ce_fiber_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_fiber_handle->create_object = async_fiber_object_create;
	async_ce_fiber_handle->default_object_handlers = &reactor_fiber_handle_handlers;

	async_ce_file_handle = register_class_Async_FileHandle(async_ce_poll_handle);
	async_ce_file_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_socket_handle = register_class_Async_SocketHandle(async_ce_poll_handle);
	async_ce_socket_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_pipe_handle = register_class_Async_PipeHandle(async_ce_poll_handle);
	async_ce_pipe_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_tty_handle = register_class_Async_TtyHandle(async_ce_poll_handle);
	async_ce_tty_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_timer_handle = register_class_Async_TimerHandle(async_ce_notifier);
	async_ce_timer_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_timer_handle->create_object = NULL;
	async_ce_timer_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_signal_handle = register_class_Async_SignalHandle(async_ce_notifier);
	async_ce_signal_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_signal_handle->create_object = NULL;
	async_ce_signal_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_process_handle = register_class_Async_ProcessHandle(async_ce_notifier);
	async_ce_process_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_process_handle->create_object = NULL;
	async_ce_process_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_thread_handle = register_class_Async_ThreadHandle(async_ce_notifier);
	async_ce_thread_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_thread_handle->create_object = NULL;
	async_ce_thread_handle->default_object_handlers = &reactor_object_handlers;

	async_ce_dns_info = register_class_Async_DnsInfoHandle(async_ce_notifier);
	async_ce_dns_info->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_dns_info->create_object = NULL;
	async_ce_dns_info->default_object_handlers = &reactor_object_handlers;

	async_ce_file_system_handle = register_class_Async_FileSystemHandle(async_ce_notifier);
	async_ce_file_system_handle->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_file_system_handle->create_object = NULL;
	async_ce_file_system_handle->default_object_handlers = &reactor_object_handlers;
}

static void async_fiber_handle_defer_cb(zend_fiber * fiber, zend_fiber_defer_entry * entry)
{
	zval event;
	ZVAL_OBJ(&event, fiber);
	async_notifier_notify((reactor_notifier_t *) entry->object, &event, NULL);
}

reactor_fiber_handle_t * async_fiber_handle_new(zend_fiber * fiber)
{
	if (fiber == NULL) {
		fiber = EG(active_fiber);
	}

	if (fiber == NULL) {
		return NULL;
	}

	DEFINE_ZEND_INTERNAL_OBJECT(reactor_fiber_handle_t, handle, async_ce_fiber_handle);
	async_notifier_object_init(&handle->handle);

	handle->fiber = fiber;
	GC_ADDREF(&fiber->std);

	zend_fiber_defer_entry * entry = emalloc(sizeof(zend_fiber_defer_entry));
	entry->object = &handle->handle.std;
	entry->func = async_fiber_handle_defer_cb;
	entry->without_dtor = true;

	handle->callback_index = zend_fiber_defer(handle->fiber, entry, true);

	return handle;
}