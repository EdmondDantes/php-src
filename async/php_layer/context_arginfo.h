/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 09ea388a783b93dd67bb882435519ed981ad376b */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Key___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, description, IS_STRING, 0, "\'\'")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Key___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Context_newCurrent, 0, 0, Async\\Context, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Context_current, 0, 0, Async\\Context, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, createIfNull, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Context_overrideCurrent, 0, 0, Async\\Context, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, weakParent, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Context_local arginfo_class_Async_Context_newCurrent

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Context___construct, 0, 0, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, parent, Async\\Context, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, weakParent, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Context_find, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_OBJECT, NULL)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Context_get arginfo_class_Async_Context_find

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Context_has, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_OBJECT, NULL)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Context_findLocal arginfo_class_Async_Context_find

#define arginfo_class_Async_Context_getLocal arginfo_class_Async_Context_find

#define arginfo_class_Async_Context_hasLocal arginfo_class_Async_Context_has

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Context_setKey, 0, 2, Async\\Context, 0)
	ZEND_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_OBJECT, NULL)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, replace, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Context_delKey, 0, 1, Async\\Context, 0)
	ZEND_ARG_TYPE_MASK(0, key, MAY_BE_STRING|MAY_BE_OBJECT, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_Context_getParent, 0, 0, Async\\Context, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Context_isEmpty, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Async_Key, __construct);
ZEND_METHOD(Async_Key, __toString);
ZEND_METHOD(Async_Context, newCurrent);
ZEND_METHOD(Async_Context, current);
ZEND_METHOD(Async_Context, overrideCurrent);
ZEND_METHOD(Async_Context, local);
ZEND_METHOD(Async_Context, __construct);
ZEND_METHOD(Async_Context, find);
ZEND_METHOD(Async_Context, get);
ZEND_METHOD(Async_Context, has);
ZEND_METHOD(Async_Context, findLocal);
ZEND_METHOD(Async_Context, getLocal);
ZEND_METHOD(Async_Context, hasLocal);
ZEND_METHOD(Async_Context, setKey);
ZEND_METHOD(Async_Context, delKey);
ZEND_METHOD(Async_Context, getParent);
ZEND_METHOD(Async_Context, isEmpty);

static const zend_function_entry class_Async_Key_methods[] = {
	ZEND_ME(Async_Key, __construct, arginfo_class_Async_Key___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Key, __toString, arginfo_class_Async_Key___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_Context_methods[] = {
	ZEND_ME(Async_Context, newCurrent, arginfo_class_Async_Context_newCurrent, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_Context, current, arginfo_class_Async_Context_current, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_Context, overrideCurrent, arginfo_class_Async_Context_overrideCurrent, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_Context, local, arginfo_class_Async_Context_local, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_Context, __construct, arginfo_class_Async_Context___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, find, arginfo_class_Async_Context_find, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, get, arginfo_class_Async_Context_get, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, has, arginfo_class_Async_Context_has, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, findLocal, arginfo_class_Async_Context_findLocal, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, getLocal, arginfo_class_Async_Context_getLocal, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, hasLocal, arginfo_class_Async_Context_hasLocal, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, setKey, arginfo_class_Async_Context_setKey, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, delKey, arginfo_class_Async_Context_delKey, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, getParent, arginfo_class_Async_Context_getParent, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Context, isEmpty, arginfo_class_Async_Context_isEmpty, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Key(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Key", class_Async_Key_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval property_description_default_value;
	ZVAL_UNDEF(&property_description_default_value);
	zend_string *property_description_name = zend_string_init("description", sizeof("description") - 1, 1);
	zend_declare_typed_property(class_entry, property_description_name, &property_description_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_STRING));
	zend_string_release(property_description_name);

	return class_entry;
}

static zend_class_entry *register_class_Async_Context(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Context", class_Async_Context_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_ContextException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ContextException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, ZEND_ACC_FINAL);

	return class_entry;
}
