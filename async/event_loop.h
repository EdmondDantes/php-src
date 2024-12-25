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
 * async_ev_startup_t - Function pointer type for initializing the event system.
 * This function performs necessary setup and starts the event loop.
 */
typedef void (*async_ev_startup_t)(void);

/**
 * async_ev_shutdown_t - Function pointer type for shutting down the event system.
 * This function performs cleanup and stops the event loop.
 */
typedef void (*async_ev_shutdown_t)(void);

/**
 * async_ev_loop_run_t - Function pointer type for starting the event loop.
 * This function initiates the event loop and begins processing events.
 */
typedef void (*async_ev_loop_run_t)(void);

/**
 * async_ev_loop_stop_t - Function pointer type for stopping the event loop.
 * This function stops the event loop on the next iteration.
 */
typedef void (*async_ev_loop_stop_t)(void);

/**
 * async_ev_loop_alive_t - Function pointer type for checking if the event loop is active.
 * This function returns whether the event loop still has active handles or tasks.
 */
typedef zend_bool (*async_ev_loop_alive_t)(void);

/**
 * async_ev_loop_set_microtask_handler - Function pointer type for setting the microtask execution handler.
 * Replaces the current handler responsible for executing pending microtasks in the event loop.
 * Returns the previous handler.
 *
 * @param handler The new microtask handler to be set.
 * @return The previous microtask handler.
 */
typedef async_execute_microtasks_handler_t (*async_ev_loop_set_microtask_handler)(async_execute_microtasks_handler_t handler);

/**
 * async_ev_loop_set_next_fiber_handler - Function pointer type for setting the next fiber resume handler.
 * Replaces the current handler responsible for resuming the next fiber in the event loop.
 * Returns the previous handler.
 *
 * @param handler The new fiber resume handler to be set.
 * @return The previous fiber resume handler.
 */
typedef async_resume_next_fiber_handler_t (*async_ev_loop_set_next_fiber_handler)(async_resume_next_fiber_handler_t handler);

/**
 * async_ev_handle_method_t - Function pointer type for operating on an event handle.
 * This function performs an action on the specified handle.
 */
typedef void (*async_ev_handle_method_t)(async_ev_handle_t *handle);

/**
 * async_ev_handle_from_resource_t - Creates an event handle from a resource.
 * Converts a zend_resource to an event handle with specified actions.
 */
typedef async_ev_handle_t* (*async_ev_handle_from_resource_t)(zend_resource *resource, zend_ulong actions);

/**
 * async_ev_file_new_t - Creates a new file event handle.
 * Monitors file descriptor for specified events.
 */
typedef async_ev_handle_t* (*async_ev_file_new_t)(zend_ulong fd, zend_ulong events);

/**
 * async_ev_socket_new_t - Creates a new socket event handle.
 * Monitors socket file descriptor for specified events.
 */
typedef async_ev_handle_t* (*async_ev_socket_new_t)(zend_ulong fd, zend_ulong events);

/**
 * async_ev_timeout_new_t - Creates a new timeout event handle.
 * Triggers an event after the specified timeout duration.
 */
typedef async_ev_handle_t* (*async_ev_timeout_new_t)(zend_ulong timeout);

/**
 * async_ev_signal_new_t - Creates a new signal event handle.
 * Monitors the specified signal number.
 */
typedef async_ev_handle_t* (*async_ev_signal_new_t)(zend_long sig_number);

/**
 * async_ev_pipe_new_t - Creates a new pipe event handle.
 * Monitors a pipe for specified events.
 */
typedef async_ev_handle_t* (*async_ev_pipe_new_t)(zend_ulong fd, zend_ulong events);

/**
 * async_ev_tty_new_t - Creates a new TTY (terminal) event handle.
 * Monitors TTY file descriptor for specified events.
 */
typedef async_ev_handle_t* (*async_ev_tty_new_t)(zend_ulong fd, zend_ulong events);

/**
 * async_ev_file_system_new_t - Creates a new filesystem event handle.
 * Monitors filesystem descriptor for specified events.
 */
typedef async_ev_handle_t* (*async_ev_file_system_new_t)(zend_ulong fd, zend_ulong events);

/**
 * async_ev_process_new_t - Creates a new process event handle.
 * Monitors process ID for specified events.
 */
typedef async_ev_handle_t* (*async_ev_process_new_t)(zend_ulong pid, zend_ulong events);

/**
 * async_ev_thread_new_t - Creates a new thread event handle.
 * Monitors thread ID for specified events.
 */
typedef async_ev_handle_t* (*async_ev_thread_new_t)(zend_ulong thread_id, zend_ulong events);

/**
 * async_ev_idle_new_t - Creates a new idle event handle.
 * Triggers an event when the event loop is idle.
 */
typedef async_ev_handle_t* (*async_ev_idle_new_t)(void);

ZEND_API zend_bool async_ev_is_enabled(void);
void async_ev_add_handle(async_ev_handle_t *handle);

ZEND_API async_ev_loop_run_t async_ev_loop_run_fn;
ZEND_API async_ev_loop_stop_t async_ev_loop_stop_fn;
ZEND_API async_ev_loop_alive_t async_ev_loop_alive_fn;

ZEND_API async_ev_loop_set_microtask_handler async_ev_loop_set_microtask_handler_fn;
ZEND_API async_ev_loop_set_next_fiber_handler async_ev_loop_set_next_fiber_handler_fn;

ZEND_API async_ev_startup_t async_ev_global_ctor_fn;
ZEND_API async_ev_startup_t async_ev_startup_fn;
ZEND_API async_ev_shutdown_t async_ev_shutdown_fn;

ZEND_API async_ev_handle_method_t async_ev_add_handle_ex_fn;
ZEND_API async_ev_handle_method_t async_ev_remove_handle_fn;

ZEND_API async_ev_handle_from_resource_t async_ev_handle_from_resource_fn;
ZEND_API async_ev_file_new_t async_ev_file_new_fn;
ZEND_API async_ev_socket_new_t async_ev_socket_new_fn;
ZEND_API async_ev_timeout_new_t async_ev_timeout_new_fn;
ZEND_API async_ev_signal_new_t async_ev_signal_new_fn;
ZEND_API async_ev_pipe_new_t async_ev_pipe_new_fn;
ZEND_API async_ev_tty_new_t async_ev_tty_new_fn;
ZEND_API async_ev_file_system_new_t async_ev_file_system_new_fn;
ZEND_API async_ev_process_new_t async_ev_process_new_fn;
ZEND_API async_ev_thread_new_t async_ev_thread_new_fn;
ZEND_API async_ev_idle_new_t async_ev_idle_new_fn;

#endif //EVENT_LOOP_H
