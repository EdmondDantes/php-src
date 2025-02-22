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
#include "context.h"

#include <IPHlpApi.h>
#include <zend_fibers.h>
#include <zend_weakrefs.h>
#include <async/php_async.h>

#include "context_arginfo.h"
#include "exceptions.h"

#define METHOD(name) PHP_METHOD(Async_Context, name)
#define THIS_CONTEXT ((async_context_t *)(Z_OBJ_P(ZEND_THIS)))
#define THIS_KEY ((async_key_t *)(Z_OBJ_P(ZEND_THIS)))
#define RECURSION_LIMIT 127

METHOD(current)
{
	bool auto_create = false;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(auto_create)
	ZEND_PARSE_PARAMETERS_END();

	async_context_t *context = async_context_current(auto_create, true);

	if (context == NULL) {
		RETURN_NULL();
	}

	RETURN_OBJ(&context->std);
}

METHOD(newCurrent)
{
	ZEND_PARSE_PARAMETERS_NONE();

	async_context_t *context = async_context_current_new(false, false);

	if (context == NULL) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&context->std);
}

METHOD(overrideCurrent)
{
	bool is_weak_ref = false;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(is_weak_ref)
	ZEND_PARSE_PARAMETERS_END();

	async_context_t *context = async_context_current_new(true, is_weak_ref);

	if (context == NULL) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&context->std);
}

METHOD(__construct)
{
	zval *parent = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_OBJ_OR_NULL(parent)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(THIS_CONTEXT->is_weak_ref)
	ZEND_PARSE_PARAMETERS_END();

	if (Z_TYPE(parent) != IS_NULL) {

		if (THIS_CONTEXT->is_weak_ref) {
			zval retval;
			zend_new_weak_reference_from(parent, &retval);

			if (Z_TYPE(retval) != IS_OBJECT) {
				async_throw_error("Failed to create weak reference to parent context");
				RETURN_THROWS();
			}

			THIS_CONTEXT->parent_weak_ref = Z_OBJ(retval);
		} else {
			THIS_CONTEXT->parent = (async_context_t *) Z_OBJ_P(parent);
			GC_ADDREF(&THIS_CONTEXT->parent->std);
		}
	}
}

zend_always_inline void try_to_resolve_weak_reference(
	async_context_t * context, zend_object *object_key, zend_string *string_key, zval * result, zval * return_value
)
{
	if (result != NULL) {

		// Try to resolve WeakReference.
		if (Z_TYPE_P(result) == IS_OBJECT && Z_OBJ_P(result)->ce == zend_ce_weakref) {
			zval retval;
			zend_resolve_weak_reference(result, &retval);

			if (Z_TYPE(retval) == IS_OBJECT) {
				RETURN_ZVAL(&retval, 1, 0);
			} else {
				// Automatically remove the key if it is a WeakReference.
				zval tmp;

				if (object_key != NULL) {
					ZVAL_OBJ(&tmp, object_key);
				} else {
					ZVAL_STR(&tmp, string_key);
				}

				async_context_del_key(context, &tmp);
				RETURN_NULL();
			}
		}

		RETURN_ZVAL(result, 1, 0);
	}

	RETURN_NULL();
}

METHOD(find)
{
	zend_object *object_key = NULL;
	zend_string *string_key = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OR_STR(object_key, string_key)
	ZEND_PARSE_PARAMETERS_END();

	if (object_key != NULL) {
		zval *result = async_context_find_by_key(THIS_CONTEXT, object_key, false, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, object_key, NULL, result, return_value);
		return;
	} else if (string_key != NULL) {
		zval *result = async_context_find_by_str(THIS_CONTEXT, string_key, false, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, NULL, string_key, result, return_value);
		return;
	}

	RETURN_NULL();
}

METHOD(get)
{
	zend_object *object_key = NULL;
	zend_string *string_key = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OR_STR(object_key, string_key)
	ZEND_PARSE_PARAMETERS_END();

	if (object_key != NULL) {
		zval *result = async_context_find_by_key(THIS_CONTEXT, object_key, false, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, object_key, NULL, result, return_value);
	} else if (string_key != NULL) {
		zval *result = async_context_find_by_str(THIS_CONTEXT, string_key, false, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, NULL, string_key, result, return_value);
	}

	if (Z_TYPE(return_value) == IS_NULL) {
		if (object_key != NULL) {
			async_throw_error("Key object class '%s' is nullable in context", object_key->ce->name->val);
		} else {
			async_throw_error("String key '%s' is nullable in context", string_key->val);
		}
	}
}

METHOD(has)
{
	zend_object *object_key = NULL;
	zend_string *string_key = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OR_STR(object_key, string_key)
	ZEND_PARSE_PARAMETERS_END();

	if (object_key != NULL) {
		zval *result = async_context_find_by_key(THIS_CONTEXT, object_key, false, 0);

		RETURN_BOOL(result != NULL);
	} else if (string_key != NULL) {
		zval *result = async_context_find_by_str(THIS_CONTEXT, string_key, false, 0);

		RETURN_BOOL(result != NULL);
	}

	RETURN_FALSE;
}

METHOD(findLocal)
{
	zend_object *object_key = NULL;
	zend_string *string_key = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OR_STR(object_key, string_key)
	ZEND_PARSE_PARAMETERS_END();

	if (object_key != NULL) {
		zval *result = async_context_find_by_key(THIS_CONTEXT, object_key, true, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, object_key, NULL, result, return_value);
	} else if (string_key != NULL) {
		zval *result = async_context_find_by_str(THIS_CONTEXT, string_key, true, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, NULL, string_key, result, return_value);
	}

	RETURN_NULL();
}

METHOD(getLocal)
{
	zend_object *object_key = NULL;
	zend_string *string_key = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OR_STR(object_key, string_key)
	ZEND_PARSE_PARAMETERS_END();

	if (object_key != NULL) {
		zval *result = async_context_find_by_key(THIS_CONTEXT, object_key, true, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, object_key, NULL, result, return_value);
	} else if (string_key != NULL) {
		zval *result = async_context_find_by_str(THIS_CONTEXT, string_key, true, 0);
		try_to_resolve_weak_reference(THIS_CONTEXT, NULL, string_key, result, return_value);
	}

	if (Z_TYPE(return_value) == IS_NULL) {
		if (object_key != NULL) {
			async_throw_error("Key object class '%s' is nullable in context", object_key->ce->name->val);
		} else {
			async_throw_error("String key '%s' is nullable in context", string_key->val);
		}
	}
}

METHOD(hasLocal)
{
	zend_object *object_key = NULL;
	zend_string *string_key = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OR_STR(object_key, string_key)
	ZEND_PARSE_PARAMETERS_END();

	if (object_key != NULL) {
		zval *result = async_context_find_by_key(THIS_CONTEXT, object_key, true, 0);

		RETURN_BOOL(result != NULL);
	} else if (string_key != NULL) {
		zval *result = async_context_find_by_str(THIS_CONTEXT, string_key, true, 0);

		RETURN_BOOL(result != NULL);
	}

	RETURN_FALSE;
}

METHOD(setKey)
{
	zend_object *object_key = NULL;
	zend_string *string_key = NULL;
	bool replace = 0;
	zval *value;

	ZEND_PARSE_PARAMETERS_START(2, 3)
		Z_PARAM_OBJ_OR_STR(object_key, string_key)
		Z_PARAM_ZVAL(value)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(replace)
	ZEND_PARSE_PARAMETERS_END();

	if (object_key != NULL) {
		async_context_set_key(THIS_CONTEXT, object_key, value, replace);
	} else if (string_key != NULL) {
		async_context_set_str(THIS_CONTEXT, string_key, value, replace);
	}

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&THIS_CONTEXT->std);
}

METHOD(delKey)
{
	zval *key;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(key)
	ZEND_PARSE_PARAMETERS_END();

	if (Z_TYPE_P(key) != IS_STRING && Z_TYPE_P(key) != IS_OBJECT) {
		zend_wrong_parameter_type_error(1, Z_EXPECTED_OBJECT_OR_STRING, key);
		RETURN_THROWS();
	}

	async_context_del_key(THIS_CONTEXT, key);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	RETURN_OBJ(&THIS_CONTEXT->std);
}

METHOD(getParent)
{
	if (THIS_CONTEXT->is_weak_ref) {
		zval weak_ref;
		ZVAL_OBJ(&weak_ref, &THIS_CONTEXT->parent_weak_ref);
		zend_resolve_weak_reference(&weak_ref, return_value);
	} else if (THIS_CONTEXT->parent) {
		RETURN_OBJ(&THIS_CONTEXT->parent->std);
	}

	RETURN_NULL();
}

METHOD(isEmpty)
{
	RETURN_BOOL(zend_hash_num_elements(THIS_CONTEXT->map) == 0);
}

PHP_METHOD(Async_Key, __construct)
{
	zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

	ZVAL_STR(&THIS_KEY->description, zend_string_dup(name, 0));
}

PHP_METHOD(Async_Key, __toString)
{
    RETURN_STR(zend_string_copy(&THIS_KEY->description));
}

static zend_object_handlers async_context_handlers;


static void async_context_object_destroy(zend_object *object)
{
	async_context_t *context = (async_context_t *) object;

	zend_array_release(context->map);
	zend_array_release(context->objects);

	if (context->is_weak_ref) {
		OBJ_RELEASE(context->parent_weak_ref);
	} else if (context->parent != NULL) {
		OBJ_RELEASE(&context->parent->std);
		context->parent = NULL;
	}
}

void async_register_context_ce(void)
{
	async_ce_context = register_class_Async_Context();

	async_ce_context->default_object_handlers = &async_context_handlers;

	async_context_handlers = std_object_handlers;
	async_context_handlers.dtor_obj = async_context_object_destroy;
	async_context_handlers.clone_obj = async_context_clone;

	async_ce_key = register_class_Async_Key();

	async_ce_context_exception = register_class_Async_ContextException(zend_ce_exception);
}

async_context_t * async_context_new(async_context_t * parent, const bool is_weak_ref)
{
	zend_object * weak_object = NULL;

	if (parent != NULL && is_weak_ref) {
		zval z_parent, retval;
		ZVAL_OBJ(&z_parent, &parent->std);

		zend_new_weak_reference_from(&z_parent, &retval);

		if (Z_TYPE(retval) != IS_OBJECT) {
			async_warning("Failed to create weak reference to parent context");
			return NULL;
		}

		weak_object = Z_OBJ(retval);
	}

	DEFINE_ZEND_INTERNAL_OBJECT(async_context_t, context, async_ce_context);

	context->map = zend_new_array(8);
	context->objects = zend_new_array(8);

	if (weak_object) {
		context->is_weak_ref = true;
		context->parent_weak_ref = weak_object;
	} else if (parent) {
		context->parent = parent;
		GC_ADDREF(&parent->std);
	}

	return context;
}

zval * async_context_find_by_key(async_context_t * context, zend_object * key, bool local, int recursion)
{
	zval *result = NULL;

	result = zend_hash_index_find(context->map, key->handle);

	if (local) {
		return result;
	}

	if (recursion >= RECURSION_LIMIT) {
		async_throw_error("Recursion limit reached: 127");
		return NULL;
	}

	if (result == NULL && context->is_weak_ref && context->parent_weak_ref) {

		zval weak_ref, reference;
		ZVAL_OBJ(&weak_ref, &context->parent_weak_ref);
		zend_resolve_weak_reference(&weak_ref, &reference);

		if (Z_TYPE(reference) == IS_OBJECT) {
			result = async_context_find_by_key((async_context_t *) Z_OBJ(reference), key, local, ++recursion);
			zval_ptr_dtor(&reference);
			return result;
		} else {
			return NULL;
		}

	} else if (result == NULL && context->parent != NULL) {
		return async_context_find_by_key(context->parent, key, local, ++recursion);
	} else {
		return result;
	}
}

zval * async_context_find_by_str(async_context_t * context, zend_string * key, bool local, int recursion)
{
	zval *result = NULL;

	result = zend_hash_find(context->map, key);

	if (local) {
		return result;
	}

	if (recursion >= RECURSION_LIMIT) {
		async_throw_error("Recursion limit reached: 127");
		return NULL;
	}

	if (result == NULL && context->is_weak_ref && context->parent_weak_ref) {

		zval weak_ref, reference;
		ZVAL_OBJ(&weak_ref, &context->parent_weak_ref);
		zend_resolve_weak_reference(&weak_ref, &reference);

		if (Z_TYPE(reference) == IS_OBJECT) {
			result = async_context_find_by_str((async_context_t *) Z_OBJ(reference), key, local, ++recursion);
			zval_ptr_dtor(&reference);
			return result;
		} else {
			return NULL;
		}

	} else if (result == NULL && context->parent != NULL) {
		return async_context_find_by_str(context->parent, key, local, ++recursion);
	} else {
		return result;
	}
}

void async_context_set_key(async_context_t * context, zend_object * key, zval * value, bool replace)
{
	if (context == NULL) {
		return;
	}

	if (false == replace && zend_hash_index_find(context->map, key->handle) != NULL) {

		zval key_as_string;

		if (zend_std_cast_object_tostring(key, &key_as_string, IS_STRING) == FAILURE) {
			zend_throw_error(
				async_ce_context_exception,
				"Attempt to overwrite a key of class '%s' that already exists in the context.",
				key->ce->name->val
			);
		} else {
			zend_throw_error(
				async_ce_context_exception,
				"Attempt to overwrite a key '%s' of class '%s' that already exists in the context.",
				Z_STRVAL(key_as_string),
				key->ce->name->val
			);
		}

		return;
	}

	if (UNEXPECTED(zend_hash_index_update(context->map, key->handle, value) == NULL)) {
		async_throw_error("Failed to update context map");
		return;
	}

	Z_TRY_ADDREF_P(value);

	zval z_key;
	ZVAL_OBJ(&z_key, key);

	if (UNEXPECTED(zend_hash_index_add(context->objects, key->handle, &z_key) == NULL)) {
		async_throw_error("Failed to update context objects");
		return;
	}

	GC_ADDREF(key);
}

void async_context_set_str(async_context_t * context, zend_string * key, zval * value, bool replace)
{
	if (false == replace && zend_hash_find(context->map, key) != NULL) {
		zend_throw_error(async_ce_context_exception,
			"Attempt to overwrite a string key '%s' that already exists in the context.",
			key->val
		);
		return;
	}

	if (UNEXPECTED(zend_hash_update(context->map, key, value) == NULL)) {
		async_throw_error("Failed to update context map");
		return;
	}

	Z_TRY_ADDREF_P(value);
}

zend_object* async_context_clone(zend_object * object)
{
	async_context_t *src_context = (async_context_t *) object;

	DEFINE_ZEND_INTERNAL_OBJECT(async_context_t, context, async_ce_context);

	context->map = zend_array_dup(src_context->map);
	context->objects = zend_array_dup(src_context->objects);

	if (src_context->is_weak_ref) {
		zval retval, weak_ref, reference;
		ZVAL_OBJ(&weak_ref, &src_context->parent_weak_ref);
		zend_resolve_weak_reference(&weak_ref, &reference);

		if (Z_TYPE(reference) == IS_OBJECT) {

			zend_new_weak_reference_from(&reference, &retval);
			zval_ptr_dtor(&reference);

			if (Z_TYPE(retval) != IS_OBJECT) {
				async_warning("Failed to create weak reference to parent context");
				return NULL;
			}

			context->parent_weak_ref = Z_OBJ(retval);
			context->is_weak_ref = true;
		}
	} else if (src_context->parent != NULL) {
		context->parent = src_context->parent;
		GC_ADDREF(&src_context->parent->std);
	}

	return &context->std;
}

void async_context_del_key(const async_context_t * context, zval * key)
{
	if (UNEXPECTED(context == NULL)) {
		return;
	}

	if (Z_TYPE_P(key) == IS_STRING) {
		zend_hash_del(context->map, Z_STR_P(key));
	} else {
		zend_object *object = Z_OBJ_P(key);
		zend_hash_index_del(context->map, object->handle);
		zend_hash_index_del(context->objects, object->handle);
	}
}

async_context_t * async_context_current(const bool auto_create, const bool add_ref)
{
	if (EG(active_fiber)) {

		if (EG(active_fiber)->async_context == NULL) {
			EG(active_fiber)->async_context = auto_create ? async_context_new(NULL, false) : NULL;
		}

		if (add_ref && EG(active_fiber)->async_context != NULL) {
			GC_ADDREF(&EG(active_fiber)->async_context->std);
		}

		return EG(active_fiber)->async_context;
	} else {

		if (ASYNC_G(root_context) == NULL) {
			ASYNC_G(root_context) = auto_create ? async_context_new(NULL, false) : NULL;
		}

		if (add_ref && ASYNC_G(root_context) != NULL) {
			GC_ADDREF(&ASYNC_G(root_context)->std);
		}

		return ASYNC_G(root_context);
	}
}

async_context_t * async_context_current_new(bool is_override, const bool is_weak_ref)
{
	async_context_t *parent_context = NULL;

	if (is_override) {
		parent_context = async_context_current(false, false);
	}

	async_context_t * context = async_context_new(parent_context, is_weak_ref);

	if (context == NULL) {
		return NULL;
	}

	if (EG(active_fiber)) {
		EG(active_fiber)->async_context = context;
	} else {
		ASYNC_G(root_context) = context;
	}

	if (parent_context != NULL) {
		OBJ_RELEASE(&parent_context->std);
	}

	return context;
}

async_key_t * async_key_new(zend_string * description)
{
	DEFINE_ZEND_INTERNAL_OBJECT(async_key_t, key, async_ce_key);
    ZVAL_STR_COPY(&key->description, description);
    return key;
}

async_key_t * async_key_new_from_string(char * description, size_t len)
{
	zend_string *str = zend_string_init(description, len, 0);
    async_key_t *key = async_key_new(str);
    zend_string_release(str);
    return key;
}