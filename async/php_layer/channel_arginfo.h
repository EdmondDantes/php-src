/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 1d67b8290bda821670eed435bfa5f34ae5858aaf */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Channel___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, capacity, IS_LONG, 0, "8")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, expandable, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Channel_send, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, waitOnFull, _IS_BOOL, 1, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Channel_sendAsync, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Channel_receive, 0, 0, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Channel_receiveAsync, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Channel_close, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Channel_isClosed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Channel_isFull arginfo_class_Async_Channel_isClosed

#define arginfo_class_Async_Channel_isEmpty arginfo_class_Async_Channel_isClosed

#define arginfo_class_Async_Channel_isNotEmpty arginfo_class_Async_Channel_isClosed

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_Channel_getCapacity, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Channel_getUsed arginfo_class_Async_Channel_getCapacity

#define arginfo_class_Async_ThreadChannel_getSenderThreadId arginfo_class_Async_Channel_getCapacity

#define arginfo_class_Async_ThreadChannel_getReceiverThreadId arginfo_class_Async_Channel_getCapacity

#define arginfo_class_Async_ProcessChannel_getSenderProcessId arginfo_class_Async_Channel_getCapacity

#define arginfo_class_Async_ProcessChannel_getReceiverProcessId arginfo_class_Async_Channel_getCapacity

ZEND_METHOD(Async_Channel, __construct);
ZEND_METHOD(Async_Channel, send);
ZEND_METHOD(Async_Channel, sendAsync);
ZEND_METHOD(Async_Channel, receive);
ZEND_METHOD(Async_Channel, receiveAsync);
ZEND_METHOD(Async_Channel, close);
ZEND_METHOD(Async_Channel, isClosed);
ZEND_METHOD(Async_Channel, isFull);
ZEND_METHOD(Async_Channel, isEmpty);
ZEND_METHOD(Async_Channel, isNotEmpty);
ZEND_METHOD(Async_Channel, getCapacity);
ZEND_METHOD(Async_Channel, getUsed);
ZEND_METHOD(Async_ThreadChannel, getSenderThreadId);
ZEND_METHOD(Async_ThreadChannel, getReceiverThreadId);
ZEND_METHOD(Async_ProcessChannel, getSenderProcessId);
ZEND_METHOD(Async_ProcessChannel, getReceiverProcessId);

static const zend_function_entry class_Async_Channel_methods[] = {
	ZEND_ME(Async_Channel, __construct, arginfo_class_Async_Channel___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, send, arginfo_class_Async_Channel_send, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, sendAsync, arginfo_class_Async_Channel_sendAsync, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, receive, arginfo_class_Async_Channel_receive, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, receiveAsync, arginfo_class_Async_Channel_receiveAsync, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, close, arginfo_class_Async_Channel_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isClosed, arginfo_class_Async_Channel_isClosed, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isFull, arginfo_class_Async_Channel_isFull, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isEmpty, arginfo_class_Async_Channel_isEmpty, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isNotEmpty, arginfo_class_Async_Channel_isNotEmpty, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, getCapacity, arginfo_class_Async_Channel_getCapacity, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, getUsed, arginfo_class_Async_Channel_getUsed, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ThreadChannel_methods[] = {
	ZEND_ME(Async_ThreadChannel, getSenderThreadId, arginfo_class_Async_ThreadChannel_getSenderThreadId, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_ThreadChannel, getReceiverThreadId, arginfo_class_Async_ThreadChannel_getReceiverThreadId, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ProcessChannel_methods[] = {
	ZEND_ME(Async_ProcessChannel, getSenderProcessId, arginfo_class_Async_ProcessChannel_getSenderProcessId, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_ProcessChannel, getReceiverProcessId, arginfo_class_Async_ProcessChannel_getReceiverProcessId, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_ChannelException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}

static zend_class_entry *register_class_Async_ChannelWasClosed(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelWasClosed", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}

static zend_class_entry *register_class_Async_ChannelIsFull(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelIsFull", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}

static zend_class_entry *register_class_Async_Channel(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Channel", class_Async_Channel_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_ThreadChannel(zend_class_entry *class_entry_Async_Channel)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ThreadChannel", class_Async_ThreadChannel_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Channel, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_ProcessChannel(zend_class_entry *class_entry_Async_Channel)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ProcessChannel", class_Async_ProcessChannel_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Channel, ZEND_ACC_FINAL);

	return class_entry;
}
