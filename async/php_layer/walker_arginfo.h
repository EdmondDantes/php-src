/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5445f211924e38856705fe6761544d2b8ab9e2f4 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Walker_walk, 0, 2, IS_VOID, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, iterator, Traversable, MAY_BE_ARRAY, NULL)
	ZEND_ARG_TYPE_INFO(0, function, IS_CALLABLE, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, customData, IS_MIXED, 0, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, defer, IS_CALLABLE, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Walker_run, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Walker_next arginfo_class_Async_Walker_run

#define arginfo_class_Async_Walker_cancel arginfo_class_Async_Walker_run

ZEND_METHOD(Async_Walker, walk);
ZEND_METHOD(Async_Walker, run);
ZEND_METHOD(Async_Walker, next);
ZEND_METHOD(Async_Walker, cancel);

static const zend_function_entry class_Async_Walker_methods[] = {
	ZEND_ME(Async_Walker, walk, arginfo_class_Async_Walker_walk, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_Walker, run, arginfo_class_Async_Walker_run, ZEND_ACC_PRIVATE)
	ZEND_ME(Async_Walker, next, arginfo_class_Async_Walker_next, ZEND_ACC_PRIVATE)
	ZEND_ME(Async_Walker, cancel, arginfo_class_Async_Walker_cancel, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Walker(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Walker", class_Async_Walker_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval property_isFinished_default_value;
	ZVAL_FALSE(&property_isFinished_default_value);
	zend_string *property_isFinished_name = zend_string_init("isFinished", sizeof("isFinished") - 1, 1);
	zend_declare_typed_property(class_entry, property_isFinished_name, &property_isFinished_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_isFinished_name);

	zval property_iterator_default_value;
	ZVAL_UNDEF(&property_iterator_default_value);
	zend_string *property_iterator_name = zend_string_init("iterator", sizeof("iterator") - 1, 1);
	zend_string *property_iterator_class_Traversable = zend_string_init("Traversable", sizeof("Traversable")-1, 1);
	zend_declare_typed_property(class_entry, property_iterator_name, &property_iterator_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_iterator_class_Traversable, 0, MAY_BE_ARRAY));
	zend_string_release(property_iterator_name);

	zval property_customData_default_value;
	ZVAL_UNDEF(&property_customData_default_value);
	zend_string *property_customData_name = zend_string_init("customData", sizeof("customData") - 1, 1);
	zend_declare_typed_property(class_entry, property_customData_name, &property_customData_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ANY));
	zend_string_release(property_customData_name);

	zval property_defer_default_value;
	ZVAL_UNDEF(&property_defer_default_value);
	zend_string *property_defer_name = zend_string_init("defer", sizeof("defer") - 1, 1);
	zend_declare_typed_property(class_entry, property_defer_name, &property_defer_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ANY));
	zend_string_release(property_defer_name);

	return class_entry;
}
