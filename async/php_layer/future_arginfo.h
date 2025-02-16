/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 921b8ccd30229d07e25c85115a950d4289927061 */

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

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Future_complete, 0, 0, Async\\Future, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Future_error, 0, 1, Async\\Future, 0)
	ZEND_ARG_OBJ_INFO(0, throwable, Throwable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Future___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, state, Async\\FutureState, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Future_isComplete arginfo_class_Async_FutureState_isComplete

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Future_ignore, 0, 0, Async\\Future, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Future_map, 0, 1, Async\\Future, 0)
	ZEND_ARG_TYPE_INFO(0, map, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Future_catch, 0, 1, Async\\Future, 0)
	ZEND_ARG_TYPE_INFO(0, catch, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Future_finally, 0, 1, Async\\Future, 0)
	ZEND_ARG_TYPE_INFO(0, finally, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Future_await, 0, 0, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_FutureState, __construct);
ZEND_METHOD(Async_FutureState, complete);
ZEND_METHOD(Async_FutureState, error);
ZEND_METHOD(Async_FutureState, isComplete);
ZEND_METHOD(Async_FutureState, ignore);
ZEND_METHOD(Async_Future, complete);
ZEND_METHOD(Async_Future, error);
ZEND_METHOD(Async_Future, __construct);
ZEND_METHOD(Async_Future, isComplete);
ZEND_METHOD(Async_Future, ignore);
ZEND_METHOD(Async_Future, map);
ZEND_METHOD(Async_Future, catch);
ZEND_METHOD(Async_Future, finally);
ZEND_METHOD(Async_Future, await);

static const zend_function_entry class_Async_FutureState_methods[] = {
	ZEND_ME(Async_FutureState, __construct, arginfo_class_Async_FutureState___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, complete, arginfo_class_Async_FutureState_complete, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, error, arginfo_class_Async_FutureState_error, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, isComplete, arginfo_class_Async_FutureState_isComplete, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FutureState, ignore, arginfo_class_Async_FutureState_ignore, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_Future_methods[] = {
	ZEND_ME(Async_Future, complete, arginfo_class_Async_Future_complete, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_Future, error, arginfo_class_Async_Future_error, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_Future, __construct, arginfo_class_Async_Future___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Future, isComplete, arginfo_class_Async_Future_isComplete, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Future, ignore, arginfo_class_Async_Future_ignore, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Future, map, arginfo_class_Async_Future_map, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Future, catch, arginfo_class_Async_Future_catch, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Future, finally, arginfo_class_Async_Future_finally, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Future, await, arginfo_class_Async_Future_await, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_FutureState(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FutureState", class_Async_FutureState_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

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

static zend_class_entry *register_class_Async_Future(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Future", class_Async_Future_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL);

	zval property_state_default_value;
	ZVAL_UNDEF(&property_state_default_value);
	zend_string *property_state_name = zend_string_init("state", sizeof("state") - 1, 1);
	zend_string *property_state_class_Async_FutureState = zend_string_init("Async\\FutureState", sizeof("Async\\FutureState")-1, 1);
	zend_declare_typed_property(class_entry, property_state_name, &property_state_default_value, ZEND_ACC_PRIVATE|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_state_class_Async_FutureState, 0, 0));
	zend_string_release(property_state_name);

	return class_entry;
}
