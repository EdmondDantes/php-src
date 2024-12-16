//
// Created by Edmond on 16.12.2024.
//

#include "zend_common.h"
#include "callback.h"
#include "callback_arginfo.h"

ZEND_API zend_class_entry* async_ce_callback;
static zend_object_handlers async_callback_handlers;

static zend_object* async_callback_object_create(zend_class_entry* ce)
{
	zend_fiber* fiber = emalloc(sizeof(zend_fiber));
	memset(fiber, 0, sizeof(zend_fiber));

	zend_object_std_init(&fiber->std, ce);
	return &fiber->std;
}

static void async_callback_object_destroy(zend_object* object)
{
	zend_fiber* fiber = (zend_fiber*)object;
}

static void async_callback_object_free(zend_object* object)
{
	zend_fiber* fiber = (zend_fiber*)object;

	zval_ptr_dtor(&fiber->fci.function_name);
	zval_ptr_dtor(&fiber->result);

	zend_object_std_dtor(&fiber->std);
}

void async_register_notifier_ce(void)
{
	async_ce_callback = register_class_Async_Notifier();
	async_ce_callback->create_object = async_callback_object_create;
	async_ce_callback->default_object_handlers = &async_callback_handlers;

	async_callback_handlers = std_object_handlers;
	async_callback_handlers.dtor_obj = async_callback_object_destroy;
	async_callback_handlers.free_obj = async_callback_object_free;
	async_callback_handlers.clone_obj = NULL;
}

zend_result async_callback_notify(zend_object* callback, zend_object* notifier, zval* event, zval* error)
{

}
