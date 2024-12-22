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

#ifndef NOTIFIER_H
#define NOTIFIER_H

#ifdef PHP_ASYNC_LIBUV
#include <uv.h>
#endif

ZEND_API zend_class_entry *async_ce_notifier;

typedef enum {
	ASYNC_UNKNOWN = 0,
	ASYNC_FILE = 1,
	ASYNC_SOCKET = 2,
	ASYNC_TIMER = 3,
	ASYNC_SIGNAL = 4,
	ASYNC_PIPE = 5,
	ASYNC_TTY = 6,
	ASYNC_FILE_SYSTEM = 7,
	ASYNC_PROCESS = 8,
	ASYNC_IDLE = 9,
	ASYNC_GET_ADDR_INFO = 10,
	ASYNC_GET_NAME_INFO = 11,
	ASYNC_CUSTOM = 128
} ASYNC_HANDLE_TYPE;

typedef struct _async_handle_s async_handle_t;
typedef struct _async_handle_s async_notifier_t;
typedef void (*async_handle_method)(async_handle_t *notifier);

struct _async_handle_s {
	/* PHP std object Async\Notifier */
	zend_object std;
	/**
	 * The type of handler that is hidden behind the Notify object.
	 */
	ASYNC_HANDLE_TYPE type;
	/**
	 * A method that is called when the notifier must be destroyed to remove the handler from the event loop.
	 */
	async_handle_method dtor;
	/**
	 * The handle that is used to identify the handler in the event loop.
	 */
	void *handle;
};

void async_register_notifier_ce(void);
void async_notifier_remove_callback(zend_object* notifier, const zval* callback);

#endif //NOTIFIER_H
