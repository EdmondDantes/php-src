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

static zend_always_inline zval* async_callback_get_callback(zend_object* callback)
{
	return &callback->properties_table[0];
}

/**
 * This method is used to get the Notifiers array from the Callback object.
 *
 * The method returns a pointer to the HashTable.
 */
static zend_always_inline HashTable* async_callback_get_notifiers(const zend_object* callback)
{
	return Z_ARRVAL_P(&callback->properties_table[1]);
}

static zend_always_inline zval* async_callback_get_zval_notifiers(zend_object* callback)
{
	return &callback->properties_table[1];
}

static zend_always_inline zval* async_callback_get_resume(zend_object* callback)
{
	return &callback->properties_table[2];
}


void async_callback_notify(zend_object *callback, zend_object *notifier, const zval *event, const zval *error);
zend_result async_callback_bind_resume(zend_object* callback, const zval* resume);
void async_callback_registered(zend_object* callback, const zval* notifier);
zval* async_callback_get_resume(const zend_object* callback);

#endif //ASYNC_CALLBACK_H
