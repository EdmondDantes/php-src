/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 3f255fcc4dd2678fec17f5c6dd99939c387189ac */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Callback_disposeCallback, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Callback, disposeCallback);

static const zend_function_entry class_Async_Callback_methods[] = {
	ZEND_ME(Async_Callback, disposeCallback, arginfo_class_Async_Callback_disposeCallback, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Callback(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Callback", class_Async_Callback_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}
