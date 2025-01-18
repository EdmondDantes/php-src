/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f7a7406cf03cc95a305639f14dac323c246aca87 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Resume___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_resume, 0, 0, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_throw, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, error, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_isPending, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Resume_isResolved arginfo_class_Async_Resume_isPending

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_getNotifiers, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Resume_getTriggeredNotifiers arginfo_class_Async_Resume_getNotifiers

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_when, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, notifier, Async\\\116otifier, 0)
	ZEND_ARG_TYPE_MASK(0, callback, MAY_BE_CALLABLE|MAY_BE_LONG, "Async\\Resume::RESOLVE")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_removeNotifier, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, notifier, Async\\\116otifier, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Resume, __construct);
ZEND_METHOD(Async_Resume, resume);
ZEND_METHOD(Async_Resume, throw);
ZEND_METHOD(Async_Resume, isPending);
ZEND_METHOD(Async_Resume, isResolved);
ZEND_METHOD(Async_Resume, getNotifiers);
ZEND_METHOD(Async_Resume, getTriggeredNotifiers);
ZEND_METHOD(Async_Resume, when);
ZEND_METHOD(Async_Resume, removeNotifier);
ZEND_METHOD(Async_Resume, __toString);

static const zend_function_entry class_Async_Resume_methods[] = {
	ZEND_ME(Async_Resume, __construct, arginfo_class_Async_Resume___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, resume, arginfo_class_Async_Resume_resume, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, throw, arginfo_class_Async_Resume_throw, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, isPending, arginfo_class_Async_Resume_isPending, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, isResolved, arginfo_class_Async_Resume_isResolved, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, getNotifiers, arginfo_class_Async_Resume_getNotifiers, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, getTriggeredNotifiers, arginfo_class_Async_Resume_getTriggeredNotifiers, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, when, arginfo_class_Async_Resume_when, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, removeNotifier, arginfo_class_Async_Resume_removeNotifier, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, __toString, arginfo_class_Async_Resume___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Resume(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Resume", class_Async_Resume_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval const_RESOLVE_value;
	ZVAL_LONG(&const_RESOLVE_value, 1);
	zend_string *const_RESOLVE_name = zend_string_init_interned("RESOLVE", sizeof("RESOLVE") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_RESOLVE_name, &const_RESOLVE_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_RESOLVE_name);

	zval const_CANCEL_value;
	ZVAL_LONG(&const_CANCEL_value, 2);
	zend_string *const_CANCEL_name = zend_string_init_interned("CANCEL", sizeof("CANCEL") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_CANCEL_name, &const_CANCEL_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_CANCEL_name);

	zval const_TIMEOUT_value;
	ZVAL_LONG(&const_TIMEOUT_value, 3);
	zend_string *const_TIMEOUT_name = zend_string_init_interned("TIMEOUT", sizeof("TIMEOUT") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_TIMEOUT_name, &const_TIMEOUT_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_TIMEOUT_name);

	zval property_result_default_value;
	ZVAL_NULL(&property_result_default_value);
	zend_string *property_result_name = zend_string_init("result", sizeof("result") - 1, 1);
	zend_declare_typed_property(class_entry, property_result_name, &property_result_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ANY));
	zend_string_release(property_result_name);

	return class_entry;
}
