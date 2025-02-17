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
#include "callback.h"
#include "notifier.h"
#include "callback_arginfo.h"
#include "../php_async.h"

#define METHOD(name) PHP_METHOD(Async_Callback, name)

#define PROPERTY_CALLBACK "callback"
#define GET_PROPERTY_CALLBACK() async_callback_get_callback(Z_OBJ_P(ZEND_THIS))

#define PROPERTY_FIBER "fiber"
#define GET_PROPERTY_FIBER() async_callback_get_fiber(Z_OBJ_P(ZEND_THIS))

#define PROPERTY_NOTIFIERS "notifiers"
#define GET_PROPERTY_NOTIFIERS() async_callback_get_notifiers(Z_OBJ_P(ZEND_THIS))

#define PROPERTY_RESUME "resume"
#define GET_PROPERTY_RESUME() async_callback_get_resume(Z_OBJ_P(ZEND_THIS))

static zend_object_handlers async_callback_handlers;

METHOD(__construct)
{
	zval* callable;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable)
	ZEND_PARSE_PARAMETERS_END();

	if (!zend_is_callable(callable, 0, NULL)) {
		zend_argument_value_error(1, "Expected parameter to be a valid callable");
		RETURN_THROWS();
	}

	if (EXPECTED(EG(active_fiber) != NULL)) {
		ZVAL_OBJ_COPY(GET_PROPERTY_FIBER(), (zend_object *) EG(active_fiber));
	}

	zval_property_copy(GET_PROPERTY_CALLBACK(), callable);
}

METHOD(disposeCallback)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zval* notifiers = async_callback_get_zval_notifiers(Z_OBJ_P(ZEND_THIS));

	zval *current;

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(notifiers), current)
		if (Z_TYPE_P(current) == IS_OBJECT) {
			async_notifier_remove_callback(Z_OBJ_P(current), ZEND_THIS);
		}
	ZEND_HASH_FOREACH_END();

	// Destroy the notifiers.
	zval_ptr_dtor(notifiers);

	// And create a new empty array.
	array_init(notifiers);
}

/**
 * The method is called before the destruction
 * of the object and notifies all Notifiers about it.
 */
static void async_callback_object_destroy(zend_object* object)
{
	// Notify the notifiers about the destruction of the Callback object.
	const HashTable* notifiers = async_callback_get_notifiers(object);

	zval* current;
	zval current_object;
	ZVAL_OBJ(&current_object, object);

	ZEND_HASH_FOREACH_VAL(notifiers, current)
		if (Z_TYPE_P(current) == IS_OBJECT) {
			async_notifier_remove_callback(Z_OBJ_P(current), &current_object);
        }
	ZEND_HASH_FOREACH_END();
}

void async_register_callback_ce(void)
{
	async_ce_callback = register_class_Async_Callback();
	async_ce_callback->default_object_handlers = &async_callback_handlers;

	async_callback_handlers = std_object_handlers;
	async_callback_handlers.dtor_obj = async_callback_object_destroy;
	async_callback_handlers.clone_obj = NULL;
}

zend_object * async_callback_new(const async_callback_function_t callback)
{
	zval object;

	// Create a new object without calling the constructor
	if (object_init_ex(&object, async_ce_callback) == FAILURE) {
		return NULL;
	}

	ZVAL_PTR(async_callback_get_callback(Z_OBJ_P(&object)), callback);

	return Z_OBJ_P(&object);
}

async_callback_t * async_callback_new_with_owner(const async_callback_function_t callback, zend_object * owner)
{
	async_callback_t * object = (async_callback_t *) zend_object_internal_create(sizeof(async_callback_t), async_ce_callback);

	ZVAL_PTR(&object->callback, callback);
	object->owner = owner;

	return object;
}

/**
 * This method is used to bind the Callback and Resume object.
 *
 * Such a binding can be useful for debugging to know which Callbacks affect which events.
 * This code is available only within the PHP Core and is not accessible from userland.
 */
zend_result async_callback_bind_resume(zend_object* callback, const zval* resume)
{
	zval* resume_current = async_callback_get_resume(callback);

	if (UNEXPECTED(resume_current != NULL && Z_TYPE_P(resume_current) == IS_NULL)) {
		async_warning("Attempt to bind the resume object and Callback twice.");
		return FAILURE;
	}

	zval weak_reference;
	zend_new_weak_reference_from(resume, &weak_reference);

	if (UNEXPECTED(Z_TYPE(weak_reference) == IS_UNDEF)) {
		return FAILURE;
	}

	zval_property_move(resume_current, &weak_reference);

	return SUCCESS;
}

/**
 * The method links the notifier and the callback together.
 * The method is always called from the notifier when someone attempts to add a callback to the notifier.
 */
void async_callback_registered(zend_object* callback, zend_object* notifier)
{
    zval* notifiers = async_callback_get_zval_notifiers(callback);

	if (zend_hash_index_find(Z_ARRVAL_P(notifiers), notifier->handle) != NULL) {
        return;
    }

	zval zval_notifier;
	ZVAL_OBJ(&zval_notifier, notifier);
	zend_property_array_index_update(notifiers, notifier->handle, &zval_notifier, false);
}

/**
 * This method is used to get the Resume object from the Callback object.
 *
 * The method returns a created ZVAL and transfers ownership.
 */
zend_object * async_callback_resolve_resume(const zend_object* callback)
{
	zval* resume_ref = async_callback_get_resume((zend_object*) callback);

	if (resume_ref == NULL || Z_TYPE_P(resume_ref) == IS_NULL) {
        return NULL;
    }

	zval retval;
	ZVAL_UNDEF(&retval);

	zend_resolve_weak_reference(resume_ref, &retval);

	if (Z_TYPE(retval) == IS_NULL) {
        return NULL;
    }

	return Z_OBJ(retval);
}

/**
 * The method is used to notify the callback about the event.
 *
 * The method calls the 'callable' with the notifier, event, and error.
 */
void async_callback_notify(zend_object* callback, zend_object* notifier, zval* event, zval* error)
{
	zval * defer_callback = NULL;

	if (EXPECTED(HT_IS_INITIALIZED(&ASYNC_G(defer_callbacks)))) {
		defer_callback = zend_hash_index_find(&ASYNC_G(defer_callbacks), callback->handle);
	}

	zval * property_callback = async_callback_get_callback(callback);

	if (Z_TYPE_P(property_callback) == IS_NULL) {
		if (defer_callback != NULL) {
			zend_hash_index_del(&ASYNC_G(defer_callbacks), callback->handle);
		}

        return;
    }

	// Support pure pointer callbacks.
	if (Z_TYPE_P(property_callback) == IS_PTR) {
    	const async_callback_function_t cb_function = Z_PTR_P(property_callback);
    	cb_function(callback, notifier, event, error);

		if (defer_callback != NULL) {
			zend_hash_index_del(&ASYNC_G(defer_callbacks), callback->handle);
		}

    	return;
    }

	if (false == zend_is_callable(property_callback, 0, NULL)) {
		// Notify the user about the error.
		zend_string *callable_name = zend_get_callable_name(property_callback);
		php_error_docref(NULL, E_WARNING, "The property callback is not callable: %s", ZSTR_VAL(callable_name));
		zend_string_release(callable_name);
		zval_ptr_dtor(property_callback);
		ZVAL_NULL(property_callback);

		if (defer_callback != NULL) {
			zend_hash_index_del(&ASYNC_G(defer_callbacks), callback->handle);
		}

		return;
	}

	zval args[3];
	ZVAL_OBJ(&args[0], notifier);

	if (event) {
		ZVAL_COPY_VALUE(&args[1], event);
	} else {
		ZVAL_NULL(&args[1]);
	}

	if (error) {
		ZVAL_COPY_VALUE(&args[2], error);
	} else {
		ZVAL_NULL(&args[2]);
	}

	zval retval;
	ZVAL_UNDEF(&retval);

	// Call the callable
	if (call_user_function(EG(function_table), NULL, property_callback, &retval, 3, args) == FAILURE) {
		// Notify the user about the error.
		zend_string *callable_name = zend_get_callable_name(property_callback);
		php_error_docref(NULL, E_WARNING, "Failed to call the callable %s", ZSTR_VAL(callable_name));
		zend_string_release(callable_name);
	}

	// Cleanup return value
	zval_ptr_dtor(&retval);

	//
	// If an exception occurred during the callback execution, we need to transfer it to the Fiber.
	// (if callback is executed within the Fiber)
	//
	const zval * fiber = async_callback_get_fiber(callback);

	if (UNEXPECTED(EG(exception) != NULL && Z_TYPE_P(fiber) == IS_OBJECT)) {
		async_transfer_throw_to_fiber((zend_fiber *) Z_OBJ_P(async_callback_get_fiber(callback)), EG(exception));
		zend_clear_exception();
	}

	if (defer_callback != NULL) {
		zend_hash_index_del(&ASYNC_G(defer_callbacks), callback->handle);
	}
}