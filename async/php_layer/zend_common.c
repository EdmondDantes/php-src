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

zval* async_new_weak_reference_from(zval* referent)
{
	static zend_class_entry* weak_ref_ce = NULL;
	static zend_function* create_fn;
	static int initialized = 0;

	if (!initialized) {

		weak_ref_ce = zend_lookup_class(ZEND_STRL("WeakReference"));

		if (UNEXPECTED(weak_ref_ce == NULL)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't find class WeakReference");
		}

		create_fn = zend_hash_str_find_ptr_lc(&weak_ref_ce->function_table, ZEND_STRL("create"));

		if (UNEXPECTED(create_fn == NULL)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method WeakReference::create");
		}

		initialized = 1;
	}

	zval* retval = emalloc(sizeof(zval));
	ZVAL_UNDEF(retval);

	if (UNEXPECTED(zend_call_known_function(create_fn, NULL, weak_ref_ce, retval, 1, referent, NULL) != SUCCESS)) {
		zend_error(E_WARNING, "Failed to invoke WeakReference::create");
		efree(retval);
		return NULL;
	}

	return retval;
}
