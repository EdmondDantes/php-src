/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5751d631fea92ffe85992b9dd830cc9c78d299fa */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Callback___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Callback_disposeCallback, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Callback, __construct);
ZEND_METHOD(Async_Callback, disposeCallback);

static const zend_function_entry class_Async_Callback_methods[] = {
	ZEND_ME(Async_Callback, __construct, arginfo_class_Async_Callback___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Callback, disposeCallback, arginfo_class_Async_Callback_disposeCallback, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Callback(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Callback", class_Async_Callback_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}
