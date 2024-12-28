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

#ifndef ASYNC_NOTIFIER_H
#define ASYNC_NOTIFIER_H

ZEND_API zend_class_entry *async_ce_notifier;

typedef enum {
	REACTOR_H_UNKNOWN = 0,
	REACTOR_H_FILE = 1,
	REACTOR_H_SOCKET = 2,
	REACTOR_H_TIMER = 3,
	REACTOR_H_SIGNAL = 4,
	REACTOR_H_PIPE = 5,
	REACTOR_H_TTY = 6,
	REACTOR_H_FILE_SYSTEM = 7,
	REACTOR_H_PROCESS = 8,
	REACTOR_H_THREAD = 9,
	REACTOR_H_FIBER  = 64,
	REACTOR_H_CUSTOM = 128
} REACTOR_HANDLE_TYPE;

typedef struct _reactor_handle_s reactor_handle_t;
typedef struct _reactor_handle_s reactor_notifier_t;
typedef void (*reactor_handle_method)(reactor_handle_t *handle, ...);

struct _reactor_handle_s {
	/* PHP std object Async\Notifier */
	zend_object std;
	/**
	 * The type of handler that is hidden behind the Notify object.
	 */
	REACTOR_HANDLE_TYPE type;
	/**
	 * The method is called immediately after the constructor to complete the initialization of internal data structures.
	 */
	reactor_handle_method ctor;
	/**
	 * A method that is called when the notifier must be destroyed to remove the handler from the event loop.
	 */
	reactor_handle_method dtor;
};

void async_register_notifier_ce(void);
void async_notifier_add_callback(zend_object* notifier, const zval* callback);
void async_notifier_remove_callback(zend_object* notifier, const zval* callback);
void async_notifier_notify(reactor_notifier_t * notifier, const zval * event, const zval * error);

#endif //ASYNC_NOTIFIER_H
