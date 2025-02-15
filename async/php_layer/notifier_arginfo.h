/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5876d08e97b5df7936406a1c77b493b28446bc87 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_addCallback, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Async\\Callback, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Notifier_removeCallback arginfo_class_Async_Notifier_addCallback

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_notify, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, error, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier_terminate, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Notifier___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Notifier, addCallback);
ZEND_METHOD(Async_Notifier, removeCallback);
ZEND_METHOD(Async_Notifier, notify);
ZEND_METHOD(Async_Notifier, terminate);
ZEND_METHOD(Async_Notifier, __toString);

static const zend_function_entry class_Async_Notifier_methods[] = {
	ZEND_ME(Async_Notifier, addCallback, arginfo_class_Async_Notifier_addCallback, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Notifier, removeCallback, arginfo_class_Async_Notifier_removeCallback, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Notifier, notify, arginfo_class_Async_Notifier_notify, ZEND_ACC_PROTECTED|ZEND_ACC_FINAL)
	ZEND_ME(Async_Notifier, terminate, arginfo_class_Async_Notifier_terminate, ZEND_ACC_PROTECTED|ZEND_ACC_FINAL)
	ZEND_ME(Async_Notifier, __toString, arginfo_class_Async_Notifier___toString, ZEND_ACC_PUBLIC)
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

	zval property_isTerminated_default_value;
	ZVAL_FALSE(&property_isTerminated_default_value);
	zend_string *property_isTerminated_name = zend_string_init("isTerminated", sizeof("isTerminated") - 1, 1);
	zend_declare_typed_property(class_entry, property_isTerminated_name, &property_isTerminated_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_isTerminated_name);

	zval property_toString_default_value;
	ZVAL_UNDEF(&property_toString_default_value);
	zend_string *property_toString_name = zend_string_init("toString", sizeof("toString") - 1, 1);
	zend_declare_typed_property(class_entry, property_toString_name, &property_toString_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ANY));
	zend_string_release(property_toString_name);

	zval property_reserved_default_value;
	ZVAL_LONG(&property_reserved_default_value, 0);
	zend_string *property_reserved_name = zend_string_init("reserved", sizeof("reserved") - 1, 1);
	zend_declare_typed_property(class_entry, property_reserved_name, &property_reserved_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_reserved_name);

	return class_entry;
}
