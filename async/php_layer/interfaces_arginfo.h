/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: fd684a6b865417883977fe0e4b665325c984ae40 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CallbackInterface_disposeCallback, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_EventHandleInterface_addCallback, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Async\\CallbackInterface, 0)
ZEND_END_ARG_INFO()


static const zend_function_entry class_Async_CallbackInterface_methods[] = {
	ZEND_RAW_FENTRY("disposeCallback", NULL, arginfo_class_Async_CallbackInterface_disposeCallback, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_EventHandleInterface_methods[] = {
	ZEND_RAW_FENTRY("addCallback", NULL, arginfo_class_Async_EventHandleInterface_addCallback, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_CallbackInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "CallbackInterface", class_Async_CallbackInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_EventHandleInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "EventHandleInterface", class_Async_EventHandleInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
