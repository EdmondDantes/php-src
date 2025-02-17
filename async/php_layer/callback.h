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

ZEND_API zend_class_entry *async_ce_callback;

typedef struct {
	union
	{
		/* PHP object handle. */
		zend_object std;
		/* The properties table is used to store the object properties. */
		struct {
			char _padding[sizeof(zend_object) - sizeof(zval)];
			zval callback;
			zval fiber;
			zval notifiers;
			zval resume;

			/**
			 * Internal link to the owning object.
			 * This object does not affect the REFCOUNT in any way,
			 * so the lifetime of the owner must be strictly equal to the lifetime of the callback object.
			 */
			zend_object * owner;
		};
	};
} async_callback_t;

static zend_always_inline zval* async_callback_get_callback(zend_object* callback)
{
	return &callback->properties_table[0];
}

static zend_always_inline zval* async_callback_get_fiber(zend_object* callback)
{
	return &callback->properties_table[1];
}

static zend_always_inline zend_fiber * async_callback_get_fiber_object(const zend_object* callback)
{
	const zval * zval_fiber = &callback->properties_table[1];

	if (Z_TYPE_P(zval_fiber) == IS_OBJECT) {
		return (zend_fiber *) Z_OBJ_P(zval_fiber);
	}

	return NULL;
}

/**
 * This method is used to get the Notifiers array from the Callback object.
 *
 * The method returns a pointer to the HashTable.
 */
static zend_always_inline HashTable* async_callback_get_notifiers(const zend_object* callback)
{
	return Z_ARRVAL_P(&callback->properties_table[2]);
}

static zend_always_inline zval* async_callback_get_zval_notifiers(zend_object* callback)
{
	return &callback->properties_table[2];
}

static zend_always_inline zval* async_callback_get_resume(zend_object* callback)
{
	return &callback->properties_table[3];
}

typedef void (*async_callback_function_t)(zend_object * callback, zend_object * notifier, zval* event, zval* error);

void async_register_callback_ce(void);
zend_object * async_callback_new(async_callback_function_t callback);
async_callback_t * async_callback_new_with_owner(async_callback_function_t callback, zend_object * owner);
void async_callback_notify(zend_object *callback, zend_object *notifier, zval *event, zval *error);
zend_result async_callback_bind_resume(zend_object* callback, const zval* resume);
void async_callback_registered(zend_object* callback, zend_object* notifier);
zend_object* async_callback_resolve_resume(const zend_object* callback);

#endif //ASYNC_CALLBACK_H
