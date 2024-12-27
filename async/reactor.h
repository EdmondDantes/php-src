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
#include "scheduler.h"
#include <Zend/zend_types.h>
#include "php_layer/ev_handles.h"

/**
 * reactor_startup_t - Function pointer type for initializing the event system.
 * This function performs necessary setup and starts the event loop.
 */
typedef void (*reactor_startup_t)(void);

/**
 * reactor_shutdown_t - Function pointer type for shutting down the event system.
 * This function performs cleanup and stops the event loop.
 */
typedef void (*reactor_shutdown_t)(void);

/**
 * reactor_run_t - Function pointer type for starting the event loop.
 * This function initiates the event loop and begins processing events.
 */
typedef void (*reactor_run_t)(void);

/**
 * reactor_stop_t - Function pointer type for stopping the event loop.
 * This function stops the event loop on the next iteration.
 */
typedef void (*reactor_stop_t)(void);

/**
 * reactor_loop_alive_t - Function pointer type for checking if the event loop is active.
 * This function returns whether the event loop still has active handles or tasks.
 */
typedef zend_bool (*reactor_loop_alive_t)(void);

/**
 * reactor_handle_method_t - Function pointer type for operating on an event handle.
 * This function performs an action on the specified handle.
 */
typedef void (*reactor_handle_method_t)(reactor_handle_t *handle);

/**
 * reactor_object_new_t - Creates a new event handle object.
 * Allocates memory for a new event handle object of the specified class.
 */
typedef reactor_handle_t* (*reactor_object_create_t)(zend_class_entry *class_entry);

/**
 * reactor_handle_from_resource_t - Creates an event handle from a resource.
 * Converts a zend_resource to an event handle with specified actions.
 */
typedef reactor_handle_t* (*reactor_handle_from_resource_t)(zend_resource *resource, zend_ulong actions);

/**
 * reactor_file_new_t - Creates a new file event handle.
 * Monitors file descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_file_new_t)(php_file_descriptor_t fd, zend_ulong events);

/**
 * reactor_socket_new_t - Creates a new socket event handle.
 * Monitors socket file descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_socket_new_t)(php_socket_t fd, zend_ulong events);

/**
 * reactor_timeout_new_t - Creates a new timeout event handle.
 * Triggers an event after the specified timeout duration.
 */
typedef reactor_handle_t* (*reactor_timeout_new_t)(zend_ulong timeout);

/**
 * reactor_signal_new_t - Creates a new signal event handle.
 * Monitors the specified signal number.
 */
typedef reactor_handle_t* (*reactor_signal_new_t)(zend_long sig_number);

/**
 * reactor_pipe_new_t - Creates a new pipe event handle.
 * Monitors a pipe for specified events.
 */
typedef reactor_handle_t* (*reactor_pipe_new_t)(php_file_descriptor_t fd, zend_ulong events);

/**
 * reactor_tty_new_t - Creates a new TTY (terminal) event handle.
 * Monitors TTY file descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_tty_new_t)(php_file_descriptor_t fd, zend_ulong events);

/**
 * reactor_file_system_new_t - Creates a new filesystem event handle.
 * Monitors filesystem descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_file_system_new_t)(zend_ulong fd, zend_ulong events);

/**
 * reactor_process_new_t - Creates a new process event handle.
 * Monitors process ID for specified events.
 */
typedef reactor_handle_t* (*reactor_process_new_t)(php_process_id_t pid, zend_ulong events);

/**
 * reactor_thread_new_t - Creates a new thread event handle.
 * Monitors thread ID for specified events.
 */
typedef reactor_handle_t* (*reactor_thread_new_t)(THREAD_T thread_id, zend_ulong events);

ZEND_API zend_bool reactor_is_enabled(void);
ZEND_API void reactor_add_handle(reactor_handle_t *handle);
ZEND_API void reactor_cancel_fiber(const zend_fiber *fiber, const zend_object *error);

/**
 * Reactor API function pointers.
 */

ZEND_API reactor_handle_t* reactor_default_object_create(zend_class_entry *class_entry);

ZEND_API reactor_stop_t reactor_stop_fn;
ZEND_API reactor_loop_alive_t reactor_loop_alive_fn;

ZEND_API reactor_startup_t reactor_global_ctor_fn;
ZEND_API reactor_startup_t reactor_startup_fn;
ZEND_API reactor_shutdown_t reactor_shutdown_fn;

ZEND_API reactor_handle_method_t reactor_add_handle_ex_fn;
ZEND_API reactor_handle_method_t reactor_remove_handle_fn;

ZEND_API reactor_object_create_t reactor_object_create_fn;

ZEND_API reactor_handle_from_resource_t reactor_handle_from_resource_fn;
ZEND_API reactor_file_new_t reactor_file_new_fn;
ZEND_API reactor_socket_new_t reactor_socket_new_fn;
ZEND_API reactor_timeout_new_t reactor_timeout_new_fn;
ZEND_API reactor_signal_new_t reactor_signal_new_fn;
ZEND_API reactor_pipe_new_t reactor_pipe_new_fn;
ZEND_API reactor_tty_new_t reactor_tty_new_fn;
ZEND_API reactor_file_system_new_t reactor_file_system_new_fn;
ZEND_API reactor_process_new_t reactor_process_new_fn;
ZEND_API reactor_thread_new_t reactor_thread_new_fn;

ZEND_API reactor_get_os_handle();

#endif //EVENT_LOOP_H
