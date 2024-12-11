/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: c1ddb49b28f9b2423a1e61405f6328e80e16d2c4 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_await, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, awaitable, Async\\AwaitableInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_async, 0, 1, Async\\FutureInterface, 0)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_defer, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, task, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_delay, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, task, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_repeat, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, interval, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, task, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_onSignal, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, sigNumber, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, task, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_FutureInterface_getStatus, 0, 0, Async\\FutureStatus, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FutureInterface_isReady, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_FutureInterface_isFailed arginfo_class_Async_FutureInterface_isReady

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FutureInterface_getResult, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_FutureInterface_getError, 0, 0, Throwable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CancellationInterface_cancel, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_EventDescriptorInterface_onRegistered, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, disposeCallback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_AwaitableInterface_getDeferredResume, 0, 0, Async\\DeferredResume, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_AwaitableInterface_getWaitingEvents, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CompletionPublisherInterface_onSuccess, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, onFulfilled, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CompletionPublisherInterface_onError, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, onRejected, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CompletionPublisherInterface_onFinally, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, onFinally, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ThenInterface_thenIgnore, 0, 0, IS_STATIC, 0)
	ZEND_ARG_VARIADIC_OBJ_INFO(0, objects, Async\\CancellationInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ThenInterface_thenResolve, 0, 0, IS_STATIC, 0)
	ZEND_ARG_VARIADIC_OBJ_INFO(0, objects, Async\\DeferredInterface, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_ThenInterface_thenReject arginfo_class_Async_ThenInterface_thenResolve

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_DeferredInterface_resolve, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_DeferredInterface_reject, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, error, Throwable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_DeferredInterface_getFuture, 0, 0, Async\\FutureInterface, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_CompletionPublisherAbstract_onSuccess arginfo_class_Async_CompletionPublisherInterface_onSuccess

#define arginfo_class_Async_CompletionPublisherAbstract_onError arginfo_class_Async_CompletionPublisherInterface_onError

#define arginfo_class_Async_CompletionPublisherAbstract_onFinally arginfo_class_Async_CompletionPublisherInterface_onFinally

#define arginfo_class_Async_CompletionPublisherAbstract_thenIgnore arginfo_class_Async_ThenInterface_thenIgnore

#define arginfo_class_Async_CompletionPublisherAbstract_thenResolve arginfo_class_Async_ThenInterface_thenResolve

#define arginfo_class_Async_CompletionPublisherAbstract_thenReject arginfo_class_Async_ThenInterface_thenResolve

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_CompletionPublisherAbstract_invokeCompletionHandlers, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, status, Async\\FutureStatus, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_DeferredAbstract_getStatus arginfo_class_Async_FutureInterface_getStatus

#define arginfo_class_Async_DeferredAbstract_isReady arginfo_class_Async_FutureInterface_isReady

#define arginfo_class_Async_DeferredAbstract_isFailed arginfo_class_Async_FutureInterface_isReady

#define arginfo_class_Async_DeferredAbstract_getResult arginfo_class_Async_FutureInterface_getResult

#define arginfo_class_Async_DeferredAbstract_getError arginfo_class_Async_FutureInterface_getError

#define arginfo_class_Async_DeferredAbstract_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_DeferredAbstract_resolve arginfo_class_Async_DeferredInterface_resolve

#define arginfo_class_Async_DeferredAbstract_reject arginfo_class_Async_DeferredInterface_reject

#define arginfo_class_Async_DeferredAbstract_getFuture arginfo_class_Async_DeferredInterface_getFuture

#define arginfo_class_Async_DeferredAbstract_internalHandler arginfo_class_Async_CancellationInterface_cancel

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_DeferredResume___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_DeferredResume_internalHandler arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_InputOutputEvent_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_InputOutputEvent_onRegistered arginfo_class_Async_EventDescriptorInterface_onRegistered

#define arginfo_class_Async_TimerEvent_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_TimerEvent_onRegistered arginfo_class_Async_EventDescriptorInterface_onRegistered

#define arginfo_class_Async_SignalEvent_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_SignalEvent_onRegistered arginfo_class_Async_EventDescriptorInterface_onRegistered

#define arginfo_class_Async_FileSystemEvent_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_FileSystemEvent_onRegistered arginfo_class_Async_EventDescriptorInterface_onRegistered

#define arginfo_class_Async_ProcessEvent_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_ProcessEvent_onRegistered arginfo_class_Async_EventDescriptorInterface_onRegistered

#define arginfo_class_Async_ThreadEvent_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_ThreadEvent_onRegistered arginfo_class_Async_EventDescriptorInterface_onRegistered

ZEND_FUNCTION(Async_await);
ZEND_FUNCTION(Async_async);
ZEND_FUNCTION(Async_defer);
ZEND_FUNCTION(Async_delay);
ZEND_FUNCTION(Async_repeat);
ZEND_FUNCTION(Async_onSignal);
ZEND_METHOD(Async_CompletionPublisherAbstract, onSuccess);
ZEND_METHOD(Async_CompletionPublisherAbstract, onError);
ZEND_METHOD(Async_CompletionPublisherAbstract, onFinally);
ZEND_METHOD(Async_CompletionPublisherAbstract, thenIgnore);
ZEND_METHOD(Async_CompletionPublisherAbstract, thenResolve);
ZEND_METHOD(Async_CompletionPublisherAbstract, thenReject);
ZEND_METHOD(Async_CompletionPublisherAbstract, invokeCompletionHandlers);
ZEND_METHOD(Async_DeferredAbstract, getStatus);
ZEND_METHOD(Async_DeferredAbstract, isReady);
ZEND_METHOD(Async_DeferredAbstract, isFailed);
ZEND_METHOD(Async_DeferredAbstract, getResult);
ZEND_METHOD(Async_DeferredAbstract, getError);
ZEND_METHOD(Async_DeferredAbstract, cancel);
ZEND_METHOD(Async_DeferredAbstract, resolve);
ZEND_METHOD(Async_DeferredAbstract, reject);
ZEND_METHOD(Async_DeferredAbstract, getFuture);
ZEND_METHOD(Async_DeferredResume, __construct);
ZEND_METHOD(Async_DeferredResume, internalHandler);
ZEND_METHOD(Async_InputOutputEvent, cancel);
ZEND_METHOD(Async_InputOutputEvent, onRegistered);
ZEND_METHOD(Async_TimerEvent, cancel);
ZEND_METHOD(Async_TimerEvent, onRegistered);
ZEND_METHOD(Async_SignalEvent, cancel);
ZEND_METHOD(Async_SignalEvent, onRegistered);
ZEND_METHOD(Async_FileSystemEvent, cancel);
ZEND_METHOD(Async_FileSystemEvent, onRegistered);
ZEND_METHOD(Async_ProcessEvent, cancel);
ZEND_METHOD(Async_ProcessEvent, onRegistered);
ZEND_METHOD(Async_ThreadEvent, cancel);
ZEND_METHOD(Async_ThreadEvent, onRegistered);

static const zend_function_entry ext_functions[] = {
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "await"), zif_Async_await, arginfo_Async_await, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "async"), zif_Async_async, arginfo_Async_async, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "defer"), zif_Async_defer, arginfo_Async_defer, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "delay"), zif_Async_delay, arginfo_Async_delay, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "repeat"), zif_Async_repeat, arginfo_Async_repeat, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "onSignal"), zif_Async_onSignal, arginfo_Async_onSignal, 0, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_FutureInterface_methods[] = {
	ZEND_RAW_FENTRY("getStatus", NULL, arginfo_class_Async_FutureInterface_getStatus, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("isReady", NULL, arginfo_class_Async_FutureInterface_isReady, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("isFailed", NULL, arginfo_class_Async_FutureInterface_isFailed, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("getResult", NULL, arginfo_class_Async_FutureInterface_getResult, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("getError", NULL, arginfo_class_Async_FutureInterface_getError, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_CancellationInterface_methods[] = {
	ZEND_RAW_FENTRY("cancel", NULL, arginfo_class_Async_CancellationInterface_cancel, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_EventDescriptorInterface_methods[] = {
	ZEND_RAW_FENTRY("onRegistered", NULL, arginfo_class_Async_EventDescriptorInterface_onRegistered, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_AwaitableInterface_methods[] = {
	ZEND_RAW_FENTRY("getDeferredResume", NULL, arginfo_class_Async_AwaitableInterface_getDeferredResume, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("getWaitingEvents", NULL, arginfo_class_Async_AwaitableInterface_getWaitingEvents, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_CompletionPublisherInterface_methods[] = {
	ZEND_RAW_FENTRY("onSuccess", NULL, arginfo_class_Async_CompletionPublisherInterface_onSuccess, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("onError", NULL, arginfo_class_Async_CompletionPublisherInterface_onError, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("onFinally", NULL, arginfo_class_Async_CompletionPublisherInterface_onFinally, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ThenInterface_methods[] = {
	ZEND_RAW_FENTRY("thenIgnore", NULL, arginfo_class_Async_ThenInterface_thenIgnore, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("thenResolve", NULL, arginfo_class_Async_ThenInterface_thenResolve, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("thenReject", NULL, arginfo_class_Async_ThenInterface_thenReject, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_DeferredInterface_methods[] = {
	ZEND_RAW_FENTRY("resolve", NULL, arginfo_class_Async_DeferredInterface_resolve, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("reject", NULL, arginfo_class_Async_DeferredInterface_reject, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("getFuture", NULL, arginfo_class_Async_DeferredInterface_getFuture, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_CompletionPublisherAbstract_methods[] = {
	ZEND_ME(Async_CompletionPublisherAbstract, onSuccess, arginfo_class_Async_CompletionPublisherAbstract_onSuccess, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_CompletionPublisherAbstract, onError, arginfo_class_Async_CompletionPublisherAbstract_onError, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_CompletionPublisherAbstract, onFinally, arginfo_class_Async_CompletionPublisherAbstract_onFinally, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_CompletionPublisherAbstract, thenIgnore, arginfo_class_Async_CompletionPublisherAbstract_thenIgnore, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_CompletionPublisherAbstract, thenResolve, arginfo_class_Async_CompletionPublisherAbstract_thenResolve, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_CompletionPublisherAbstract, thenReject, arginfo_class_Async_CompletionPublisherAbstract_thenReject, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_CompletionPublisherAbstract, invokeCompletionHandlers, arginfo_class_Async_CompletionPublisherAbstract_invokeCompletionHandlers, ZEND_ACC_PROTECTED|ZEND_ACC_FINAL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_DeferredAbstract_methods[] = {
	ZEND_ME(Async_DeferredAbstract, getStatus, arginfo_class_Async_DeferredAbstract_getStatus, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, isReady, arginfo_class_Async_DeferredAbstract_isReady, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, isFailed, arginfo_class_Async_DeferredAbstract_isFailed, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, getResult, arginfo_class_Async_DeferredAbstract_getResult, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, getError, arginfo_class_Async_DeferredAbstract_getError, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, cancel, arginfo_class_Async_DeferredAbstract_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, resolve, arginfo_class_Async_DeferredAbstract_resolve, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, reject, arginfo_class_Async_DeferredAbstract_reject, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, getFuture, arginfo_class_Async_DeferredAbstract_getFuture, ZEND_ACC_PUBLIC)
	ZEND_RAW_FENTRY("internalHandler", NULL, arginfo_class_Async_DeferredAbstract_internalHandler, ZEND_ACC_PROTECTED|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_DeferredResume_methods[] = {
	ZEND_ME(Async_DeferredResume, __construct, arginfo_class_Async_DeferredResume___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredResume, internalHandler, arginfo_class_Async_DeferredResume_internalHandler, ZEND_ACC_PROTECTED)
	ZEND_FE_END
};

static const zend_function_entry class_Async_InputOutputEvent_methods[] = {
	ZEND_ME(Async_InputOutputEvent, cancel, arginfo_class_Async_InputOutputEvent_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_InputOutputEvent, onRegistered, arginfo_class_Async_InputOutputEvent_onRegistered, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_TimerEvent_methods[] = {
	ZEND_ME(Async_TimerEvent, cancel, arginfo_class_Async_TimerEvent_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_TimerEvent, onRegistered, arginfo_class_Async_TimerEvent_onRegistered, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_SignalEvent_methods[] = {
	ZEND_ME(Async_SignalEvent, cancel, arginfo_class_Async_SignalEvent_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_SignalEvent, onRegistered, arginfo_class_Async_SignalEvent_onRegistered, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_FileSystemEvent_methods[] = {
	ZEND_ME(Async_FileSystemEvent, cancel, arginfo_class_Async_FileSystemEvent_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_FileSystemEvent, onRegistered, arginfo_class_Async_FileSystemEvent_onRegistered, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ProcessEvent_methods[] = {
	ZEND_ME(Async_ProcessEvent, cancel, arginfo_class_Async_ProcessEvent_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_ProcessEvent, onRegistered, arginfo_class_Async_ProcessEvent_onRegistered, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ThreadEvent_methods[] = {
	ZEND_ME(Async_ThreadEvent, cancel, arginfo_class_Async_ThreadEvent_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_ThreadEvent, onRegistered, arginfo_class_Async_ThreadEvent_onRegistered, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_FutureStatus(void)
{
	zend_class_entry *class_entry = zend_register_internal_enum("Async\\FutureStatus", IS_LONG, NULL);

	zval enum_case_PENDING_value;
	ZVAL_LONG(&enum_case_PENDING_value, 0);
	zend_enum_add_case_cstr(class_entry, "PENDING", &enum_case_PENDING_value);

	zval enum_case_RESOLVED_value;
	ZVAL_LONG(&enum_case_RESOLVED_value, 1);
	zend_enum_add_case_cstr(class_entry, "RESOLVED", &enum_case_RESOLVED_value);

	zval enum_case_REJECTED_value;
	ZVAL_LONG(&enum_case_REJECTED_value, 2);
	zend_enum_add_case_cstr(class_entry, "REJECTED", &enum_case_REJECTED_value);

	zval enum_case_CANCELLED_value;
	ZVAL_LONG(&enum_case_CANCELLED_value, 3);
	zend_enum_add_case_cstr(class_entry, "CANCELLED", &enum_case_CANCELLED_value);

	return class_entry;
}

static zend_class_entry *register_class_Async_FutureInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FutureInterface", class_Async_FutureInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_CancellationInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "CancellationInterface", class_Async_CancellationInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_EventDescriptorInterface(zend_class_entry *class_entry_Async_CancellationInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "EventDescriptorInterface", class_Async_EventDescriptorInterface_methods);
	class_entry = zend_register_internal_interface(&ce);
	zend_class_implements(class_entry, 1, class_entry_Async_CancellationInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_AwaitableInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "AwaitableInterface", class_Async_AwaitableInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_CompletionPublisherInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "CompletionPublisherInterface", class_Async_CompletionPublisherInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_ThenInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ThenInterface", class_Async_ThenInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_DeferredInterface(zend_class_entry *class_entry_Async_FutureInterface, zend_class_entry *class_entry_Async_CancellationInterface, zend_class_entry *class_entry_Async_CompletionPublisherInterface, zend_class_entry *class_entry_Async_ThenInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "DeferredInterface", class_Async_DeferredInterface_methods);
	class_entry = zend_register_internal_interface(&ce);
	zend_class_implements(class_entry, 4, class_entry_Async_FutureInterface, class_entry_Async_CancellationInterface, class_entry_Async_CompletionPublisherInterface, class_entry_Async_ThenInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_CompletionPublisherAbstract(zend_class_entry *class_entry_Async_CompletionPublisherInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "CompletionPublisherAbstract", class_Async_CompletionPublisherAbstract_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_ABSTRACT);
	zend_class_implements(class_entry, 1, class_entry_Async_CompletionPublisherInterface);

	zval property_onFulfilled_default_value;
	ZVAL_EMPTY_ARRAY(&property_onFulfilled_default_value);
	zend_string *property_onFulfilled_name = zend_string_init("onFulfilled", sizeof("onFulfilled") - 1, 1);
	zend_declare_typed_property(class_entry, property_onFulfilled_name, &property_onFulfilled_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
	zend_string_release(property_onFulfilled_name);

	zval property_onRejected_default_value;
	ZVAL_EMPTY_ARRAY(&property_onRejected_default_value);
	zend_string *property_onRejected_name = zend_string_init("onRejected", sizeof("onRejected") - 1, 1);
	zend_declare_typed_property(class_entry, property_onRejected_name, &property_onRejected_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
	zend_string_release(property_onRejected_name);

	zval property_onFinally_default_value;
	ZVAL_EMPTY_ARRAY(&property_onFinally_default_value);
	zend_string *property_onFinally_name = zend_string_init("onFinally", sizeof("onFinally") - 1, 1);
	zend_declare_typed_property(class_entry, property_onFinally_name, &property_onFinally_default_value, ZEND_ACC_PRIVATE, NULL, (zend_type) ZEND_TYPE_INIT_MASK(MAY_BE_ARRAY));
	zend_string_release(property_onFinally_name);

	return class_entry;
}

static zend_class_entry *register_class_Async_DeferredAbstract(zend_class_entry *class_entry_Async_CompletionPublisherAbstract, zend_class_entry *class_entry_Async_DeferredInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "DeferredAbstract", class_Async_DeferredAbstract_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_CompletionPublisherAbstract, ZEND_ACC_ABSTRACT);
	zend_class_implements(class_entry, 1, class_entry_Async_DeferredInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_DeferredResume(zend_class_entry *class_entry_Async_DeferredAbstract)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "DeferredResume", class_Async_DeferredResume_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_DeferredAbstract, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_InputOutputEvent(zend_class_entry *class_entry_Async_CompletionPublisherAbstract, zend_class_entry *class_entry_Async_EventDescriptorInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "InputOutputEvent", class_Async_InputOutputEvent_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_CompletionPublisherAbstract, 0);
	zend_class_implements(class_entry, 1, class_entry_Async_EventDescriptorInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_TimerEvent(zend_class_entry *class_entry_Async_CompletionPublisherAbstract, zend_class_entry *class_entry_Async_EventDescriptorInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "TimerEvent", class_Async_TimerEvent_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_CompletionPublisherAbstract, ZEND_ACC_FINAL);
	zend_class_implements(class_entry, 1, class_entry_Async_EventDescriptorInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_SignalEvent(zend_class_entry *class_entry_Async_CompletionPublisherAbstract, zend_class_entry *class_entry_Async_EventDescriptorInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "SignalEvent", class_Async_SignalEvent_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_CompletionPublisherAbstract, ZEND_ACC_FINAL);
	zend_class_implements(class_entry, 1, class_entry_Async_EventDescriptorInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_FileSystemEvent(zend_class_entry *class_entry_Async_CompletionPublisherAbstract, zend_class_entry *class_entry_Async_EventDescriptorInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "FileSystemEvent", class_Async_FileSystemEvent_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_CompletionPublisherAbstract, ZEND_ACC_FINAL);
	zend_class_implements(class_entry, 1, class_entry_Async_EventDescriptorInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_ProcessEvent(zend_class_entry *class_entry_Async_CompletionPublisherAbstract, zend_class_entry *class_entry_Async_EventDescriptorInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ProcessEvent", class_Async_ProcessEvent_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_CompletionPublisherAbstract, ZEND_ACC_FINAL);
	zend_class_implements(class_entry, 1, class_entry_Async_EventDescriptorInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_ThreadEvent(zend_class_entry *class_entry_Async_CompletionPublisherAbstract, zend_class_entry *class_entry_Async_EventDescriptorInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ThreadEvent", class_Async_ThreadEvent_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_CompletionPublisherAbstract, ZEND_ACC_FINAL);
	zend_class_implements(class_entry, 1, class_entry_Async_EventDescriptorInterface);

	return class_entry;
}
