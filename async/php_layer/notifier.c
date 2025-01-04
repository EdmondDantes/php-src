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

METHOD(addCallback)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(callback, async_ce_callback)
	ZEND_PARSE_PARAMETERS_END();

	async_notifier_add_callback(Z_OBJ_P(ZEND_THIS), callback);

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
	return &object->std;
}

static void async_notifier_object_destroy(zend_object *object)
{
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

void async_notifier_add_callback(zend_object* notifier, zval* callback)
{
	const zval* callbacks = async_notifier_get_callbacks(notifier);

	if (zend_hash_index_find(Z_ARRVAL_P(callbacks), Z_OBJ_P(callback)->handle) != NULL) {
		return;
	}

	// Add the weak reference to the callbacks array.
	zend_hash_index_update(Z_ARRVAL_P(callbacks), Z_OBJ_P(callback)->handle, async_new_weak_reference_from(callback));

	// Link the callback and the notifier together.
	if (Z_OBJ_P(callback)->ce != async_ce_resume) {
		async_callback_registered(Z_OBJ_P(callback), notifier);
    }
}

/**
 * The method is used to remove the callback from the notifier.
 *
 * @param notifier The notifier object.
 * @param callback The callback object.
 */
void async_notifier_remove_callback(zend_object* notifier, zval* callback)
{
	zend_hash_index_del(Z_ARRVAL_P(async_notifier_get_callbacks(notifier)), Z_OBJ_P(callback)->handle);
}

void async_notifier_notify(reactor_notifier_t * notifier, zval * event, zval * error)
{
	const zval* callbacks = async_notifier_get_callbacks(&notifier->std);

	zval *current;
	zval resolved_callback;
	ZVAL_UNDEF(&resolved_callback);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)
		if (EXPECTED(Z_TYPE_P(current) == IS_OBJECT)) {
			async_resolve_weak_reference(current, &resolved_callback);

			// Resume object and Callback object use different handlers.
			if (Z_TYPE(resolved_callback) == IS_OBJECT && Z_OBJ_P(&resolved_callback)->ce == async_ce_resume) {
				async_resume_notify((async_resume_t *) Z_OBJ(resolved_callback), notifier, event, error);
			} else if (Z_TYPE(resolved_callback) == IS_OBJECT) {
				async_callback_notify(Z_OBJ(resolved_callback), &notifier->std, event, error);
			}

			IF_THROW_RETURN_VOID;
		}

	IF_THROW_RETURN_VOID;
	ZEND_HASH_FOREACH_END();
}