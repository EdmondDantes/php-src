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

static zend_class_entry* weak_ref_ce = NULL;
static zend_function* create_fn = NULL;
static zend_function* get_fn = NULL;

zend_always_inline zend_class_entry* async_get_weak_reference_ce()
{
	if (UNEXPECTED(weak_ref_ce == NULL)) {
		zend_string *cname = zend_string_init(ZEND_STRL("WeakReference"), 0);
		weak_ref_ce = zend_lookup_class(cname);
		zend_string_release_ex(cname, 0);

		if (UNEXPECTED(weak_ref_ce == NULL)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't find class WeakReference");
		}
	}

	return weak_ref_ce;
}

void zend_exception_to_warning(const char * format, const bool clean)
{
	if (EG(exception) == NULL) {
		return;
	}

	zval rv;
	const zval *message = zend_read_property_ex(
		EG(exception)->ce, EG(exception), zend_known_strings[ZEND_STR_MESSAGE], 0, &rv
	);

	if (message == NULL) {
		async_warning(format, "No message");
	} else {
		async_warning(format, Z_STRVAL_P(message));
	}

	if (clean) {
		zend_clear_exception();
	}
}

zend_string * zend_current_exception_get_message(const bool clean)
{
	if (EG(exception) == NULL) {
		return NULL;
	}

	zval rv;
	const zval *message = zend_read_property_ex(
		EG(exception)->ce, EG(exception), zend_known_strings[ZEND_STR_MESSAGE], 0, &rv
	);

	if (clean) {
		zend_clear_exception();
	}

	if (message != NULL && Z_TYPE_P(message) == IS_STRING) {
		return Z_STR_P(message);
	} else {
		return NULL;
	}
}

void zend_new_weak_reference_from(const zval* referent, zval * retval)
{
	if (!create_fn) {

		async_get_weak_reference_ce();
		create_fn = zend_hash_str_find_ptr_lc(&weak_ref_ce->function_table, ZEND_STRL("create"));

		if (UNEXPECTED(create_fn == NULL)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method WeakReference::create");
		}
	}

	ZVAL_UNDEF(retval);

	zend_call_known_function(create_fn, NULL, weak_ref_ce, retval, 1, (zval*) referent, NULL);

	if (UNEXPECTED(Z_TYPE_P(retval) == IS_NULL || Z_ISUNDEF_P(retval))) {
		async_warning("Failed to invoke WeakReference::create");
	}
}

/**
 * The method is used to resolve the weak reference.
 *
 * The method returns the resolved object.
 *
 * @warning The method may return a ZVAL with the type NULL!
 * @warning You must call the dtor if the result is no longer needed!
 */
void zend_resolve_weak_reference(zval* weak_reference, zval* retval)
{
	if (!get_fn) {

		async_get_weak_reference_ce();
		get_fn = zend_hash_str_find_ptr_lc(&weak_ref_ce->function_table, ZEND_STRL("get"));

		if (UNEXPECTED(get_fn == NULL)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method WeakReference::get");
		}
	}

	zend_call_known_function(get_fn, Z_OBJ_P(weak_reference), weak_ref_ce, retval, 0, NULL, NULL);
}

zif_handler zend_hook_php_function(const char *name, const size_t len, zif_handler new_function)
{
	zend_function *original = zend_hash_str_find_ptr(CG(function_table), name, len);

	if (original == NULL) {
		return NULL;
	}

	zif_handler original_handler = original->internal_function.handler;
	original->internal_function.handler = new_function;

	return original_handler;
}

zif_handler zend_replace_method(zend_object * object, const char * method, const size_t len, const zif_handler handler)
{
	zif_handler original_handler = NULL;
	zend_function *func = zend_hash_str_find_ptr(&object->ce->function_table, method, len);

	if (func == NULL) {
		return original_handler;
	}

	original_handler = func->internal_function.handler;
	func->internal_function.handler = handler;

	return original_handler;
}

void zend_get_function_name_by_fci(zend_fcall_info * fci, zend_fcall_info_cache *fci_cache, zend_string **name)
{
	if (fci_cache != NULL && fci_cache->function_handler != NULL) {
        *name = fci_cache->function_handler->common.function_name;
    } else if (fci != NULL && Z_TYPE(fci->function_name) != IS_UNDEF) {
        *name = Z_STR(fci->function_name);
    } else {
        *name = NULL;
    }
}
