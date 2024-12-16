//
// Created by Edmond on 16.12.2024.
//

#include "zend_common.h"
#include "notifier.h"
#include "callback.h"
#include "notifier_arginfo.h"

#define METHOD(name) PHP_METHOD(Async_Notifier, name)
#define PROPERTY_CALLBACKS "callbacks"
#define GET_PROPERTY_CALLBACKS() zend_read_property(async_ce_notifier, Z_OBJ_P(ZEND_THIS), PROPERTY_CALLBACKS, strlen(PROPERTY_CALLBACKS), 0, NULL);


ZEND_API zend_class_entry *async_ce_notifier;
static zend_object_handlers async_notifier_handlers;

METHOD(addCallback)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(callback, async_ce_callback)
	ZEND_PARSE_PARAMETERS_END();

	zval* callbacks = GET_PROPERTY_CALLBACKS();

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)
		if (Z_TYPE_P(current) == IS_OBJECT && Z_OBJ_P(current) == Z_OBJ_P(callback)) {
			RETURN_ZVAL(ZEND_THIS, 1, 0);
		}
	ZEND_HASH_FOREACH_END();

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

	zval* callbacks = GET_PROPERTY_CALLBACKS();

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)
		if (Z_TYPE_P(current) == IS_OBJECT && Z_OBJ_P(current) == Z_OBJ_P(callback)) {
			zend_hash_del(__ht, zend_hash_get_current_key_zval(__ht));
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

	zval* callbacks = GET_PROPERTY_CALLBACKS();

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)
		async_callback_notify(Z_OBJ_P(current), ZEND_THIS, event, error);
		IF_THROW_RETURN_VOID;
	ZEND_HASH_FOREACH_END();
}

void async_register_notifier_ce(void)
{
	async_ce_notifier = register_class_Async_Notifier();
	async_ce_notifier->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
}
