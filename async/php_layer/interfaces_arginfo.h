/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 74d5675983c3f20005d13f037c022768e8b3f8d9 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CallbackInterface___invoke, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, error, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CallbackInterface_disposeCallback, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_NotifierInterface_addCallback, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Async\\CallbackInterface, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_NotifierInterface_removeCallback arginfo_class_Async_NotifierInterface_addCallback


static const zend_function_entry class_Async_CallbackInterface_methods[] = {
	ZEND_RAW_FENTRY("__invoke", NULL, arginfo_class_Async_CallbackInterface___invoke, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("disposeCallback", NULL, arginfo_class_Async_CallbackInterface_disposeCallback, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_NotifierInterface_methods[] = {
	ZEND_RAW_FENTRY("addCallback", NULL, arginfo_class_Async_NotifierInterface_addCallback, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("removeCallback", NULL, arginfo_class_Async_NotifierInterface_removeCallback, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_CallbackInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "CallbackInterface", class_Async_CallbackInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_NotifierInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "NotifierInterface", class_Async_NotifierInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
