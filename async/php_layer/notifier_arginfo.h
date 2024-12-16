/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 88f95b16a1de8f3dc3fdbfe4ef6beeac7e88ad12 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_addCallback, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Async\\Callback, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Notifier_removeCallback arginfo_class_Async_Notifier_addCallback

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_notify, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, error, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Notifier, addCallback);
ZEND_METHOD(Async_Notifier, removeCallback);
ZEND_METHOD(Async_Notifier, notify);

static const zend_function_entry class_Async_Notifier_methods[] = {
	ZEND_ME(Async_Notifier, addCallback, arginfo_class_Async_Notifier_addCallback, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Notifier, removeCallback, arginfo_class_Async_Notifier_removeCallback, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Notifier, notify, arginfo_class_Async_Notifier_notify, ZEND_ACC_PROTECTED|ZEND_ACC_FINAL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Notifier(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Notifier", class_Async_Notifier_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	return class_entry;
}
