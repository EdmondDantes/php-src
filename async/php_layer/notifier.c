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

METHOD(__toString)
{
	reactor_notifier_t * notifier = (reactor_notifier_t *) Z_OBJ_P(ZEND_THIS);

	if (Z_TYPE(notifier->to_string) == IS_PTR) {
		const reactor_notifier_to_string_t to_string = Z_PTR(notifier->to_string);
		RETURN_STR(zend_string_copy(to_string(notifier)));
	} else if (Z_TYPE(notifier->to_string) != IS_UNDEF && zend_is_callable(&notifier->to_string, 0, NULL)) {
		zval retval;
		call_user_function(EG(function_table), NULL, &notifier->to_string, &retval, 0, NULL);
		RETURN_ZVAL(&retval, 1, 1);
	}

	// By default, this method returns the class name.
	RETURN_STR(zend_string_copy(Z_OBJCE_P(ZEND_THIS)->name));
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

static zend_object *async_notifier_object_create(zend_class_entry *class_entry)
{
	// Allocate memory for the object and initialize it with zero bytes.
	DEFINE_ZEND_RAW_OBJECT(reactor_notifier_t, object, class_entry);
	zend_object_std_init(&object->std, class_entry);
	object_properties_init(&object->std, class_entry);
	async_notifier_object_init(object);

	return &object->std;
}

static void async_notifier_object_destroy(zend_object *object)
{
	reactor_notifier_t *notifier = (reactor_notifier_t *) object;

	if (notifier->remove_callback_fn) {
		zval callback;
		ZVAL_NULL(&callback);
		notifier->remove_callback_fn(notifier, &callback);
	}

	notifier->handler_fn = NULL;
	notifier->remove_callback_fn = NULL;
	ZVAL_NULL(&notifier->_padding2);
}

void async_register_notifier_ce(void)
{
	async_ce_notifier = register_class_Async_Notifier();
	async_ce_notifier->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_notifier->create_object = async_notifier_object_create;

	async_ce_notifier->default_object_handlers = &async_notifier_handlers;

	async_notifier_handlers = std_object_handlers;
	async_notifier_handlers.dtor_obj = async_notifier_object_destroy;
	async_notifier_handlers.clone_obj = NULL;
}

reactor_notifier_t * async_notifier_new_ex(
	size_t size,
	reactor_notifier_handler_t handler_fn,
	reactor_remove_callback_t remove_callback_fn,
	reactor_notifier_to_string_t to_string_fn
)
{
	if (size == 0) {
		size = sizeof(reactor_notifier_t);
	}

	ZEND_ASSERT(sizeof(reactor_notifier_t) <= size && "Extra size must be at least the size of the reactor_notifier_t structure");

	reactor_notifier_t * notifier = zend_object_alloc_ex(size, async_ce_notifier);
	zend_object_std_init(&notifier->std, async_ce_notifier);
	object_properties_init(&notifier->std, async_ce_notifier);

	notifier->handler_fn = handler_fn;
	notifier->remove_callback_fn = remove_callback_fn;

	if (to_string_fn) {
		ZVAL_PTR(&notifier->to_string, to_string_fn);
	}

	return notifier;
}

reactor_notifier_t * async_notifier_new_by_class(const size_t size, zend_class_entry *class_entry)
{
	reactor_notifier_t * notifier = zend_object_alloc_ex(size, class_entry);
	zend_object_std_init(&notifier->std, class_entry);
	object_properties_init(&notifier->std, class_entry);

	notifier->handler_fn = NULL;
	notifier->remove_callback_fn = NULL;

	return notifier;
}

void async_notifier_add_callback(zend_object* notifier, zval* callback)
{
	zval* callbacks = async_notifier_get_callbacks(notifier);

	ZEND_ASSERT(Z_TYPE_P(callback) == IS_OBJECT
		&& (Z_OBJ_P(callback)->ce == async_ce_callback || Z_OBJ_P(callback)->ce == async_ce_resume));

	if (zend_hash_index_find(Z_ARRVAL_P(callbacks), Z_OBJ_P(callback)->handle) != NULL) {
		return;
	}

	// Add the weak reference to the callbacks array.
	zval weak_reference;
	zend_new_weak_reference_from(callback, &weak_reference);
	zend_property_array_index_update(callbacks, Z_OBJ_P(callback)->handle, &weak_reference, true);

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
	// If the notifier has a custom remove callback function, then we call it
	// and if it returns false, then we do not remove the callback.
	if (((reactor_handle_t *) notifier)->remove_callback_fn
		&& false == ((reactor_handle_t *) notifier)->remove_callback_fn((reactor_notifier_t *) notifier, callback)) {
		return;
	}

	ZEND_ASSERT(Z_TYPE_P(callback) == IS_OBJECT
		&& (Z_OBJ_P(callback)->ce == async_ce_callback || Z_OBJ_P(callback)->ce == async_ce_resume));

	HashTable * callbacks = Z_ARRVAL_P(async_notifier_get_callbacks(notifier));

	if (EXPECTED(HT_IS_INITIALIZED(callbacks))) {
		zend_hash_index_del(callbacks, Z_OBJ_P(callback)->handle);
	}
}

void async_notifier_notify(reactor_notifier_t * notifier, zval * event, zval * error)
{
	const zval* callbacks = async_notifier_get_callbacks(&notifier->std);

	zend_try
	{
		// We increase the reference count on ourselves
		// to prevent the object from being deleted during the execution of this function.
		GC_ADDREF(&notifier->std);

		if (notifier->handler_fn != NULL) {
			notifier->handler_fn(notifier, event, error);

			IF_THROW_FINALLY;
		}

		zval *current;
		zval resolved_callback;
		ZVAL_UNDEF(&resolved_callback);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(callbacks), current)

			if (EXPECTED(Z_TYPE_P(current) == IS_OBJECT)) {
				zend_resolve_weak_reference(current, &resolved_callback);

				// Resume object and Callback object use different handlers.
				if (Z_TYPE(resolved_callback) == IS_OBJECT && Z_OBJ_P(&resolved_callback)->ce == async_ce_resume) {
					async_resume_notify((async_resume_t *) Z_OBJ(resolved_callback), notifier, event, error);
				} else if (Z_TYPE(resolved_callback) == IS_OBJECT) {
					async_callback_notify(Z_OBJ(resolved_callback), &notifier->std, event, error);
				}

				zval_ptr_dtor(&resolved_callback);

				IF_THROW_FINALLY;
			}

		IF_THROW_FINALLY;
		ZEND_HASH_FOREACH_END();

	} zend_catch {
		OBJ_RELEASE(&notifier->std);
		zend_bailout();
	} zend_end_try();

finally:

	OBJ_RELEASE(&notifier->std);
}
