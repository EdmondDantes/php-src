/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: eefea6d88f8340e341c7d94433ad8f38c6a0d74b */

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

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_InputOutputHandle___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, handle, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_TimerHandle___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, microseconds, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_SignalHandle___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, sigNumber, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_ThreadHandle___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, threadId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_ProcessHandle___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, processId, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_FileSystemHandle___construct arginfo_class_Async_ProcessHandle___construct

ZEND_METHOD(Async_FiberHandle, isStarted);
ZEND_METHOD(Async_FiberHandle, isSuspended);
ZEND_METHOD(Async_FiberHandle, isRunning);
ZEND_METHOD(Async_FiberHandle, isTerminated);
ZEND_METHOD(Async_FiberHandle, getContext);
ZEND_METHOD(Async_FiberHandle, cancelWith);
ZEND_METHOD(Async_InputOutputHandle, __construct);
ZEND_METHOD(Async_TimerHandle, __construct);
ZEND_METHOD(Async_SignalHandle, __construct);
ZEND_METHOD(Async_ThreadHandle, __construct);
ZEND_METHOD(Async_ProcessHandle, __construct);
ZEND_METHOD(Async_FileSystemHandle, __construct);

static const zend_function_entry class_Async_FiberHandle_methods[] = {
	ZEND_ME(Async_FiberHandle, isStarted, arginfo_class_Async_FiberHandle_isStarted, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, isSuspended, arginfo_class_Async_FiberHandle_isSuspended, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, isRunning, arginfo_class_Async_FiberHandle_isRunning, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, isTerminated, arginfo_class_Async_FiberHandle_isTerminated, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, getContext, arginfo_class_Async_FiberHandle_getContext, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FiberHandle, cancelWith, arginfo_class_Async_FiberHandle_cancelWith, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_InputOutputHandle_methods[] = {
	ZEND_ME(Async_InputOutputHandle, __construct, arginfo_class_Async_InputOutputHandle___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_TimerHandle_methods[] = {
	ZEND_ME(Async_TimerHandle, __construct, arginfo_class_Async_TimerHandle___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_SignalHandle_methods[] = {
	ZEND_ME(Async_SignalHandle, __construct, arginfo_class_Async_SignalHandle___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ThreadHandle_methods[] = {
	ZEND_ME(Async_ThreadHandle, __construct, arginfo_class_Async_ThreadHandle___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ProcessHandle_methods[] = {
	ZEND_ME(Async_ProcessHandle, __construct, arginfo_class_Async_ProcessHandle___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_FileSystemHandle_methods[] = {
	ZEND_ME(Async_FileSystemHandle, __construct, arginfo_class_Async_FileSystemHandle___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_EvHandle(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "EvHandle", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_ABSTRACT);
	zend_class_implements(class_entry, 1, class_entry_Async_Notifier);

	return class_entry;
}

static zend_class_entry *register_class_Async_FiberHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FiberHandle", class_Async_FiberHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_InputOutputHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "InputOutputHandle", class_Async_InputOutputHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_TimerHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "TimerHandle", class_Async_TimerHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_SignalHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "SignalHandle", class_Async_SignalHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_ThreadHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ThreadHandle", class_Async_ThreadHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_ProcessHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ProcessHandle", class_Async_ProcessHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_FileSystemHandle(zend_class_entry *class_entry_Async_EvHandle)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FileSystemHandle", class_Async_FileSystemHandle_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_EvHandle, ZEND_ACC_FINAL);

	return class_entry;
}
