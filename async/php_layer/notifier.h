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

#include "php.h"

BEGIN_EXTERN_C()

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

typedef void (* reactor_notifier_handler_t) (reactor_notifier_t* notifier, zval* event, zval* error);
typedef bool (* reactor_remove_callback_t) (reactor_notifier_t * notifier, zval * callback);

struct _reactor_handle_s {
	union
	{
		/* PHP std object Async\Notifier */
		zend_object std;
		struct
		{
			char _padding[sizeof(zend_object) - sizeof(zval)];
			/**
			 * PHP array of callbacks.
			 * point to the std.properties_table[0]
			 */
			zval callbacks;

			// Padding memory zone for notify_fn, user_data
			zval _padding2;
		};
		struct
		{
			// zend object std + callbacks
			char _padding3[sizeof(zend_object)];

			// padding2 + padding3 memory zone

			/**
			 * Notify function.
			 * Called when the notifier is triggered.
			 */
			reactor_notifier_handler_t handler_fn;
			/**
			 * Remove callback function.
			 * Called when a callback is should be removed or when the notifier is destroyed.
			 */
			reactor_remove_callback_t remove_callback_fn;
		};
	};
};

static zend_always_inline zval* async_notifier_get_callbacks(zend_object* notifier)
{
	return &notifier->properties_table[0];
}

static zend_always_inline HashTable* async_notifier_get_callbacks_hash(const zend_object* notifier)
{
	return Z_ARRVAL_P(&notifier->properties_table[0]);
}

void async_register_notifier_ce(void);

zend_always_inline void async_notifier_object_init(reactor_notifier_t * notifier)
{
	notifier->handler_fn = NULL;
	notifier->remove_callback_fn = NULL;
}

ZEND_API reactor_notifier_t * async_notifier_new_ex(
	size_t size, reactor_notifier_handler_t handler_fn, reactor_remove_callback_t remove_callback_fn
);
ZEND_API reactor_notifier_t * async_notifier_new_by_class(const size_t size, zend_class_entry *class_entry);
ZEND_API void async_notifier_add_callback(zend_object* notifier, zval* callback);
ZEND_API void async_notifier_remove_callback(zend_object* notifier, zval* callback);
ZEND_API void async_notifier_notify(reactor_notifier_t * notifier, zval * event, zval * error);

END_EXTERN_C()

#endif //ASYNC_NOTIFIER_H
