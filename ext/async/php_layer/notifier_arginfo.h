/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b2c7f313ef6db649cefa77e03c9955268bf59dcf */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_getCallbacks, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_addCallback, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Async\\Closure, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Notifier_removeCallback arginfo_class_Async_Notifier_addCallback

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_notify, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, error, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_close, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_getInfo, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_NotifierEx___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Cancellation_isCancelled, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Cancellation_cancel, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Notifier, getCallbacks);
ZEND_METHOD(Async_Notifier, addCallback);
ZEND_METHOD(Async_Notifier, removeCallback);
ZEND_METHOD(Async_Notifier, notify);
ZEND_METHOD(Async_Notifier, close);
ZEND_METHOD(Async_Notifier, getInfo);
ZEND_METHOD(Async_NotifierEx, __construct);
ZEND_METHOD(Async_Cancellation, isCancelled);
ZEND_METHOD(Async_Cancellation, cancel);

static const zend_function_entry class_Async_Notifier_methods[] = {
	ZEND_ME(Async_Notifier, getCallbacks, arginfo_class_Async_Notifier_getCallbacks, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	ZEND_ME(Async_Notifier, addCallback, arginfo_class_Async_Notifier_addCallback, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Notifier, removeCallback, arginfo_class_Async_Notifier_removeCallback, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Notifier, notify, arginfo_class_Async_Notifier_notify, ZEND_ACC_PROTECTED|ZEND_ACC_FINAL)
	ZEND_ME(Async_Notifier, close, arginfo_class_Async_Notifier_close, ZEND_ACC_PROTECTED|ZEND_ACC_FINAL)
	ZEND_ME(Async_Notifier, getInfo, arginfo_class_Async_Notifier_getInfo, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_NotifierEx_methods[] = {
	ZEND_ME(Async_NotifierEx, __construct, arginfo_class_Async_NotifierEx___construct, ZEND_ACC_PRIVATE)
	ZEND_FE_END
};

static const zend_function_entry class_Async_Cancellation_methods[] = {
	ZEND_ME(Async_Cancellation, isCancelled, arginfo_class_Async_Cancellation_isCancelled, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Cancellation, cancel, arginfo_class_Async_Cancellation_cancel, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Notifier(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Notifier", class_Async_Notifier_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval property_callbacks_default_value;
	ZVAL_EMPTY_ARRAY(&property_callbacks_default_value);
	zend_string *property_callbacks_name = zend_string_init("callbacks", sizeof("callbacks") - 1, 1);
	zend_declare_typed_property(class_entry, property_callbacks_name, &property_callbacks_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
	zend_string_release(property_callbacks_name);

	zval property_isClosed_default_value;
	ZVAL_FALSE(&property_isClosed_default_value);
	zend_string *property_isClosed_name = zend_string_init("isClosed", sizeof("isClosed") - 1, 1);
	zend_declare_typed_property(class_entry, property_isClosed_name, &property_isClosed_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_isClosed_name);

	zval property_debugInfo_default_value;
	ZVAL_UNDEF(&property_debugInfo_default_value);
	zend_string *property_debugInfo_name = zend_string_init("debugInfo", sizeof("debugInfo") - 1, 1);
	zend_declare_typed_property(class_entry, property_debugInfo_name, &property_debugInfo_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ANY));
	zend_string_release(property_debugInfo_name);

	return class_entry;
}

static zend_class_entry *register_class_Async_NotifierEx(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "NotifierEx", class_Async_NotifierEx_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_Cancellation(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Cancellation", class_Async_Cancellation_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval property_notifier_default_value;
	ZVAL_UNDEF(&property_notifier_default_value);
	zend_string *property_notifier_name = zend_string_init("notifier", sizeof("notifier") - 1, 1);
	zend_string *property_notifier_class_Async_Notifier = zend_string_init("Async\\\116otifier", sizeof("Async\\\116otifier")-1, 1);
	zend_declare_typed_property(class_entry, property_notifier_name, &property_notifier_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_notifier_class_Async_Notifier, 0, 0));
	zend_string_release(property_notifier_name);

	return class_entry;
}
