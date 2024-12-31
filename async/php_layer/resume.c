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
#include "resume.h"
#include "notifier.h"
#include "callback.h"
#include "ev_handles.h"
#include "exceptions.h"
#include "resume_arginfo.h"

#define METHOD(name) PHP_METHOD(Async_Resume, name)
#define THIS(field) ((async_resume_t *) Z_OBJ_P(ZEND_THIS))->field

static zend_object_handlers async_resume_handlers;

METHOD(resume)
{
	if (THIS(status) != ASYNC_RESUME_PENDING) {
		async_throw_error("Cannot resume a non-pending operation");
		return;
	}

	zval *value;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	zval_copy(&THIS(result), value);
	THIS(status) = ASYNC_RESUME_SUCCESS;
}

METHOD(throw)
{
	if (THIS(status) != ASYNC_RESUME_PENDING) {
		async_throw_error("Cannot resume a non-pending operation");
		return;
	}

	zend_object *error;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OF_CLASS(error, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	if (THIS(error) != NULL) {
		GC_DELREF(THIS(error));
	}

	THIS(error) = error;
	GC_ADDREF(error);
	THIS(status) = ASYNC_RESUME_ERROR;
}

METHOD(isResolved)
{
	RETURN_BOOL(THIS(status) == ASYNC_RESUME_SUCCESS || THIS(status) == ASYNC_RESUME_ERROR);
}

METHOD(getEventDescriptors)
{
	GC_REFCOUNT(&THIS(notifiers));
	RETURN_ARR(&THIS(notifiers));
}

METHOD(getTriggeredNotifiers)
{
	if (THIS(triggered_notifiers) == NULL) {
		RETURN_EMPTY_ARRAY();
    }

	GC_REFCOUNT(THIS(triggered_notifiers));
	RETURN_ARR(THIS(triggered_notifiers));
}

#define RESUME_CALLBACK_RESOLVE 1
#define RESUME_CALLBACK_THROW 2
#define RESUME_CALLBACK_CANCEL 3
#define RESUME_CALLBACK_TIMEOUT 4

METHOD(when)
{
	if (THIS(status) != ASYNC_RESUME_NO_STATUS) {
		async_throw_error("You cannot modify the Resume object after it has been used in an await call");
		return;
	}

	zval *notifier;
	zval *callback = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_OBJECT_OF_CLASS(notifier, async_ce_notifier)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	async_resume_t * resume = (async_resume_t *) Z_OBJ_P(ZEND_THIS);

	if (zend_hash_index_find(&resume->notifiers, Z_OBJ_P(notifier)->handle) != NULL) {
		zend_throw_exception(zend_ce_error, "Notifier already registered", 0);
		RETURN_THROWS();
	}

	async_resume_notifier_t * resume_notifier = emalloc(sizeof(async_resume_notifier_t));
	resume_notifier->notifier = (reactor_notifier_t *) Z_OBJ_P(notifier);
	ZVAL_NULL(&resume_notifier->callback);

	if (Z_TYPE_P(callback) == IS_LONG) {
		switch (Z_LVAL_P(callback))
		{
			case RESUME_CALLBACK_RESOLVE:
				ZVAL_PTR(&resume_notifier->callback, async_resume_when_callback_resolve);
				break;
			case RESUME_CALLBACK_CANCEL:
				ZVAL_PTR(&resume_notifier->callback, async_resume_when_callback_cancel);
				break;
			case RESUME_CALLBACK_TIMEOUT:

				if (Z_OBJ_P(notifier)->ce != async_ce_timer_handle) {
					zend_throw_error(zend_ce_type_error, "Timeout callback can only be used with TimerHandle");
					RETURN_THROWS();
				}

				ZVAL_PTR(&resume_notifier->callback, async_resume_when_callback_timeout);
				break;

			default:
				zend_throw_exception(zend_ce_type_error, "Invalid default callback type. Should be RESUME, THROW, CANCEL, TIMEOUT", 0);
				RETURN_THROWS();
		}
	} else if (zend_is_callable(callback, 0, NULL)) {
		ZVAL_COPY(&resume_notifier->callback, callback);
	} else {
		zend_throw_exception_ex(zend_ce_type_error, 0, "Expected parameter callback to be a valid callable");
		RETURN_THROWS();
	}

	zval zval_notifier;
	ZVAL_PTR(&zval_notifier, resume_notifier);
	zend_hash_index_update(&resume->notifiers, Z_OBJ_P(notifier)->handle, &zval_notifier);
}

static zend_object *async_resume_object_create(zend_class_entry *class_entry)
{
	async_resume_t * object = zend_object_alloc(sizeof(reactor_handle_t), class_entry);

	zend_object_std_init(&object->std, class_entry);
	object_properties_init(&object->std, class_entry);

	object->std.handlers = &async_resume_handlers;

	// Define current Fiber and set it to the property $fiber
	if (EXPECTED(EG(active_fiber))) {
		object->fiber = EG(active_fiber);
		zend_hash_init(&object->notifiers, 4, NULL, ZVAL_PTR_DTOR, 0);
	}

	return &object->std;
}

static void async_resume_object_destroy(zend_object* object)
{
}

static void async_resume_object_free(zend_object* object)
{
    async_resume_t * resume = (async_resume_t *) object;

    if (resume->triggered_notifiers != NULL) {
        zend_array_destroy(resume->triggered_notifiers);
    }

    zend_hash_destroy(&resume->notifiers);
    zend_object_std_dtor(&resume->std);
}

void async_register_resume_ce(void)
{
	async_ce_resume = register_class_Async_Resume();

	async_ce_resume->default_object_handlers = &async_resume_handlers;
	async_ce_resume->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
    async_ce_resume->create_object = async_resume_object_create;

	async_resume_handlers = std_object_handlers;
	async_resume_handlers.dtor_obj = async_resume_object_destroy;
	async_resume_handlers.free_obj = async_resume_object_free;
	async_resume_handlers.clone_obj = NULL;
}

async_resume_t * async_resume_new()
{
	zval object;

	// Create a new object without calling the constructor
	if (object_init_ex(&object, async_ce_resume) == FAILURE) {
		return NULL;
	}

	return (async_resume_t *) Z_OBJ_P(&object);
}

void async_resume_notify(async_resume_t* resume, reactor_notifier_t* notifier, const zval* event, const zval* error)
{
	// If the triggered_notifiers array is not initialized, create it.
	if (resume->triggered_notifiers == NULL) {
		resume->triggered_notifiers = zend_new_array(4);
	}

	// Try to find the notifier in the notifiers array.
	const zval * known_notifier = zend_hash_index_find(&resume->notifiers, notifier->std.handle);

	if (known_notifier == NULL) {
		zend_error(
			E_WARNING,
			"Received notification from an unregistered Notifier object. Class: %s",
			ZSTR_VAL(notifier->std.ce->name)
		);
		return;
	}

	// The elements of the notifiers array are custom data structures of type async_resume_notifier_t.
	if (Z_TYPE_P(known_notifier) != IS_PTR) {
		return;
	}

	async_resume_notifier_t * resume_notifier = Z_PTR_P(known_notifier);

	zval zval_notifier;
	ZVAL_OBJ(&zval_notifier, &notifier->std);
	zend_hash_index_update(resume->triggered_notifiers, notifier->std.handle, &zval_notifier);

	/**
	 * The callback can be of two types:
	 * * An internal Zend Engine callback, a pointer to a C function of type `async_resume_when_callback_t`.
	 * * A `PHP` user-mode callback.
	 */
	if (Z_TYPE(resume_notifier->callback) == IS_PTR) {

		const async_resume_when_callback_t resume_callback = (async_resume_when_callback_t) Z_PTR_P(known_notifier);
		resume_callback(resume, notifier, event, error);

	} else if (zend_is_callable(&resume_notifier->callback, 0, NULL)) {

		zval retval;
		ZVAL_UNDEF(&retval);
		zval params[3];
		ZVAL_OBJ(&params[0], &notifier->std);
		ZVAL_COPY(&params[1], event);
		ZVAL_COPY(&params[2], error);

		call_user_function(CG(function_table), NULL, &resume_notifier->callback, &retval, 3, params);
		zval_ptr_dtor(&retval);
	}
}

void async_resume_pending(async_resume_t *resume)
{
    resume->status = ASYNC_RESUME_PENDING;

	if (resume->triggered_notifiers != NULL) {
        zend_array_release(resume->triggered_notifiers);
        resume->triggered_notifiers = NULL;
    }
}