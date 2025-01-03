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
#include "php_reactor.h"
#include "php_layer/exceptions.h"
#include "php_layer/notifier.h"
#include "php_layer/ev_handles.h"

void reactor_startup(void)
{
	async_throw_error("Reactor API method startup not implemented");
}

void reactor_shutdown(void)
{
	async_throw_error("Reactor API method shutdown not implemented");
}

#ifdef PHP_ASYNC_TRACK_HANDLES
/**
 * The method returns TRUE if the specified handle is already waiting in the event loop.
 * This check can detect complex errors in the application's operation.
 */
static zend_always_inline zend_bool reactor_handle_is_waiting(const zend_object *handle)
{
	// TODO: extract real handle->handle from zend_object
	zval *result = zend_hash_index_find(&ASYNC_G(linked_handles), handle->handle);
	const bool is_linked = result != NULL;
	zval_ptr_dtor(result);
	return is_linked;
}
#endif

void reactor_add_handle(reactor_handle_t *handle)
{
#ifdef PHP_ASYNC_TRACK_HANDLES
	if (reactor_handle_is_waiting(handle)) {
		async_throw_error("Cannot add a handle that is already waiting");
		return;
	}
#endif

	reactor_add_handle_ex_fn(handle);
}

static void reactor_handle_method_no(reactor_handle_t *handle)
{
	async_throw_error("Reactor API method handle_method not implemented");
}

ZEND_API zend_bool reactor_is_enabled(void)
{
	return reactor_startup_fn != reactor_startup;
}

static reactor_handle_t* reactor_handle_from_resource(zend_resource *resource, zend_ulong actions)
{
	async_throw_error("Reactor API method handle_from_resource not implemented");
	return NULL;
}

static reactor_handle_t* reactor_file_new(async_file_descriptor_t fd, zend_ulong events)
{
	async_throw_error("Reactor API method file_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_socket_new(php_socket_t fd, zend_ulong events)
{
	async_throw_error("Reactor API method socket_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_timeout_new(zend_ulong timeout, zend_ulong is_period)
{
	async_throw_error("Reactor API method timeout_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_signal_new(zend_long sig_number)
{
	async_throw_error("Reactor API method signal_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_pipe_new(async_file_descriptor_t fd, zend_ulong events)
{
	async_throw_error("Reactor API method pipe_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_tty_new(async_file_descriptor_t fd, zend_ulong events)
{
	async_throw_error("Reactor API method tty_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_file_system_new(const char *path, size_t length, zend_ulong events)
{
	async_throw_error("Reactor API method file_system_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_process_new(async_process_id_t pid, zend_ulong events)
{
	async_throw_error("Reactor API method process_new not implemented");
	return NULL;
}

static reactor_handle_t* reactor_thread_new(THREAD_T tread_id, zend_ulong events)
{
	async_throw_error("Reactor API method thread_new not implemented");
	return NULL;
}

static php_socket_t reactor_extract_os_socket_handle(reactor_handle_t *handle)
{
	async_throw_error("Reactor API method extract_os_socket_handle not implemented");
	return -1;
}

static async_file_descriptor_t reactor_extract_os_file_handle(reactor_handle_t *handle)
{
	async_throw_error("Reactor API method extract_os_file_handle not implemented");
	return NULL;
}

ZEND_API reactor_handle_t* reactor_default_object_create(zend_class_entry *class_entry)
{
	// This is function call from zend_API.c
	// ZEND_API zend_result object_and_properties_init(zval *arg, zend_class_entry *class_type, HashTable *properties)
	// => _object_and_properties_init
	//
	// This function is responsible for:
	// * Allocating memory
	// * Initializing properties
	//
	// It is inherited by all child objects. Therefore, you must take this into account!
	//

	reactor_handle_t * object = zend_object_alloc(sizeof(reactor_handle_t), class_entry);
	object->type = REACTOR_H_UNKNOWN;
	object->ctor = NULL;
	object->dtor = NULL;

	zend_object_std_init(&object->std, class_entry);
	object_properties_init(&object->std, class_entry);

	return object;
}

reactor_startup_t reactor_startup_fn = reactor_startup;
reactor_shutdown_t reactor_shutdown_fn = reactor_shutdown;

reactor_handle_method_t reactor_add_handle_ex_fn = reactor_handle_method_no;
reactor_handle_method_t reactor_remove_handle_fn = reactor_handle_method_no;

reactor_stop_t reactor_stop_fn = NULL;
reactor_loop_alive_t reactor_loop_alive_fn = NULL;

reactor_object_create_t reactor_object_create_fn = reactor_default_object_create;

reactor_handle_from_resource_t reactor_handle_from_resource_fn = reactor_handle_from_resource;
reactor_file_new_t reactor_file_new_fn = reactor_file_new;
reactor_socket_new_t reactor_socket_new_fn = reactor_socket_new;
reactor_pipe_new_t reactor_pipe_new_fn = reactor_pipe_new;
reactor_tty_new_t reactor_tty_new_fn = reactor_tty_new;

reactor_timer_new_t reactor_timer_new_fn = reactor_timeout_new;
reactor_signal_new_t reactor_signal_new_fn = reactor_signal_new;
reactor_process_new_t reactor_process_new_fn = reactor_process_new;
reactor_thread_new_t reactor_thread_new_fn = reactor_thread_new;

reactor_extract_os_socket_handle_t reactor_extract_os_socket_handle_fn = reactor_extract_os_socket_handle;
reactor_extract_os_file_handle_t reactor_extract_os_file_handle_fn = reactor_extract_os_file_handle;

reactor_file_system_new_t reactor_file_system_new_fn = reactor_file_system_new;