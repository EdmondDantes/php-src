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
#include "callback.h"
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

static void async_callback_object_destroy(zend_object* object)
{
	zend_fiber* fiber = (zend_fiber*)object;
}

void async_register_notifier_ce(void)
{
	async_ce_callback = register_class_Async_Callback();
	async_ce_callback->default_object_handlers = &async_callback_handlers;

	async_callback_handlers = std_object_handlers;
	async_callback_handlers.dtor_obj = async_callback_object_destroy;
	async_callback_handlers.clone_obj = NULL;
}

zend_result async_callback_register(zend_object* callback, zval* notifier)
{
	const zval * notifiers = zend_read_property(
		async_ce_callback, callback, PROPERTY_NOTIFIERS, strlen(PROPERTY_NOTIFIERS), 0, NULL
	);

	zval *current;

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(notifiers), current)
		if (Z_TYPE_P(current) == IS_OBJECT && Z_OBJ_P(current) == Z_OBJ_P(notifier)) {
			return FAILURE;
		}
	ZEND_HASH_FOREACH_END();

	add_next_index_zval(callbacks, notifier);
	Z_TRY_ADDREF_P(notifier);

	return SUCCESS;
}

zend_result async_callback_notify(zend_object* callback, zend_object* notifier, zval* event, zval* error)
{

}

/**
 * This method is used to bind the Callback and Resume object.
 *
 * Such a binding can be useful for debugging to know which Callbacks affect which events.
 * This code is available only within the PHP Core and is not accessible from userland.
 */
zend_result async_callback_bind_resume(zend_object* callback, zval* resume)
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
 * This method is used to get the Notifiers array from the Callback object.
 *
 * The method returns a pointer to the HashTable.
 */
zend_always_inline HashTable* async_callback_get_notifiers(zend_object* callback)
{
	return Z_ARRVAL_P(zend_read_property(
        async_ce_callback, callback, PROPERTY_NOTIFIERS, strlen(PROPERTY_NOTIFIERS), 0, NULL
    ));
}

/**
 * This method is used to get the Resume object from the Callback object.
 *
 * The method returns a created ZVAL and transfers ownership.
 */
zend_always_inline zval* async_callback_get_resume(zend_object* callback)
{
	zval* resume_ref = zend_read_property(
		async_ce_callback, callback, PROPERTY_RESUME, strlen(PROPERTY_RESUME), 0, NULL
	);

	if (resume_ref == NULL || Z_TYPE_P(resume_ref) == IS_NULL) {
        return NULL;
    }

	zval* retval = emalloc(sizeof(zval));
	ZVAL_UNDEF(retval);

	async_resolve_weak_reference(resume_ref, retval);

	return retval;
}
