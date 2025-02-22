/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9c2259301b6e4499e843ce17da4d4c513aace6b8 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Closure___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Closure_disposeClosure, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Closure, __construct);
ZEND_METHOD(Async_Closure, disposeClosure);

static const zend_function_entry class_Async_Closure_methods[] = {
	ZEND_ME(Async_Closure, __construct, arginfo_class_Async_Closure___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Closure, disposeClosure, arginfo_class_Async_Closure_disposeClosure, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Closure(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Closure", class_Async_Closure_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}
