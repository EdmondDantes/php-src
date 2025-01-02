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

#ifndef ASYNC_ZEND_COMMON_H
#define ASYNC_ZEND_COMMON_H

#include "php.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"

#define IF_THROW_RETURN_VOID if(EG(exception) != NULL) { return; }
#define IF_THROW_RETURN(value) if(EG(exception) != NULL) { return value; }

void zend_always_inline zval_move(zval * destination, const zval * source)
{
	if (Z_ISREF_P(source)) {
		source = Z_REFVAL_P(source);
	}

	zval_ptr_dtor(destination);
	ZVAL_COPY_VALUE(destination, source);
}

void zend_always_inline zval_copy(zval * destination, zval * source)
{
	if (Z_ISREF_P(source)) {
		source = Z_REFVAL_P(source);
	}

	zval_ptr_dtor(destination);
	ZVAL_COPY_VALUE(destination, source);
	Z_TRY_ADDREF_P(source);
}

void zend_always_inline zval_null(zval * destination)
{
	zval_ptr_dtor(destination);
	ZVAL_NULL(destination);
}

void zend_always_inline zval_property_move(zval * property, const zval * value)
{
	if (EXPECTED(Z_TYPE_P(property) != IS_UNDEF)) {
		zval_ptr_dtor(property);
	} else {
		Z_PROP_FLAG_P(property) &= ~(IS_PROP_UNINIT|IS_PROP_REINITABLE);
	}

	if (Z_ISREF_P(value)) {
		value = Z_REFVAL_P(value);
	}

	ZVAL_COPY_VALUE(property, value);
}

void zend_always_inline zval_property_copy(zval * property, zval * value)
{
	if (EXPECTED(Z_TYPE_P(property) != IS_UNDEF)) {
		zval_ptr_dtor(property);
	} else {
		Z_PROP_FLAG_P(property) &= ~(IS_PROP_UNINIT|IS_PROP_REINITABLE);
	}

	if (Z_ISREF_P(value)) {
		value = Z_REFVAL_P(value);
	}

	ZVAL_COPY_VALUE(property, value);
	Z_TRY_ADDREF_P(value);
}

void zend_always_inline zend_object_ptr_copy(zend_object * destination, zend_object * source)
{
	if (EXPECTED(destination != NULL)) {
		OBJ_RELEASE(destination);
	}

	destination = source;
	GC_ADDREF(source);
}

void zend_always_inline zend_object_ptr_reset(zend_object * destination)
{
	if (EXPECTED(destination != NULL)) {
		OBJ_RELEASE(destination);
	}

	destination = NULL;
}

/**
 * Creates a new weak reference to the given zval.
 *
 * This function attempts to create a weak reference to the specified `referent` by invoking
 * the `WeakReference::create` method. If the creation function is not yet cached, the function
 * retrieves and caches the method reference from the `WeakReference` class during the first call.
 *
 * @param referent  A constant pointer to the zval that will be referenced weakly.
 *
 * @return A pointer to a newly allocated zval containing the weak reference object,
 *         or NULL if the creation fails.
 *
 * @note
 * - If the `WeakReference::create` method cannot be found, the function triggers a core error
 *   and halts execution.
 * - If the invocation of `WeakReference::create` returns `NULL` or an undefined value,
 *   a warning is issued, and the allocated memory is freed.
 * - The caller is responsible for managing the returned zval, which must be freed using
 *   `zval_ptr_dtor()` when no longer needed.
 * - The function performs dynamic allocation for the return value (`retval`), ensuring
 *   the resulting object is heap-allocated and safe to return.
 */
zval* async_new_weak_reference_from(const zval* referent);

/**
 * Resolves a weak reference to its underlying object.
 *
 * This method attempts to retrieve the object referenced by the weak reference.
 * If the referenced object has been garbage collected, the method returns NULL.
 *
 * @param weak_reference  A pointer to the zval representing the weak reference object.
 * @param retval          A pointer to a zval that will hold the resolved object.
 *
 * @warning
 * - The `retval` may contain a zval of type `NULL` if the referenced object no longer exists.
 * - If the result is no longer needed, you must call `zval_ptr_dtor()` to properly free the memory.
 *
 * @note
 * - If the `WeakReference::get` method is not yet cached, it is retrieved from the
 *   `WeakReference` class during the first call.
 * - If the method cannot be found, a core error is triggered, terminating execution.
 * - The function internally calls the `WeakReference::get` method to resolve the reference.
 * - The `retval` must be initialized and will contain the resulting object or NULL.
 */
void async_resolve_weak_reference(zval* weak_reference, zval* retval);

#endif //ASYNC_ZEND_COMMON_H
