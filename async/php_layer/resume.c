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
#include "resume_arginfo.h"

#define METHOD(name) PHP_METHOD(Async_Resume, name)
#define PROPERTY_CALLBACK "callback"
#define PROPERTY_FIBER "fiber"
#define GET_PROPERTY_CALLBACK() async_resume_get_callback(Z_OBJ_P(ZEND_THIS));


static zend_object_handlers async_resume_handlers;


METHOD(__construct)
{
}

METHOD(resume)
{
}

METHOD(throw)
{
}

METHOD(isResolved)
{
}

METHOD(getEventDescriptors)
{
}

METHOD(when)
{
	zval *notifier;
	zval *callback = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_OBJECT_OF_CLASS(notifier, async_ce_notifier)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(callback)
	ZEND_PARSE_PARAMETERS_END();

	if (Z_TYPE_P(callback) != IS_NULL && !zend_is_callable(callback, 0, NULL)) {
		zend_throw_exception_ex(zend_ce_type_error, 0, "Expected parameter callback to be a valid callable");
		RETURN_THROWS();
	}

	zval * property_callback = GET_PROPERTY_CALLBACK();

	zval_ptr_dtor(property_callback);
	ZVAL_DUP(property_callback, callback);
}

static zend_object *async_resume_object_create(zend_class_entry *ce)
{
	zend_object *object = zend_objects_new(ce);

	zend_object_std_init(object, ce);
	object_properties_init(object, ce);

	object->handlers = &async_resume_handlers;

	// Define current Fiber and set it to the property $fiber
	if (EXPECTED(EG(active_fiber))) {
		zval fiber_val;
		ZVAL_OBJ(&fiber_val, &EG(active_fiber)->std);
		zend_update_property(ce, object, PROPERTY_FIBER, sizeof(PROPERTY_FIBER) - 1, &fiber_val);
	}

	return object;
}

static void async_resume_object_destroy(zend_object* object)
{
}

void async_register_resume_ce(void)
{
	async_ce_resume = register_class_Async_Resume();

	async_ce_resume->default_object_handlers = &async_resume_handlers;
	async_ce_resume->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
    async_ce_resume->create_object = async_resume_object_create;

	async_resume_handlers = std_object_handlers;
	async_resume_handlers.dtor_obj = async_resume_object_destroy;
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