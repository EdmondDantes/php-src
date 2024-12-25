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

#include "php_layer/exceptions.h"

void async_ev_startup(void)
{
	async_throw_error("Event Loop API method ev_startup not implemented");
}

void async_ev_shutdown(void)
{
	async_throw_error("Event Loop API method ev_shutdown not implemented");
}

#ifdef PHP_ASYNC_TRACK_HANDLES
/**
 * The method returns TRUE if the specified handle is already waiting in the event loop.
 * This check can detect complex errors in the application's operation.
 */
static zend_always_inline zend_bool async_ev_handle_is_waiting(const zend_object *handle)
{
	// TODO: extract real handle->handle from zend_object
	zval *result = zend_hash_index_find(&ASYNC_G(linked_handles), handle->handle);
	const bool is_linked = result != NULL;
	zval_ptr_dtor(result);
	return is_linked;
}
#endif

void async_ev_add_handle(async_ev_handle_t *handle)
{
#ifdef PHP_ASYNC_TRACK_HANDLES
	if (async_ev_handle_is_waiting(handle)) {
		async_throw_error("Cannot add a handle that is already waiting");
		return;
	}
#endif

	async_ev_add_handle_ex_fn(handle);
}

static void async_ev_handle_method_no(async_ev_handle_t *handle)
{
	async_throw_error("Event Loop API method ev_handle_method not implemented");
}

ZEND_API zend_bool async_ev_is_enabled(void)
{
	return async_ev_startup_fn != async_ev_startup;
}

static async_ev_handle_t* async_ev_handle_from_resource(zend_resource *resource, zend_ulong actions)
{
	async_throw_error("Event Loop API method ev_handle_from_resource not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_file_new(zend_ulong fd, zend_ulong events)
{
	async_throw_error("Event Loop API method ev_file_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_socket_new(zend_ulong fd, zend_ulong events)
{
	async_throw_error("Event Loop API method ev_socket_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_timeout_new(zend_ulong timeout)
{
	async_throw_error("Event Loop API method ev_timeout_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_signal_new(zend_long sig_number)
{
	async_throw_error("Event Loop API method ev_signal_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_pipe_new(zend_ulong fd, zend_ulong events)
{
	async_throw_error("Event Loop API method ev_pipe_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_tty_new(zend_ulong fd, zend_ulong events)
{
	async_throw_error("Event Loop API method ev_tty_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_file_system_new(zend_ulong fd, zend_ulong events)
{
	async_throw_error("Event Loop API method ev_file_system_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_process_new(zend_ulong pid, zend_ulong events)
{
	async_throw_error("Event Loop API method ev_process_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_thread_new(zend_ulong tread_id, zend_ulong events)
{
	async_throw_error("Event Loop API method ev_thread_new not implemented");
	return NULL;
}

static async_ev_handle_t* async_ev_idle_new(void)
{
	async_throw_error("Event Loop API method ev_idle_new not implemented");
	return NULL;
}

async_ev_startup_t async_ev_startup_fn = async_ev_startup;
async_ev_shutdown_t async_ev_shutdown_fn = async_ev_shutdown;

async_ev_handle_method_t async_ev_add_handle_ex_fn = async_ev_handle_method_no;
async_ev_handle_method_t async_ev_remove_handle_fn = async_ev_handle_method_no;

async_ev_loop_run_t async_ev_loop_run_fn = NULL;
async_ev_loop_stop_t async_ev_loop_stop_fn = NULL;
async_ev_loop_alive_t async_ev_loop_alive_fn = NULL;
async_ev_loop_set_microtask_handler async_ev_loop_set_microtask_handler_fn = NULL;
async_ev_loop_set_next_fiber_handler async_ev_loop_set_next_fiber_handler_fn = NULL;

async_ev_handle_from_resource_t async_ev_handle_from_resource_fn = async_ev_handle_from_resource;
async_ev_file_new_t async_ev_file_new_fn = async_ev_file_new;
async_ev_socket_new_t async_ev_socket_new_fn = async_ev_socket_new;
async_ev_timeout_new_t async_ev_timeout_new_fn = async_ev_timeout_new;
async_ev_signal_new_t async_ev_signal_new_fn = async_ev_signal_new;
async_ev_pipe_new_t async_ev_pipe_new_fn = async_ev_pipe_new;
async_ev_tty_new_t async_ev_tty_new_fn = async_ev_tty_new;
async_ev_file_system_new_t async_ev_file_system_new_fn = async_ev_file_system_new;
async_ev_process_new_t async_ev_process_new_fn = async_ev_process_new;
async_ev_thread_new_t async_ev_thread_new_fn = async_ev_thread_new;
async_ev_idle_new_t async_ev_idle_new_fn = async_ev_idle_new;