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

#define IF_THROW_RETURN_VOID if(UNEXPECTED(EG(exception) != NULL)) { return; }
#define IF_THROW_FINALLY if(UNEXPECTED(EG(exception) != NULL)) { goto finally; }
#define IF_THROW_RETURN(value) if(UNEXPECTED(EG(exception) != NULL)) { return value; }

#define DEFINE_VAR(type, var) type *var = (type *) emalloc(sizeof(type));

//
// Atomic operations
//

/* Define atomic pointer type based on platform */
#if defined(ZEND_WIN32)
/* Windows: Use volatile pointer for atomicity */
# define ATOMIC_PTR(T) T* volatile
# define zend_atomic_ptr_t void* volatile
#elif defined(HAVE_C11_ATOMICS)
/* C11: Use standard atomic types */
# include <stdatomic.h>
# define ATOMIC_PTR(T) _Atomic(T*)
# define zend_atomic_ptr_t _Atomic(void*)
#elif defined(HAVE_GNUC_ATOMICS) || defined(HAVE_SYNC_ATOMICS)
/* GCC or compatible: Use volatile pointer */
# define ATOMIC_PTR(T) T* volatile
# define zend_atomic_ptr_t void* volatile
#else
/* Fallback: Use volatile pointer (non-atomic) */
# define ATOMIC_PTR(T) T* volatile
# define zend_atomic_ptr_t void* volatile
#endif

/* Initialize an atomic pointer */
#define ZEND_ATOMIC_PTR_INIT(ptr, val) (*(ptr) = (val))

/* Windows implementation using Interlocked API */
#ifdef ZEND_WIN32
#include <windows.h>

zend_always_inline
void* zend_atomic_ptr_exchange(zend_atomic_ptr_t* target, void* value) {
    // Use InterlockedExchangePointer with proper type casting
    return InterlockedExchangePointer(
        (PVOID volatile*)target,
        value
    );
}

zend_always_inline
bool zend_atomic_ptr_compare_exchange(zend_atomic_ptr_t* target, void** expected, void* desired) {
    // Use InterlockedCompareExchangePointer with proper type casting
    void* prev = InterlockedCompareExchangePointer(
        (PVOID volatile*)target,
        desired,
        *expected
    );
    if (prev == *expected) return true;
    *expected = prev;
    return false;
}

zend_always_inline
void* zend_atomic_ptr_load(zend_atomic_ptr_t* target) {
    // Load using exchange with itself
    return InterlockedExchangePointer(
        (PVOID volatile*)target,
        *target
    );
}

zend_always_inline
void zend_atomic_ptr_store(zend_atomic_ptr_t* target, void* value) {
    // Store using exchange
    InterlockedExchangePointer(
        (PVOID volatile*)target,
        value
    );
}

/* C11 atomic implementation */
#elif defined(HAVE_C11_ATOMICS)

zend_always_inline
void* zend_atomic_ptr_exchange(zend_atomic_ptr_t* target, void* value) {
    // Use standard atomic exchange
    return atomic_exchange_explicit(target, value, memory_order_seq_cst);
}

zend_always_inline
bool zend_atomic_ptr_compare_exchange(zend_atomic_ptr_t* target, void** expected, void* desired) {
    // Use standard atomic compare-and-swap
    return atomic_compare_exchange_strong_explicit(
        target, expected, desired,
        memory_order_seq_cst, memory_order_seq_cst
    );
}

zend_always_inline
void* zend_atomic_ptr_load(zend_atomic_ptr_t* target) {
    // Use standard atomic load
    return atomic_load_explicit(target, memory_order_seq_cst);
}

zend_always_inline
void zend_atomic_ptr_store(zend_atomic_ptr_t* target, void* value) {
    // Use standard atomic store
    atomic_store_explicit(target, value, memory_order_seq_cst);
}

/* GCC atomic implementation */
#elif defined(HAVE_GNUC_ATOMICS)

zend_always_inline
void* zend_atomic_ptr_exchange(zend_atomic_ptr_t* target, void* value) {
    // Use GCC built-in atomic exchange
    void* prev;
    __atomic_exchange(target, &value, &prev, __ATOMIC_SEQ_CST);
    return prev;
}

zend_always_inline
bool zend_atomic_ptr_compare_exchange(zend_atomic_ptr_t* target, void** expected, void* desired) {
    // Use GCC built-in compare-and-swap
    return __atomic_compare_exchange(
        target, expected, &desired,
        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST
    );
}

zend_always_inline
void* zend_atomic_ptr_load(zend_atomic_ptr_t* target) {
    // Use GCC built-in atomic load
    void* val;
    __atomic_load(target, &val, __ATOMIC_SEQ_CST);
    return val;
}

zend_always_inline
void zend_atomic_ptr_store(zend_atomic_ptr_t* target, void* value) {
    // Use GCC built-in atomic store
    __atomic_store(target, &value, __ATOMIC_SEQ_CST);
}

/* Legacy GCC sync implementation */
#elif defined(HAVE_SYNC_ATOMICS)

zend_always_inline
void* zend_atomic_ptr_exchange(zend_atomic_ptr_t* target, void* value) {
    // Use GCC sync built-in exchange with memory barrier
    void* prev = __sync_lock_test_and_set(target, value);
    __sync_synchronize();
    return prev;
}

zend_always_inline
bool zend_atomic_ptr_compare_exchange(zend_atomic_ptr_t* target, void** expected, void* desired) {
    // Use GCC sync built-in compare-and-swap
    void* prev = __sync_val_compare_and_swap(target, *expected, desired);
    if (prev == *expected) return true;
    *expected = prev;
    return false;
}

zend_always_inline
void* zend_atomic_ptr_load(zend_atomic_ptr_t* target) {
    // Use GCC sync built-in fetch-and-or with 0 to load
    return __sync_fetch_and_or(target, 0);
}

zend_always_inline
void zend_atomic_ptr_store(zend_atomic_ptr_t* target, void* value) {
    // Use memory barriers for store
    __sync_synchronize();
    *target = value;
    __sync_synchronize();
}

/* Fallback non-atomic implementation */
#else

zend_always_inline
void* zend_atomic_ptr_exchange(zend_atomic_ptr_t* target, void* value) {
    // Non-atomic exchange
    void* prev = *target;
    *target = value;
    return prev;
}

zend_always_inline
bool zend_atomic_ptr_compare_exchange(zend_atomic_ptr_t* target, void** expected, void* desired) {
    // Non-atomic compare-and-swap
    if (*target == *expected) {
        *target = desired;
        return true;
    }
    *expected = *target;
    return false;
}

zend_always_inline
void* zend_atomic_ptr_load(zend_atomic_ptr_t* target) {
    // Non-atomic load
    return *target;
}

zend_always_inline
void zend_atomic_ptr_store(zend_atomic_ptr_t* target, void* value) {
    // Non-atomic store
    *target = value;
}

#endif

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

zend_always_inline void async_warning(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	zend_string *message = zend_vstrpprintf(0, format, args);
	va_end(args);
	zend_error(E_CORE_WARNING, message->val);
	zend_string_release(message);
}

/**
 * Allocates memory for a new object instance.
 * The difference from the standard zend_object_alloc function
 * is that this function is intended for custom structures where zend_object starts the structure.
 * Therefore, the entire data structure is zeroed out, not just part of it.
 *
 * This function allocates memory for a new object instance of the specified size.
 * The function initializes the memory block with zero bytes and returns a pointer to the allocated memory.
 *
 * @param obj_size  The size of the object instance to allocate.
 * @param ce        The class entry of the object instance.
 *
 * @return A pointer to the allocated memory block.
 */
zend_always_inline void *zend_object_alloc_ex(const size_t obj_size, zend_class_entry *ce)
{
	ZEND_ASSERT(zend_object_properties_size(ce) >= 0);
	const size_t size = obj_size + zend_object_properties_size(ce);
	void *obj = emalloc(size);
	memset(obj, 0, size);
	return obj;
}

#define DEFINE_ZEND_RAW_OBJECT(type, var, class_entry) type *var = (type *) zend_object_alloc_ex(sizeof(type), class_entry)

zend_always_inline zend_object* zend_object_internal_create(const size_t obj_size, zend_class_entry *class_entry)
{
	zend_object * object = zend_object_alloc_ex(obj_size, class_entry);

	zend_object_std_init(object, class_entry);
	object_properties_init(object, class_entry);

	return object;
}

#define DEFINE_ZEND_INTERNAL_OBJECT(type, var, class_entry) type *var = (type *) zend_object_internal_create(sizeof(type), class_entry)

zend_always_inline void zend_property_array_index_update(zval *property, zend_ulong h, zval *pData, const bool is_transfer_data)
{
	SEPARATE_ARRAY(property);

	if (EXPECTED(zend_hash_index_update(Z_ARRVAL_P(property), h, pData) != NULL)) {
		if (false == is_transfer_data) {
			Z_TRY_ADDREF_P(pData);
		}
	} else if (is_transfer_data) {
		zval_ptr_dtor(pData);
	}
}

zend_always_inline void zend_apply_current_filename_and_line(zend_string **filename, uint32_t *lineno)
{
	if (*filename != NULL) {
		zend_string_release(*filename);
		*filename = NULL;
		*lineno = 0;
	}

	if (zend_is_compiling()) {
		*filename = zend_get_compiled_filename();
		*lineno = zend_get_compiled_lineno();
	} else if (zend_is_executing()) {
		*filename = zend_get_executed_filename_ex();
		*lineno = zend_get_executed_lineno();
	} else {
		*filename = NULL;
		*lineno = 0;
	}

	if (*filename != NULL) {
		zend_string_addref(*filename);
	}
}

void zend_exception_to_warning(const char * format, const bool clean);

zend_string * zend_current_exception_get_message(const bool clean);
zend_string * zend_current_exception_get_file(void);
uint32_t zend_current_exception_get_line(void);

/**
 * Creates a new weak reference to the given zval.
 *
 * This function attempts to create a weak reference to the specified `referent` by invoking
 * the `WeakReference::create` method. If the creation function is not yet cached, the function
 * retrieves and caches the method reference from the `WeakReference` class during the first call.
 *
 * @param referent  A constant pointer to the zval that will be referenced weakly.
 * @param retval    A pointer to a zval that will hold the weak reference object.
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
 */
void zend_new_weak_reference_from(const zval* referent, zval * retval);

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
 * - You must call `zval_ptr_dtor()` to decrement the reference count of the returned object.
 */
void zend_resolve_weak_reference(zval* weak_reference, zval* retval);

zif_handler zend_hook_php_function(const char *name, const size_t len, zif_handler new_function);

zif_handler zend_replace_method(zend_object * object, const char * method, const size_t len, const zif_handler handler);

zend_always_inline zif_handler zend_replace_to_string_method(zend_object * object, const zif_handler handler)
{
	return zend_replace_method(object, ZEND_STRL("__toString"), handler);
}

void zend_get_function_name_by_fci(zend_fcall_info * fci, zend_fcall_info_cache *fci_cache, zend_string **name);

#endif //ASYNC_ZEND_COMMON_H
