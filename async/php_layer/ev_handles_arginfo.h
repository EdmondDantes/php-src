/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 84140c85bc6a61e9804d68ee505f2bcb21e673a2 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_EvHandle___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_FiberHandle___construct arginfo_class_Async_EvHandle___construct

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FiberHandle_isStarted, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_FiberHandle_isSuspended arginfo_class_Async_FiberHandle_isStarted

#define arginfo_class_Async_FiberHandle_isRunning arginfo_class_Async_FiberHandle_isStarted

#define arginfo_class_Async_FiberHandle_isTerminated arginfo_class_Async_FiberHandle_isStarted

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FiberHandle_getContext, 1, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FiberHandle_cancelWith, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_FileHandle_fromResource, 0, 1, Async\\FileHandle, 0)
	ZEND_ARG_TYPE_INFO(0, fd, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, actions, IS_LONG, 0, "self::READABLE | self::WRITABLE")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_SocketHandle_fromResource, 0, 1, Async\\SocketHandle, 0)
	ZEND_ARG_TYPE_INFO(0, resource, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, actions, IS_LONG, 0, "self::READABLE | self::WRITABLE")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_SocketHandle_fromSocket, 0, 1, Async\\SocketHandle, 0)
	ZEND_ARG_TYPE_INFO(0, socket, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, actions, IS_LONG, 0, "self::READABLE | self::WRITABLE")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_TimerHandle_newTimeout, 0, 1, Async\\TimerHandle, 0)
	ZEND_ARG_TYPE_INFO(0, microseconds, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_TimerHandle_newInterval arginfo_class_Async_TimerHandle_newTimeout

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_SignalHandle_new, 0, 1, Async\\SignalHandle, 0)
	ZEND_ARG_TYPE_INFO(0, sigNumber, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_ThreadHandle___construct arginfo_class_Async_EvHandle___construct

#define arginfo_class_Async_ProcessHandle___construct arginfo_class_Async_EvHandle___construct

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_FileSystemHandle_fromPath, 0, 2, Async\\FileSystemHandle, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_FileSystemHandle___construct arginfo_class_Async_EvHandle___construct

ZEND_METHOD(Async_EvHandle, __construct);
ZEND_METHOD(Async_FiberHandle, __construct);
ZEND_METHOD(Async_FiberHandle, isStarted);
ZEND_METHOD(Async_FiberHandle, isSuspended);
ZEND_METHOD(Async_FiberHandle, isRunning);
ZEND_METHOD(Async_FiberHandle, isTerminated);
ZEND_METHOD(Async_FiberHandle, getContext);
ZEND_METHOD(Async_FiberHandle, cancelWith);
ZEND_METHOD(Async_FileHandle, fromResource);
ZEND_METHOD(Async_SocketHandle, fromResource);
ZEND_METHOD(Async_SocketHandle, fromSocket);
ZEND_METHOD(Async_TimerHandle, newTimeout);
ZEND_METHOD(Async_TimerHandle, newInterval);
ZEND_METHOD(Async_SignalHandle, new);
ZEND_METHOD(Async_ThreadHandle, __construct);
ZEND_METHOD(Async_ProcessHandle, __construct);
ZEND_METHOD(Async_FileSystemHandle, fromPath);
ZEND_METHOD(Async_FileSystemHandle, __construct);

static const zend_function_entry class_Async_EvHandle_methods[] = {
	ZEND_ME(Async_EvHandle, __construct, arginfo_class_Async_EvHandle___construct, ZEND_ACC_PRIVATE|ZEND_ACC_FINAL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_FiberHandle_methods[] = {
	ZEND_ME(Async_FiberHandle, __construct, arginfo_class_Async_FiberHandle___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, isStarted, arginfo_class_Async_FiberHandle_isStarted, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, isSuspended, arginfo_class_Async_FiberHandle_isSuspended, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, isRunning, arginfo_class_Async_FiberHandle_isRunning, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, isTerminated, arginfo_class_Async_FiberHandle_isTerminated, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, getContext, arginfo_class_Async_FiberHandle_getContext, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, cancelWith, arginfo_class_Async_FiberHandle_cancelWith, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_FileHandle_methods[] = {
	ZEND_ME(Async_FileHandle, fromResource, arginfo_class_Async_FileHandle_fromResource, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_SocketHandle_methods[] = {
	ZEND_ME(Async_SocketHandle, fromResource, arginfo_class_Async_SocketHandle_fromResource, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_SocketHandle, fromSocket, arginfo_class_Async_SocketHandle_fromSocket, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_TimerHandle_methods[] = {
	ZEND_ME(Async_TimerHandle, newTimeout, arginfo_class_Async_TimerHandle_newTimeout, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_TimerHandle, newInterval, arginfo_class_Async_TimerHandle_newInterval, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_SignalHandle_methods[] = {
	ZEND_ME(Async_SignalHandle, new, arginfo_class_Async_SignalHandle_new, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ThreadHandle_methods[] = {
	ZEND_ME(Async_ThreadHandle, __construct, arginfo_class_Async_ThreadHandle___construct, ZEND_ACC_PRIVATE)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ProcessHandle_methods[] = {
	ZEND_ME(Async_ProcessHandle, __construct, arginfo_class_Async_ProcessHandle___construct, ZEND_ACC_PRIVATE)
	ZEND_FE_END
};

static const zend_function_entry class_Async_FileSystemHandle_methods[] = {
	ZEND_ME(Async_FileSystemHandle, fromPath, arginfo_class_Async_FileSystemHandle_fromPath, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Async_FileSystemHandle, __construct, arginfo_class_Async_FileSystemHandle___construct, ZEND_ACC_PRIVATE)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_EvHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "EvHandle", class_Async_EvHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_ABSTRACT|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval const_READABLE_value;
	ZVAL_LONG(&const_READABLE_value, 1);
	zend_string *const_READABLE_name = zend_string_init_interned("READABLE", sizeof("READABLE") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_READABLE_name, &const_READABLE_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_READABLE_name);

	zval const_WRITABLE_value;
	ZVAL_LONG(&const_WRITABLE_value, 2);
	zend_string *const_WRITABLE_name = zend_string_init_interned("WRITABLE", sizeof("WRITABLE") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_WRITABLE_name, &const_WRITABLE_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_WRITABLE_name);

	zval const_DISCONNECT_value;
	ZVAL_LONG(&const_DISCONNECT_value, 4);
	zend_string *const_DISCONNECT_name = zend_string_init_interned("DISCONNECT", sizeof("DISCONNECT") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_DISCONNECT_name, &const_DISCONNECT_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_DISCONNECT_name);

	zval const_PRIORITY_value;
	ZVAL_LONG(&const_PRIORITY_value, 8);
	zend_string *const_PRIORITY_name = zend_string_init_interned("PRIORITY", sizeof("PRIORITY") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_PRIORITY_name, &const_PRIORITY_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_PRIORITY_name);

	zval property_triggeredEvents_default_value;
	ZVAL_LONG(&property_triggeredEvents_default_value, 0);
	zend_string *property_triggeredEvents_name = zend_string_init("triggeredEvents", sizeof("triggeredEvents") - 1, 1);
	zend_declare_typed_property(class_entry, property_triggeredEvents_name, &property_triggeredEvents_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_triggeredEvents_name);

	return class_entry;
}

static zend_class_entry *register_class_Async_FiberHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FiberHandle", class_Async_FiberHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_FileHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FileHandle", class_Async_FileHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_SocketHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "SocketHandle", class_Async_SocketHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_PipeHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "PipeHandle", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_TtyHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "TtyHandle", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_TimerHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "TimerHandle", class_Async_TimerHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval property_microseconds_default_value;
	ZVAL_LONG(&property_microseconds_default_value, 0);
	zend_string *property_microseconds_name = zend_string_init("microseconds", sizeof("microseconds") - 1, 1);
	zend_declare_typed_property(class_entry, property_microseconds_name, &property_microseconds_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_microseconds_name);

	zval property_isPeriodic_default_value;
	ZVAL_FALSE(&property_isPeriodic_default_value);
	zend_string *property_isPeriodic_name = zend_string_init("isPeriodic", sizeof("isPeriodic") - 1, 1);
	zend_declare_typed_property(class_entry, property_isPeriodic_name, &property_isPeriodic_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_BOOL));
	zend_string_release(property_isPeriodic_name);

	return class_entry;
}

static zend_class_entry *register_class_Async_SignalHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "SignalHandle", class_Async_SignalHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval property_sigNumber_default_value;
	ZVAL_LONG(&property_sigNumber_default_value, 0);
	zend_string *property_sigNumber_name = zend_string_init("sigNumber", sizeof("sigNumber") - 1, 1);
	zend_declare_typed_property(class_entry, property_sigNumber_name, &property_sigNumber_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_sigNumber_name);

	return class_entry;
}

static zend_class_entry *register_class_Async_ThreadHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ThreadHandle", class_Async_ThreadHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_ProcessHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ProcessHandle", class_Async_ProcessHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_FileSystemHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FileSystemHandle", class_Async_FileSystemHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	zval const_EVENT_RENAME_value;
	ZVAL_LONG(&const_EVENT_RENAME_value, 1);
	zend_string *const_EVENT_RENAME_name = zend_string_init_interned("EVENT_RENAME", sizeof("EVENT_RENAME") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_EVENT_RENAME_name, &const_EVENT_RENAME_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_EVENT_RENAME_name);

	zval const_EVENT_CHANGE_value;
	ZVAL_LONG(&const_EVENT_CHANGE_value, 2);
	zend_string *const_EVENT_CHANGE_name = zend_string_init_interned("EVENT_CHANGE", sizeof("EVENT_CHANGE") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_EVENT_CHANGE_name, &const_EVENT_CHANGE_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_EVENT_CHANGE_name);

	zval const_FLAG_NONE_value;
	ZVAL_LONG(&const_FLAG_NONE_value, 0);
	zend_string *const_FLAG_NONE_name = zend_string_init_interned("FLAG_NONE", sizeof("FLAG_NONE") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_FLAG_NONE_name, &const_FLAG_NONE_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_FLAG_NONE_name);

	zval const_WATCH_ENTRY_value;
	ZVAL_LONG(&const_WATCH_ENTRY_value, 1);
	zend_string *const_WATCH_ENTRY_name = zend_string_init_interned("WATCH_ENTRY", sizeof("WATCH_ENTRY") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_WATCH_ENTRY_name, &const_WATCH_ENTRY_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_WATCH_ENTRY_name);

	zval const_WATCH_RECURSIVE_value;
	ZVAL_LONG(&const_WATCH_RECURSIVE_value, 4);
	zend_string *const_WATCH_RECURSIVE_name = zend_string_init_interned("WATCH_RECURSIVE", sizeof("WATCH_RECURSIVE") - 1, 1);
	zend_declare_typed_class_constant(class_entry, const_WATCH_RECURSIVE_name, &const_WATCH_RECURSIVE_value, ZEND_ACC_PUBLIC, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(const_WATCH_RECURSIVE_name);

	zval property_triggeredEvents_default_value;
	ZVAL_LONG(&property_triggeredEvents_default_value, 0);
	zend_string *property_triggeredEvents_name = zend_string_init("triggeredEvents", sizeof("triggeredEvents") - 1, 1);
	zend_declare_typed_property(class_entry, property_triggeredEvents_name, &property_triggeredEvents_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_triggeredEvents_name);

	zval property_path_default_value;
	ZVAL_EMPTY_STRING(&property_path_default_value);
	zend_declare_typed_property(class_entry, ZSTR_KNOWN(ZEND_STR_PATH), &property_path_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_STRING));

	zval property_flags_default_value;
	ZVAL_LONG(&property_flags_default_value, 0);
	zend_string *property_flags_name = zend_string_init("flags", sizeof("flags") - 1, 1);
	zend_declare_typed_property(class_entry, property_flags_name, &property_flags_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_LONG));
	zend_string_release(property_flags_name);

	return class_entry;
}
