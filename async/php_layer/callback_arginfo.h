/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8b4aef18255a5f082043a875db7cf7d20c8a108b */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Callback___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Callback_isRegistered, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Callback_disposeCallback, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Callback, __construct);
ZEND_METHOD(Async_Callback, isRegistered);
ZEND_METHOD(Async_Callback, disposeCallback);

static const zend_function_entry class_Async_Callback_methods[] = {
	ZEND_ME(Async_Callback, __construct, arginfo_class_Async_Callback___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Callback, isRegistered, arginfo_class_Async_Callback_isRegistered, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Callback, disposeCallback, arginfo_class_Async_Callback_disposeCallback, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Callback(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Callback", class_Async_Callback_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	zval property_callback_default_value;
	ZVAL_UNDEF(&property_callback_default_value);
	zend_string *property_callback_name = zend_string_init("callback", sizeof("callback") - 1, 1);
	zend_declare_typed_property(class_entry, property_callback_name, &property_callback_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ANY));
	zend_string_release(property_callback_name);

	zval property_notifier_default_value;
	ZVAL_NULL(&property_notifier_default_value);
	zend_string *property_notifier_name = zend_string_init("notifier", sizeof("notifier") - 1, 1);
	zend_string *property_notifier_class_Async_Notifier = zend_string_init("Async\\\116otifier", sizeof("Async\\\116otifier")-1, 1);
	zend_declare_typed_property(class_entry, property_notifier_name, &property_notifier_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_notifier_class_Async_Notifier, 0, MAY_BE_NULL));
	zend_string_release(property_notifier_name);

	return class_entry;
}
