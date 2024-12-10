/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: d782c465f885682f2061e5089bce1d8ec356abf4 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_FutureInterface_getStatus, 0, 0, IS_LONG, 0)
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

#define arginfo_class_Async_DeferredAbstract_getStatus arginfo_class_Async_FutureInterface_getStatus

#define arginfo_class_Async_DeferredAbstract_isReady arginfo_class_Async_FutureInterface_isReady

#define arginfo_class_Async_DeferredAbstract_isFailed arginfo_class_Async_FutureInterface_isReady

#define arginfo_class_Async_DeferredAbstract_getResult arginfo_class_Async_FutureInterface_getResult

#define arginfo_class_Async_DeferredAbstract_getError arginfo_class_Async_FutureInterface_getError

#define arginfo_class_Async_DeferredAbstract_cancel arginfo_class_Async_CancellationInterface_cancel

#define arginfo_class_Async_DeferredAbstract_onSuccess arginfo_class_Async_CompletionPublisherInterface_onSuccess

#define arginfo_class_Async_DeferredAbstract_onError arginfo_class_Async_CompletionPublisherInterface_onError

#define arginfo_class_Async_DeferredAbstract_onFinally arginfo_class_Async_CompletionPublisherInterface_onFinally

#define arginfo_class_Async_DeferredAbstract_thenIgnore arginfo_class_Async_ThenInterface_thenIgnore

#define arginfo_class_Async_DeferredAbstract_thenResolve arginfo_class_Async_ThenInterface_thenResolve

#define arginfo_class_Async_DeferredAbstract_thenReject arginfo_class_Async_ThenInterface_thenResolve

#define arginfo_class_Async_DeferredAbstract_resolve arginfo_class_Async_DeferredInterface_resolve

#define arginfo_class_Async_DeferredAbstract_reject arginfo_class_Async_DeferredInterface_reject

#define arginfo_class_Async_DeferredAbstract_getFuture arginfo_class_Async_DeferredInterface_getFuture

ZEND_METHOD(Async_DeferredAbstract, getStatus);
ZEND_METHOD(Async_DeferredAbstract, isReady);
ZEND_METHOD(Async_DeferredAbstract, isFailed);
ZEND_METHOD(Async_DeferredAbstract, getResult);
ZEND_METHOD(Async_DeferredAbstract, getError);
ZEND_METHOD(Async_DeferredAbstract, cancel);
ZEND_METHOD(Async_DeferredAbstract, onSuccess);
ZEND_METHOD(Async_DeferredAbstract, onError);
ZEND_METHOD(Async_DeferredAbstract, onFinally);
ZEND_METHOD(Async_DeferredAbstract, thenIgnore);
ZEND_METHOD(Async_DeferredAbstract, thenResolve);
ZEND_METHOD(Async_DeferredAbstract, thenReject);
ZEND_METHOD(Async_DeferredAbstract, resolve);
ZEND_METHOD(Async_DeferredAbstract, reject);
ZEND_METHOD(Async_DeferredAbstract, getFuture);

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

static const zend_function_entry class_Async_DeferredAbstract_methods[] = {
	ZEND_ME(Async_DeferredAbstract, getStatus, arginfo_class_Async_DeferredAbstract_getStatus, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, isReady, arginfo_class_Async_DeferredAbstract_isReady, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, isFailed, arginfo_class_Async_DeferredAbstract_isFailed, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, getResult, arginfo_class_Async_DeferredAbstract_getResult, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, getError, arginfo_class_Async_DeferredAbstract_getError, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, cancel, arginfo_class_Async_DeferredAbstract_cancel, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, onSuccess, arginfo_class_Async_DeferredAbstract_onSuccess, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, onError, arginfo_class_Async_DeferredAbstract_onError, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, onFinally, arginfo_class_Async_DeferredAbstract_onFinally, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, thenIgnore, arginfo_class_Async_DeferredAbstract_thenIgnore, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, thenResolve, arginfo_class_Async_DeferredAbstract_thenResolve, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, thenReject, arginfo_class_Async_DeferredAbstract_thenReject, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, resolve, arginfo_class_Async_DeferredAbstract_resolve, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, reject, arginfo_class_Async_DeferredAbstract_reject, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_DeferredAbstract, getFuture, arginfo_class_Async_DeferredAbstract_getFuture, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

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

static zend_class_entry *register_class_Async_DeferredAbstract(zend_class_entry *class_entry_Async_DeferredInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "DeferredAbstract", class_Async_DeferredAbstract_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_ABSTRACT);
	zend_class_implements(class_entry, 1, class_entry_Async_DeferredInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_DeferredResume(zend_class_entry *class_entry_Async_DeferredAbstract)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "DeferredResume", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_DeferredAbstract, ZEND_ACC_FINAL);

	return class_entry;
}
