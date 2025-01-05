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
#ifndef EV_HANDLES_H
#define EV_HANDLES_H

#include <php_network.h>

#include "php.h"
#include "zend_types.h"
#include "notifier.h"

// After parent property callbacks
#define TRIGGERED_EVENTS_INDEX 1

/**
 * Data structure for describing FIBER objects, which are represented in PHP as the FiberHandle class.
 */
typedef struct _reactor_fiber_handle_s reactor_fiber_handle_t;

/**
 * Data structure for describing POLL objects, which are represented in PHP as the PollHandle class.
 * The separation of file/socket descriptors is due to implementation specifics for Win32 and does not apply to UNIX.
 */
typedef struct _reactor_poll_s reactor_poll_t;

/**
 * Data structure for describing TIMER objects, which are represented in PHP as the TimerHandle class.
 */
typedef struct _reactor_timer_s reactor_timer_t;

/**
 * Data structure for describing SIGNAL objects, which are represented in PHP as the SignalHandle class.
 */
typedef struct _reactor_signal_s reactor_signal_t;

/**
 * Data structure for describing PROCESS objects, which are represented in PHP as the ProcessHandle class.
 */
typedef struct _reactor_process_s reactor_process_t;

/**
 * Data structure for describing THREAD objects, which are represented in PHP as the ThreadHandle class.
 */
typedef struct _reactor_thread_s reactor_thread_t;

/**
 * Data structure for describing FILE SYSTEM objects, which are represented in PHP as the FileSystemHandle class.
 */
typedef struct _reactor_file_system_s reactor_file_system_t;

struct _reactor_fiber_handle_s {
	reactor_handle_t handle;
	zend_fiber *fiber;
};

struct _reactor_poll_s {
	reactor_handle_t handle;
	zval triggered_events;
	union {
		async_file_descriptor_t file;
		php_socket_t socket;
	};
	int events;
};

struct _reactor_timer_s {
	reactor_handle_t handle;
	zval microseconds;
	zval is_periodic;
};

struct _reactor_signal_s {
	reactor_handle_t handle;
	zval number;
};

struct _reactor_process_s {
	reactor_handle_t handle;
	zval pid;
	zval exit_code;
};

struct _reactor_thread_s {
	reactor_handle_t handle;
	zval tid;
};

struct _reactor_file_system_s {
	reactor_handle_t handle;
	zval triggered_events;
	zval path;
	zval flags;
};


/**
 * Return the triggered events property of the given object.
 */
static zend_always_inline zval * async_ev_handle_get_triggered_events(zend_object *object)
{
	return &object->properties_table[TRIGGERED_EVENTS_INDEX];
}

static zend_always_inline zval * async_timer_get_microseconds(zend_object *object)
{
	return &object->properties_table[1];
}

static zend_always_inline zval * async_timer_get_is_periodic(zend_object *object)
{
	return &object->properties_table[2];
}

static zend_always_inline zval * async_signal_get_number(zend_object *object)
{
	return &object->properties_table[1];
}

/**
 * Return the triggered events property of the given object.
 */
static zend_always_inline zval * async_file_system_get_triggered_events(zend_object *object)
{
	return &object->properties_table[TRIGGERED_EVENTS_INDEX];
}

static zend_always_inline zval * async_file_system_get_path(zend_object *object)
{
	return &object->properties_table[TRIGGERED_EVENTS_INDEX + 1];
}

static zend_always_inline zval * async_file_system_get_flags(zend_object *object)
{
	return &object->properties_table[TRIGGERED_EVENTS_INDEX + 2];
}

BEGIN_EXTERN_C()

ZEND_API zend_class_entry *async_ce_poll_handle;

ZEND_API zend_class_entry *async_ce_fiber_handle;

ZEND_API zend_class_entry *async_ce_file_handle;
ZEND_API zend_class_entry *async_ce_socket_handle;
ZEND_API zend_class_entry *async_ce_pipe_handle;
ZEND_API zend_class_entry *async_ce_tty_handle;

ZEND_API zend_class_entry *async_ce_timer_handle;

ZEND_API zend_class_entry *async_ce_signal_handle;
ZEND_API zend_class_entry *async_ce_process_handle;
ZEND_API zend_class_entry *async_ce_thread_handle;

ZEND_API zend_class_entry *async_ce_file_system_handle;

END_EXTERN_C()

/**
 * The method registers all classes in the Handles group,
 * which are responsible for creating handler objects that can be added to the reactor event loop.
 */
void async_register_handlers_ce(void);

#endif //EV_HANDLES_H
