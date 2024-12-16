/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 6e56e5f5787f9eb07999164ebf86d18dddb46510 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_resume, 0, 0, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_throw, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, error, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_isResolved, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_getEventDescriptors, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Resume_when, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, notifier, Async\\Callback, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, callback, IS_CALLABLE, 1, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Resume, resume);
ZEND_METHOD(Async_Resume, throw);
ZEND_METHOD(Async_Resume, isResolved);
ZEND_METHOD(Async_Resume, getEventDescriptors);
ZEND_METHOD(Async_Resume, when);

static const zend_function_entry class_Async_Resume_methods[] = {
	ZEND_ME(Async_Resume, resume, arginfo_class_Async_Resume_resume, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, throw, arginfo_class_Async_Resume_throw, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, isResolved, arginfo_class_Async_Resume_isResolved, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, getEventDescriptors, arginfo_class_Async_Resume_getEventDescriptors, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Resume, when, arginfo_class_Async_Resume_when, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Resume(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Resume", class_Async_Resume_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL);

	return class_entry;
}
