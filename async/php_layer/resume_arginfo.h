/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: e9c2428fbd83a2a50f54989549cafe13eb025d6c */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Resume___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_resume, 0, 0, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_throw, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, error, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_isResolved, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_getNotifiers, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Resume_getTriggeredNotifiers arginfo_class_Async_Resume_getNotifiers

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_when, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, notifier, Async\\\116otifier, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_CALLABLE, 1, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Resume, __construct);
ZEND_METHOD(Async_Resume, resume);
ZEND_METHOD(Async_Resume, throw);
ZEND_METHOD(Async_Resume, isResolved);
ZEND_METHOD(Async_Resume, getNotifiers);
ZEND_METHOD(Async_Resume, getTriggeredNotifiers);
ZEND_METHOD(Async_Resume, when);

static const zend_function_entry class_Async_Resume_methods[] = {
	ZEND_ME(Async_Resume, __construct, arginfo_class_Async_Resume___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Async_Resume, resume, arginfo_class_Async_Resume_resume, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, throw, arginfo_class_Async_Resume_throw, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, isResolved, arginfo_class_Async_Resume_isResolved, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, getNotifiers, arginfo_class_Async_Resume_getNotifiers, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, getTriggeredNotifiers, arginfo_class_Async_Resume_getTriggeredNotifiers, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, when, arginfo_class_Async_Resume_when, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Resume(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Resume", class_Async_Resume_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL);

	zval property_callback_default_value;
	ZVAL_NULL(&property_callback_default_value);
	zend_string *property_callback_name = zend_string_init("callback", sizeof("callback") - 1, 1);
	zend_string *property_callback_class_Async_Callback = zend_string_init("Async\\Callback", sizeof("Async\\Callback")-1, 1);
	zend_declare_typed_property(class_entry, property_callback_name, &property_callback_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_callback_class_Async_Callback, 0, MAY_BE_NULL));
	zend_string_release(property_callback_name);

	return class_entry;
}
