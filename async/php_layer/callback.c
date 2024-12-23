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

#define METHOD(name) PHP_METHOD(Async_Callback, name)

#define PROPERTY_CALLBACK "callback"
#define GET_PROPERTY_CALLBACK() zend_read_property(async_ce_callback, Z_OBJ_P(ZEND_THIS), PROPERTY_CALLBACK, \
		strlen(PROPERTY_CALLBACK), 0, NULL);

#define PROPERTY_NOTIFIERS "notifiers"
#define GET_PROPERTY_NOTIFIERS() zend_read_property(async_ce_callback, Z_OBJ_P(ZEND_THIS), PROPERTY_NOTIFIERS, \
		strlen(PROPERTY_NOTIFIERS), 0, NULL);

#define PROPERTY_RESUME "resume"
#define GET_PROPERTY_RESUME() zend_read_property(async_ce_callback, Z_OBJ_P(ZEND_THIS), PROPERTY_RESUME, \
		strlen(PROPERTY_RESUME), 0, NULL);

static zend_object_handlers async_callback_handlers;

/**
 * This method is used to get the Notifiers array from the Callback object.
 *
 * The method returns a pointer to the HashTable.
 */
static zend_always_inline HashTable* async_callback_get_notifiers(zend_object* callback)
{
	return Z_ARRVAL_P(zend_read_property(
		async_ce_callback, callback, PROPERTY_NOTIFIERS, strlen(PROPERTY_NOTIFIERS), 0, NULL
	));
}

METHOD(__construct)
{
	zval* callable;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable)
	ZEND_PARSE_PARAMETERS_END();

	if (!zend_is_callable(callable, 0, NULL)) {
		zend_throw_exception_ex(zend_ce_type_error, 0, "Expected parameter to be a valid callable");
		RETURN_THROWS();
	}

	zend_update_property(async_ce_callback, Z_OBJ_P(ZEND_THIS), PROPERTY_CALLBACK, strlen(PROPERTY_CALLBACK), callable);
}

METHOD(disposeCallback)
{
	ZEND_PARSE_PARAMETERS_NONE();

	zval* notifiers = zend_read_property(
		async_ce_callback, Z_OBJ_P(ZEND_THIS), PROPERTY_NOTIFIERS, strlen(PROPERTY_NOTIFIERS), 0, NULL
	);

	zval *current;

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(notifiers), current)
		if (Z_TYPE_P(current) == IS_OBJECT) {
			async_notifier_remove_callback(Z_OBJ_P(current), ZEND_THIS);
		}
	ZEND_HASH_FOREACH_END();

	zval new_notifiers;
	array_init(&new_notifiers);

	zend_update_property(
		async_ce_callback, Z_OBJ_P(ZEND_THIS), PROPERTY_NOTIFIERS, strlen(PROPERTY_NOTIFIERS), &new_notifiers
    );

	zval_ptr_dtor(&new_notifiers);
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

	zend_object_std_dtor(object);
}

void async_register_notifier_ce(void)
{
	async_ce_callback = register_class_Async_Callback();
	async_ce_callback->default_object_handlers = &async_callback_handlers;

	async_callback_handlers = std_object_handlers;
	async_callback_handlers.dtor_obj = async_callback_object_destroy;
	async_callback_handlers.clone_obj = NULL;
}

/**
 * This method is used to bind the Callback and Resume object.
 *
 * Such a binding can be useful for debugging to know which Callbacks affect which events.
 * This code is available only within the PHP Core and is not accessible from userland.
 */
zend_result async_callback_bind_resume(zend_object* callback, const zval* resume)
{
	const zval* resume_current = zend_read_property(
		async_ce_callback, callback, PROPERTY_RESUME,strlen(PROPERTY_RESUME), 0, NULL
	);

	if (UNEXPECTED(resume_current != NULL && Z_TYPE_P(resume_current) == IS_NULL)) {
		zend_error(E_WARNING, "Attempt to bind the resume object and Callback twice.");
		return FAILURE;
	}

	zval * resume_ref = async_new_weak_reference_from(resume);

	zend_update_property(
		async_ce_callback,
		callback,
		PROPERTY_RESUME,
		strlen(PROPERTY_RESUME),
		resume_ref
	);

	zval_ptr_dtor(resume_ref);

	return SUCCESS;
}

/**
 * The method links the notifier and the callback together.
 * The method is always called from the notifier when someone attempts to add a callback to the notifier.
 */
void async_callback_registered(zend_object* callback, const zval* notifier)
{
    zval* notifiers = zend_read_property(
        async_ce_callback, callback, PROPERTY_NOTIFIERS, strlen(PROPERTY_NOTIFIERS), 0, NULL
    );

    zval *current;

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(notifiers), current)
		if (Z_TYPE_P(current) == IS_OBJECT && Z_OBJ_P(current) == Z_OBJ_P(notifier)) {
			return;
		}
	ZEND_HASH_FOREACH_END();

	add_next_index_zval(notifiers, (zval*) notifier);
	Z_TRY_ADDREF_P(notifier);

	add_next_index_zval(notifiers, async_new_weak_reference_from(notifier));
}

/**
 * This method is used to get the Resume object from the Callback object.
 *
 * The method returns a created ZVAL and transfers ownership.
 */
zval* async_callback_get_resume(const zend_object* callback)
{
	zval* resume_ref = zend_read_property(
		async_ce_callback, (zend_object*) callback, PROPERTY_RESUME, strlen(PROPERTY_RESUME), 0, NULL
	);

	if (resume_ref == NULL || Z_TYPE_P(resume_ref) == IS_NULL) {
        return NULL;
    }

	zval* retval = emalloc(sizeof(zval));
	ZVAL_UNDEF(retval);

	async_resolve_weak_reference(resume_ref, retval);

	if (Z_TYPE_P(retval) == IS_NULL) {
        zval_ptr_dtor(retval);
        efree(retval);
        return NULL;
    }

	return retval;
}

/**
 * The method is used to notify the callback about the event.
 *
 * The method calls the 'callable' with the notifier, event, and error.
 */
void async_callback_notify(zend_object* callback, zend_object* notifier, const zval* event, const zval* error)
{
	const zval* callable = zend_read_property(
		async_ce_callback, callback, PROPERTY_CALLBACK, strlen(PROPERTY_CALLBACK), 0, NULL
	);

	if (Z_TYPE_P(callable) == IS_NULL) {
        return;
    }

	if (!zend_is_callable((zval*) callable, 0, NULL)) {
		zend_update_property_null(async_ce_callback, callback, PROPERTY_CALLBACK, strlen(PROPERTY_CALLBACK));

		// Notify the user about the error.
		zend_string *callable_name = zend_get_callable_name((zval *) callable);
		php_error_docref(NULL, E_WARNING, "The callable is not callable: %s", ZSTR_VAL(callable_name));
		zend_string_release(callable_name);
		return;
	}

	zval args[3];
	ZVAL_OBJ(&args[0], notifier);

	if (event) {
		ZVAL_COPY(&args[1], event);
	} else {
		ZVAL_NULL(&args[1]);
	}

	if (error) {
		ZVAL_COPY(&args[2], error);
	} else {
		ZVAL_NULL(&args[2]);
	}

	zval retval;
	ZVAL_UNDEF(&retval);

	// Call the callable
	if (call_user_function(EG(function_table), NULL, (zval*) callable, &retval, 3, args) == FAILURE) {
		// Notify the user about the error.
		zend_string *callable_name = zend_get_callable_name((zval *) callable);
		php_error_docref(NULL, E_WARNING, "Failed to call the callable %s", ZSTR_VAL(callable_name));
		zend_string_release(callable_name);
	}

	// Cleanup arguments
	zval_ptr_dtor(&args[0]);
	zval_ptr_dtor(&args[1]);
	zval_ptr_dtor(&args[2]);

	// Cleanup return value
	zval_ptr_dtor(&retval);
}