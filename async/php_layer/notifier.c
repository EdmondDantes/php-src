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

#include "zend_common.h"
#include "notifier.h"
#include "callback.h"
#include "notifier_arginfo.h"

#define METHOD(name) PHP_METHOD(Async_Notifier, name)
#define PROPERTY_CALLBACKS "callbacks"
#define GET_PROPERTY_CALLBACKS() zend_read_property(async_ce_notifier, Z_OBJ_P(ZEND_THIS), PROPERTY_CALLBACKS, \
		strlen(PROPERTY_CALLBACKS), 0, NULL);


static zend_object_handlers async_notifier_handlers;

METHOD(addCallback)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(callback, async_ce_callback)
	ZEND_PARSE_PARAMETERS_END();

	zval* callbacks = GET_PROPERTY_CALLBACKS();
	zval *current;
	zval resolved_callback;
	ZVAL_UNDEF(&resolved_callback);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)
		// Unreferenced the weak reference and compare the objects.
		if (Z_TYPE_P(current) == IS_OBJECT) {
			async_resolve_weak_reference(current, &resolved_callback);

			bool is_same = Z_OBJ_P(&resolved_callback) == Z_OBJ_P(callback);
			zval_ptr_dtor(&resolved_callback);

			if (is_same) {
				RETURN_ZVAL(ZEND_THIS, 1, 0);
			}
		}
	ZEND_HASH_FOREACH_END();

	if (async_callback_bind_resume(Z_OBJ_P(callback), ZEND_THIS) == FAILURE) {
		RETURN_THROWS();
	}

	zval* callback_ref = async_new_weak_reference_from(callback);

	add_next_index_zval(callbacks, callback_ref);
	Z_TRY_ADDREF_P(callback_ref);

	RETURN_ZVAL(ZEND_THIS, 1, 0);
}

METHOD(removeCallback)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(callback, async_ce_callback)
		ZEND_PARSE_PARAMETERS_END();

	const zval* callbacks = GET_PROPERTY_CALLBACKS();

	zval *current;
	zend_string *key;
	zend_ulong index;

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(callbacks), index, key, current)
		if (Z_TYPE_P(current) == IS_OBJECT && Z_OBJ_P(current) == Z_OBJ_P(callback)) {
			if (key) {
				zend_hash_del(Z_ARRVAL_P(callbacks), key);
			} else {
				zend_hash_index_del(Z_ARRVAL_P(callbacks), index);
			}
		}
	ZEND_HASH_FOREACH_END();

	RETURN_ZVAL(ZEND_THIS, 1, 0);
}

METHOD(notify)
{
	zval* event;
	zval* error = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_ZVAL(event)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_OR_NULL(error, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	const zval* callbacks = GET_PROPERTY_CALLBACKS();
	zval *current;

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)
		async_callback_notify(Z_OBJ_P(current), Z_OBJ_P(ZEND_THIS), event, error);
		IF_THROW_RETURN_VOID;
	ZEND_HASH_FOREACH_END();
}

void async_register_notifier_ce(void)
{
	async_ce_notifier = register_class_Async_Notifier();
	async_ce_notifier->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
}
