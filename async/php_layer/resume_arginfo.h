/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 01dace9ddc4a53cc4d49b47ad880a4f8652ec114 */

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
	ZEND_ARG_TYPE_MASK(0, callback, MAY_BE_CALLABLE|MAY_BE_LONG, "Async\\Resume::RESUME")
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

	zval const_RESUME_value;
	ZVAL_LONG(&const_RESUME_value, 1);
	zend_string *const_RESUME_name = zend_string_init_interned("RESUME", sizeof("RESUME") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_RESUME_name, &const_RESUME_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_RESUME_name);

	zval const_THROW_value;
	ZVAL_LONG(&const_THROW_value, 2);
	zend_string *const_THROW_name = zend_string_init_interned("THROW", sizeof("THROW") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_THROW_name, &const_THROW_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_THROW_name);

	zval const_CANCEL_value;
	ZVAL_LONG(&const_CANCEL_value, 3);
	zend_string *const_CANCEL_name = zend_string_init_interned("CANCEL", sizeof("CANCEL") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_CANCEL_name, &const_CANCEL_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_CANCEL_name);

	zval const_TIMEOUT_value;
	ZVAL_LONG(&const_TIMEOUT_value, 4);
	zend_string *const_TIMEOUT_name = zend_string_init_interned("TIMEOUT", sizeof("TIMEOUT") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_TIMEOUT_name, &const_TIMEOUT_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_TIMEOUT_name);

	return class_entry;
}
