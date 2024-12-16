
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
