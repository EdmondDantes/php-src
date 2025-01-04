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
#ifndef PHP_REACTOR_H
#define PHP_REACTOR_H

#include <php.h>
#include "php_async.h"
#include "php_scheduler.h"
#include <Zend/zend_types.h>
#include "php_layer/reactor_handles.h"

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
 * reactor_handle_from_resource_t - Creates an event handle from a resource.
 * Converts a zend_resource to an event handle with specified actions.
 */
typedef reactor_handle_t* (*reactor_handle_from_resource_t)(zend_resource *resource, zend_ulong actions, REACTOR_HANDLE_TYPE expected_type);

/**
 * reactor_file_new_t - Creates a new file event handle.
 * Monitors file descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_file_new_t)(async_file_descriptor_t fd, zend_ulong events);

/**
 * reactor_socket_new_t - Creates a new socket event handle.
 * Monitors socket file descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_socket_new_t)(php_socket_t fd, zend_ulong events);

/**
 * reactor_timer_new_t - Creates a new timeout event handle.
 * Triggers an event after the specified timeout duration.
 */
typedef reactor_handle_t* (*reactor_timer_new_t)(zend_ulong timeout, zend_bool is_periodic);

/**
 * reactor_signal_new_t - Creates a new signal event handle.
 * Monitors the specified signal number.
 */
typedef reactor_handle_t* (*reactor_signal_new_t)(zend_long sig_number);

/**
 * reactor_pipe_new_t - Creates a new pipe event handle.
 * Monitors a pipe for specified events.
 */
typedef reactor_handle_t* (*reactor_pipe_new_t)(async_file_descriptor_t fd, zend_ulong events);

/**
 * reactor_tty_new_t - Creates a new TTY (terminal) event handle.
 * Monitors TTY file descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_tty_new_t)(async_file_descriptor_t fd, zend_ulong events);

/**
 * reactor_extract_os_socket_handle_t - Extracts the OS socket handle from an event handle.
 * Retrieves the underlying OS socket handle from the event handle.
 */
typedef php_socket_t (* reactor_extract_os_socket_handle_t)(reactor_handle_t *handle);

/**
 * reactor_extract_os_file_handle_t - Extracts the OS file handle from an event handle.
 * Retrieves the underlying OS file handle from the event handle.
 */
typedef async_file_descriptor_t (* reactor_extract_os_file_handle_t)(reactor_handle_t *handle);

/**
 * reactor_file_system_new_t - Creates a new filesystem event handle.
 * Monitors filesystem descriptor for specified events.
 */
typedef reactor_handle_t* (*reactor_file_system_new_t)(const char *path, size_t length, zend_ulong events);

/**
 * reactor_process_new_t - Creates a new process event handle.
 * Monitors process ID for specified events.
 */
typedef reactor_handle_t* (*reactor_process_new_t)(async_process_id_t pid, zend_ulong events);

/**
 * reactor_thread_new_t - Creates a new thread event handle.
 * Monitors thread ID for specified events.
 */
typedef reactor_handle_t* (*reactor_thread_new_t)(THREAD_T thread_id, zend_ulong events);

BEGIN_EXTERN_C()

ZEND_API zend_bool reactor_is_enabled(void);
ZEND_API void reactor_add_handle(reactor_handle_t *handle);

ZEND_API reactor_stop_t reactor_stop_fn;
ZEND_API reactor_loop_alive_t reactor_loop_alive_fn;

ZEND_API reactor_startup_t reactor_global_ctor_fn;
ZEND_API reactor_startup_t reactor_startup_fn;
ZEND_API reactor_shutdown_t reactor_shutdown_fn;

ZEND_API reactor_handle_method_t reactor_add_handle_ex_fn;
ZEND_API reactor_handle_method_t reactor_remove_handle_fn;

ZEND_API reactor_handle_from_resource_t reactor_handle_from_resource_fn;
ZEND_API reactor_file_new_t reactor_file_new_fn;
ZEND_API reactor_socket_new_t reactor_socket_new_fn;
ZEND_API reactor_timer_new_t reactor_timer_new_fn;
ZEND_API reactor_signal_new_t reactor_signal_new_fn;
ZEND_API reactor_pipe_new_t reactor_pipe_new_fn;
ZEND_API reactor_tty_new_t reactor_tty_new_fn;
ZEND_API reactor_extract_os_socket_handle_t reactor_extract_os_socket_handle_fn;
ZEND_API reactor_extract_os_file_handle_t reactor_extract_os_file_handle_fn;
ZEND_API reactor_file_system_new_t reactor_file_system_new_fn;
ZEND_API reactor_process_new_t reactor_process_new_fn;
ZEND_API reactor_thread_new_t reactor_thread_new_fn;

END_EXTERN_C()

#endif //PHP_REACTOR_H
