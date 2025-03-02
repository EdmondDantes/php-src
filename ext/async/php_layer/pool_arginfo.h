/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b4d16d34b5b0727f15519661d7d58f1f98ed13ae */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Pool___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, factory, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO(0, maxPoolSize, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, minPoolSize, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "-1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, delayPoolReduction, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Pool_borrow, 0, 0, IS_OBJECT, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Pool_return, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, object, IS_OBJECT, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Pool_rebuild, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Pool_getMaxPoolSize, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Pool_getMinPoolSize arginfo_class_Async_Pool_getMaxPoolSize

#define arginfo_class_Async_Pool_getMaxWaitTimeout arginfo_class_Async_Pool_getMaxPoolSize

#define arginfo_class_Async_Pool_getUsed arginfo_class_Async_Pool_getMaxPoolSize

#define arginfo_class_Async_Pool_getPoolSize arginfo_class_Async_Pool_getMaxPoolSize

ZEND_METHOD(Async_Pool, __construct);
ZEND_METHOD(Async_Pool, borrow);
ZEND_METHOD(Async_Pool, return);
ZEND_METHOD(Async_Pool, rebuild);
ZEND_METHOD(Async_Pool, getMaxPoolSize);
ZEND_METHOD(Async_Pool, getMinPoolSize);
ZEND_METHOD(Async_Pool, getMaxWaitTimeout);
ZEND_METHOD(Async_Pool, getUsed);
ZEND_METHOD(Async_Pool, getPoolSize);

static const zend_function_entry class_Async_Pool_methods[] = {
	ZEND_ME(Async_Pool, __construct, arginfo_class_Async_Pool___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, borrow, arginfo_class_Async_Pool_borrow, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, return, arginfo_class_Async_Pool_return, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, rebuild, arginfo_class_Async_Pool_rebuild, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, getMaxPoolSize, arginfo_class_Async_Pool_getMaxPoolSize, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, getMinPoolSize, arginfo_class_Async_Pool_getMinPoolSize, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, getMaxWaitTimeout, arginfo_class_Async_Pool_getMaxWaitTimeout, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, getUsed, arginfo_class_Async_Pool_getUsed, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Pool, getPoolSize, arginfo_class_Async_Pool_getPoolSize, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Pool(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Pool", class_Async_Pool_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);

	zval property_factory_default_value;
	ZVAL_UNDEF(&property_factory_default_value);
	zend_string *property_factory_name = zend_string_init("factory", sizeof("factory") - 1, 1);
	zend_declare_typed_property(class_entry, property_factory_name, &property_factory_default_value, ZEND_ACC_PROTECTED, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_CALLABLE));
	zend_string_release(property_factory_name);

	zval property_maxPoolSize_default_value;
	ZVAL_UNDEF(&property_maxPoolSize_default_value);
	zend_string *property_maxPoolSize_name = zend_string_init("maxPoolSize", sizeof("maxPoolSize") - 1, 1);
	zend_declare_typed_property(class_entry, property_maxPoolSize_name, &property_maxPoolSize_default_value, ZEND_ACC_PROTECTED, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_maxPoolSize_name);

	zval property_minPoolSize_default_value;
	ZVAL_LONG(&property_minPoolSize_default_value, 0);
	zend_string *property_minPoolSize_name = zend_string_init("minPoolSize", sizeof("minPoolSize") - 1, 1);
	zend_declare_typed_property(class_entry, property_minPoolSize_name, &property_minPoolSize_default_value, ZEND_ACC_PROTECTED, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_minPoolSize_name);

	zval property_timeout_default_value;
	ZVAL_LONG(&property_timeout_default_value, -1);
	zend_string *property_timeout_name = zend_string_init("timeout", sizeof("timeout") - 1, 1);
	zend_declare_typed_property(class_entry, property_timeout_name, &property_timeout_default_value, ZEND_ACC_PROTECTED, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_timeout_name);

	zval property_delayPoolReduction_default_value;
	ZVAL_LONG(&property_delayPoolReduction_default_value, 0);
	zend_string *property_delayPoolReduction_name = zend_string_init("delayPoolReduction", sizeof("delayPoolReduction") - 1, 1);
	zend_declare_typed_property(class_entry, property_delayPoolReduction_name, &property_delayPoolReduction_default_value, ZEND_ACC_PROTECTED, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_delayPoolReduction_name);

	return class_entry;
}
