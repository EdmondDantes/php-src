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

#ifndef ASYNC_CALLBACK_H
#define ASYNC_CALLBACK_H

#include "resume.h"

extern ZEND_API zend_class_entry *async_ce_closure;

typedef struct {
	union
	{
		/* PHP object handle. */
		zend_object std;
		/* The properties table is used to store the object properties. */
		struct {
			char _padding[sizeof(zend_object) - sizeof(zval)];
			zval callback;
			HashTable * notifiers;
			zend_fiber * fiber;
			async_resume_t * resume;

			/**
			 * Internal link to the owning object.
			 * This object does not affect the REFCOUNT in any way,
			 * so the lifetime of the owner must be strictly equal to the lifetime of the callback object.
			 */
			zend_object * owner;
		};
	};
} async_closure_t;

typedef void (*async_callback_function_t)(zend_object * callback, zend_object * notifier, zval* event, zval* error);

void async_register_closure_ce(void);
zend_object * async_closure_new(async_callback_function_t callback);
async_closure_t * async_closure_new_with_owner(async_callback_function_t callback, zend_object * owner);
void async_closure_notify(zend_object *object, zend_object *notifier, zval *event, zval *error);
zend_result async_closure_bind_resume(zend_object* object, const zval* resume);
void async_closure_registered(zend_object* object, zend_object* notifier);
zend_object* async_closure_resolve_resume(const zend_object* object);

#endif //ASYNC_CALLBACK_H
