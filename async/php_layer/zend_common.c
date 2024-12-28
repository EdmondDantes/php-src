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

zval* async_new_weak_reference_from(const zval* referent)
{
	if (!create_fn) {

		async_get_weak_reference_ce();
		create_fn = zend_hash_str_find_ptr_lc(&weak_ref_ce->function_table, ZEND_STRL("create"));

		if (UNEXPECTED(create_fn == NULL)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method WeakReference::create");
		}
	}

	zval* retval = emalloc(sizeof(zval));
	ZVAL_UNDEF(retval);

	zend_call_known_function(create_fn, NULL, weak_ref_ce, retval, 1, (zval*) referent, NULL);

	if (UNEXPECTED(Z_TYPE_P(retval) == IS_NULL || Z_ISUNDEF_P(retval))) {
		zend_error(E_WARNING, "Failed to invoke WeakReference::create");
		efree(retval);
		return NULL;
	}

	return retval;
}

/**
 * The method is used to resolve the weak reference.
 *
 * The method returns the resolved object.
 *
 * @warning The method may return a ZVAL with the type NULL!
 * @warning You must call the dtor if the result is no longer needed!
 */
void async_resolve_weak_reference(zval* weak_reference, zval* retval)
{
	if (!get_fn) {

		async_get_weak_reference_ce();
		get_fn = zend_hash_str_find_ptr_lc(&weak_ref_ce->function_table, ZEND_STRL("get"));

		if (UNEXPECTED(get_fn == NULL)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method WeakReference::get");
		}
	}

	zend_call_known_function(get_fn, Z_OBJ_P(weak_reference), weak_ref_ce, retval, 1, weak_reference, NULL);
}