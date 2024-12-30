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
#include "zend_smart_str.h"
#include "zend_fibers.h"
#include "notifier.h"
#include "callback.h"
#include "notifier_arginfo.h"
#include "../php_reactor.h"

#define METHOD(name) PHP_METHOD(Async_Notifier, name)
#define PROPERTY_CALLBACKS "callbacks"
#define GET_PROPERTY_CALLBACKS() async_notifier_get_callbacks(Z_OBJ_P(ZEND_THIS));


static zend_object_handlers async_notifier_handlers;

void async_notifier_notify(reactor_notifier_t * notifier, const zval * event, const zval * error)
{
	const zval* callbacks = zend_read_property(
		async_ce_notifier, &notifier->std, PROPERTY_CALLBACKS, strlen(PROPERTY_CALLBACKS), 0, NULL
	);

	zval *current;
	zval resolved_callback;
	ZVAL_UNDEF(&resolved_callback);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)
		if (EXPECTED(Z_TYPE_P(current) == IS_OBJECT)) {
			async_resolve_weak_reference(current, &resolved_callback);

			if (Z_TYPE(resolved_callback) == IS_OBJECT) {
				async_callback_notify(Z_OBJ(resolved_callback), &notifier->std, event, error);
			}

			zval_ptr_dtor(&resolved_callback);

			IF_THROW_RETURN_VOID;
		}

	IF_THROW_RETURN_VOID;
	ZEND_HASH_FOREACH_END();
}

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

			bool is_the_same = Z_OBJ_P(&resolved_callback) == Z_OBJ_P(callback);
			zval_ptr_dtor(&resolved_callback);

			if (is_the_same) {
				RETURN_ZVAL(ZEND_THIS, 1, 0);
			}
		}
	ZEND_HASH_FOREACH_END();

	// Add the weak reference to the callbacks array.
	add_next_index_zval(callbacks, async_new_weak_reference_from(callback));
	// Link the callback and the notifier together.
	async_callback_registered(Z_OBJ_P(callback), (const zval*) Z_OBJ_P(ZEND_THIS));

	RETURN_ZVAL(ZEND_THIS, 1, 0);
}

METHOD(removeCallback)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(callback, async_ce_callback)
	ZEND_PARSE_PARAMETERS_END();

	async_notifier_remove_callback(Z_OBJ_P(ZEND_THIS), callback);

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

	async_notifier_notify((reactor_notifier_t *) Z_OBJ_P(ZEND_THIS), event, error);
}

/**
 * The method creates a notifier object marked as CUSTOM.
 */
zend_object *async_notifier_object_create(zend_class_entry *class_entry)
{
	reactor_handle_t * object = reactor_default_object_create(class_entry);
	object->type = REACTOR_H_CUSTOM;

	return &object->std;
}

static void async_notifier_object_destroy(zend_object *object)
{
	reactor_handle_t* handle = (reactor_handle_t *) object;

	if (handle->dtor != NULL) {
		handle->dtor(handle);
	}
}

void async_register_notifier_ce(void)
{
	async_ce_notifier = register_class_Async_Notifier();
	async_ce_notifier->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_notifier->create_object = async_notifier_object_create;

	async_ce_notifier->default_object_handlers = &async_notifier_handlers;

	async_notifier_handlers = std_object_handlers;
	async_notifier_handlers.dtor_obj = async_notifier_object_destroy;
	//async_notifier_handlers.free_obj = async_notifier_object_free;
	async_notifier_handlers.clone_obj = NULL;
}

/**
 * The method is used to remove the callback from the notifier.
 *
 * @param notifier The notifier object.
 * @param callback The callback object.
 */
void async_notifier_remove_callback(zend_object* notifier, const zval* callback)
{
	const zval* callbacks = zend_read_property(
		async_ce_notifier, notifier, PROPERTY_CALLBACKS, strlen(PROPERTY_CALLBACKS), 0, NULL
	);

	zval *current;
	zend_string *key;
	zend_ulong index;
	zval resolved_callback;
	ZVAL_UNDEF(&resolved_callback);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(callbacks), index, key, current)
		if (Z_TYPE_P(current) == IS_OBJECT) {
			async_resolve_weak_reference(current, &resolved_callback);

			const bool is_the_same = Z_OBJ_P(&resolved_callback) == Z_OBJ_P(callback);
			zval_ptr_dtor(&resolved_callback);

			if (is_the_same) {
				if (key) {
					zend_hash_del(Z_ARRVAL_P(callbacks), key);
				} else {
					zend_hash_index_del(Z_ARRVAL_P(callbacks), index);
				}
			}
		}
	ZEND_HASH_FOREACH_END();
}