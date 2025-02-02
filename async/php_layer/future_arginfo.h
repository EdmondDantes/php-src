/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f45fe5434d3ac5ebd9a94518edf8b8d35be01e75 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_FutureState___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FutureState_complete, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, result, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FutureState_error, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, throwable, Throwable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FutureState_isComplete, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FutureState_ignore, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_FutureState, __construct);
ZEND_METHOD(Async_FutureState, complete);
ZEND_METHOD(Async_FutureState, error);
ZEND_METHOD(Async_FutureState, isComplete);
ZEND_METHOD(Async_FutureState, ignore);

static const zend_function_entry class_Async_FutureState_methods[] = {
	ZEND_ME(Async_FutureState, __construct, arginfo_class_Async_FutureState___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, complete, arginfo_class_Async_FutureState_complete, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, error, arginfo_class_Async_FutureState_error, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, isComplete, arginfo_class_Async_FutureState_isComplete, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, ignore, arginfo_class_Async_FutureState_ignore, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_FutureState(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FutureState", class_Async_FutureState_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval property_complete_default_value;
	ZVAL_FALSE(&property_complete_default_value);
	zend_string *property_complete_name = zend_string_init("complete", sizeof("complete") - 1, 1);
	zend_declare_typed_property(class_entry, property_complete_name, &property_complete_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_complete_name);

	zval property_handled_default_value;
	ZVAL_FALSE(&property_handled_default_value);
	zend_string *property_handled_name = zend_string_init("handled", sizeof("handled") - 1, 1);
	zend_declare_typed_property(class_entry, property_handled_name, &property_handled_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_handled_name);

	zval property_result_default_value;
	ZVAL_NULL(&property_result_default_value);
	zend_string *property_result_name = zend_string_init("result", sizeof("result") - 1, 1);
	zend_declare_typed_property(class_entry, property_result_name, &property_result_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ANY));
	zend_string_release(property_result_name);

	zval property_throwable_default_value;
	ZVAL_NULL(&property_throwable_default_value);
	zend_string *property_throwable_name = zend_string_init("throwable", sizeof("throwable") - 1, 1);
	zend_string *property_throwable_class_Throwable = zend_string_init("Throwable", sizeof("Throwable")-1, 1);
	zend_declare_typed_property(class_entry, property_throwable_name, &property_throwable_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_throwable_class_Throwable, 0, MAY_BE_NULL));
	zend_string_release(property_throwable_name);

	return class_entry;
}
