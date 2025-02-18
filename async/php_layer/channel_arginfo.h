/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: ca87ad163423c61f3c71ecdfa288d17060a9caf5 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ChannelStateInterface_isClosed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_ChannelStateInterface_isFull arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_ChannelStateInterface_isEmpty arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_ChannelStateInterface_isNotEmpty arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_ChannelStateInterface_isProducingFinished arginfo_class_Async_ChannelStateInterface_isClosed

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ChannelStateInterface_getCapacity, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_ChannelStateInterface_getUsed arginfo_class_Async_ChannelStateInterface_getCapacity

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ProducerInterface_send, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, waitOnFull, _IS_BOOL, 1, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ProducerInterface_trySend, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ProducerInterface_waitUntilWritable, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ProducerInterface_finishProducing, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ConsumerInterface_receive, 0, 0, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Async_ConsumerInterface_tryReceive, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Async_ConsumerInterface_waitUntilReadable arginfo_class_Async_ProducerInterface_waitUntilWritable

#define arginfo_class_Async_ConsumerInterface_discardData arginfo_class_Async_ProducerInterface_finishProducing

#define arginfo_class_Async_ConsumerInterface_finishConsuming arginfo_class_Async_ProducerInterface_finishProducing

#define arginfo_class_Async_ChannelInterface_close arginfo_class_Async_ProducerInterface_finishProducing

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Async_ChannelInterface_getNotifier, 0, 0, Async\\\116otifier, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Async_Channel___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, capacity, IS_LONG, 0, "1")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, owner, Fiber, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, expandable, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_class_Async_Channel_send arginfo_class_Async_ProducerInterface_send

#define arginfo_class_Async_Channel_trySend arginfo_class_Async_ProducerInterface_trySend

#define arginfo_class_Async_Channel_receive arginfo_class_Async_ConsumerInterface_receive

#define arginfo_class_Async_Channel_tryReceive arginfo_class_Async_ConsumerInterface_tryReceive

#define arginfo_class_Async_Channel_waitUntilWritable arginfo_class_Async_ProducerInterface_waitUntilWritable

#define arginfo_class_Async_Channel_waitUntilReadable arginfo_class_Async_ProducerInterface_waitUntilWritable

#define arginfo_class_Async_Channel_finishProducing arginfo_class_Async_ProducerInterface_finishProducing

#define arginfo_class_Async_Channel_finishConsuming arginfo_class_Async_ProducerInterface_finishProducing

#define arginfo_class_Async_Channel_discardData arginfo_class_Async_ProducerInterface_finishProducing

#define arginfo_class_Async_Channel_close arginfo_class_Async_ProducerInterface_finishProducing

#define arginfo_class_Async_Channel_isClosed arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_Channel_isFull arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_Channel_isEmpty arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_Channel_isNotEmpty arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_Channel_isProducingFinished arginfo_class_Async_ChannelStateInterface_isClosed

#define arginfo_class_Async_Channel_getCapacity arginfo_class_Async_ChannelStateInterface_getCapacity

#define arginfo_class_Async_Channel_getUsed arginfo_class_Async_ChannelStateInterface_getCapacity

#define arginfo_class_Async_Channel_getNotifier arginfo_class_Async_ChannelInterface_getNotifier

ZEND_METHOD(Async_Channel, __construct);
ZEND_METHOD(Async_Channel, send);
ZEND_METHOD(Async_Channel, trySend);
ZEND_METHOD(Async_Channel, receive);
ZEND_METHOD(Async_Channel, tryReceive);
ZEND_METHOD(Async_Channel, waitUntilWritable);
ZEND_METHOD(Async_Channel, waitUntilReadable);
ZEND_METHOD(Async_Channel, finishProducing);
ZEND_METHOD(Async_Channel, finishConsuming);
ZEND_METHOD(Async_Channel, discardData);
ZEND_METHOD(Async_Channel, close);
ZEND_METHOD(Async_Channel, isClosed);
ZEND_METHOD(Async_Channel, isFull);
ZEND_METHOD(Async_Channel, isEmpty);
ZEND_METHOD(Async_Channel, isNotEmpty);
ZEND_METHOD(Async_Channel, isProducingFinished);
ZEND_METHOD(Async_Channel, getCapacity);
ZEND_METHOD(Async_Channel, getUsed);
ZEND_METHOD(Async_Channel, getNotifier);

static const zend_function_entry class_Async_ChannelStateInterface_methods[] = {
	ZEND_RAW_FENTRY("isClosed", NULL, arginfo_class_Async_ChannelStateInterface_isClosed, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("isFull", NULL, arginfo_class_Async_ChannelStateInterface_isFull, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("isEmpty", NULL, arginfo_class_Async_ChannelStateInterface_isEmpty, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("isNotEmpty", NULL, arginfo_class_Async_ChannelStateInterface_isNotEmpty, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("isProducingFinished", NULL, arginfo_class_Async_ChannelStateInterface_isProducingFinished, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("getCapacity", NULL, arginfo_class_Async_ChannelStateInterface_getCapacity, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("getUsed", NULL, arginfo_class_Async_ChannelStateInterface_getUsed, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ProducerInterface_methods[] = {
	ZEND_RAW_FENTRY("send", NULL, arginfo_class_Async_ProducerInterface_send, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("trySend", NULL, arginfo_class_Async_ProducerInterface_trySend, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("waitUntilWritable", NULL, arginfo_class_Async_ProducerInterface_waitUntilWritable, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("finishProducing", NULL, arginfo_class_Async_ProducerInterface_finishProducing, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ConsumerInterface_methods[] = {
	ZEND_RAW_FENTRY("receive", NULL, arginfo_class_Async_ConsumerInterface_receive, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("tryReceive", NULL, arginfo_class_Async_ConsumerInterface_tryReceive, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("waitUntilReadable", NULL, arginfo_class_Async_ConsumerInterface_waitUntilReadable, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("discardData", NULL, arginfo_class_Async_ConsumerInterface_discardData, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("finishConsuming", NULL, arginfo_class_Async_ConsumerInterface_finishConsuming, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_ChannelInterface_methods[] = {
	ZEND_RAW_FENTRY("close", NULL, arginfo_class_Async_ChannelInterface_close, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("getNotifier", NULL, arginfo_class_Async_ChannelInterface_getNotifier, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static const zend_function_entry class_Async_Channel_methods[] = {
	ZEND_ME(Async_Channel, __construct, arginfo_class_Async_Channel___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, send, arginfo_class_Async_Channel_send, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, trySend, arginfo_class_Async_Channel_trySend, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, receive, arginfo_class_Async_Channel_receive, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, tryReceive, arginfo_class_Async_Channel_tryReceive, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, waitUntilWritable, arginfo_class_Async_Channel_waitUntilWritable, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, waitUntilReadable, arginfo_class_Async_Channel_waitUntilReadable, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, finishProducing, arginfo_class_Async_Channel_finishProducing, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, finishConsuming, arginfo_class_Async_Channel_finishConsuming, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, discardData, arginfo_class_Async_Channel_discardData, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, close, arginfo_class_Async_Channel_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isClosed, arginfo_class_Async_Channel_isClosed, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isFull, arginfo_class_Async_Channel_isFull, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isEmpty, arginfo_class_Async_Channel_isEmpty, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isNotEmpty, arginfo_class_Async_Channel_isNotEmpty, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, isProducingFinished, arginfo_class_Async_Channel_isProducingFinished, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, getCapacity, arginfo_class_Async_Channel_getCapacity, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, getUsed, arginfo_class_Async_Channel_getUsed, ZEND_ACC_PUBLIC)
	ZEND_ME(Async_Channel, getNotifier, arginfo_class_Async_Channel_getNotifier, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_ChannelStateInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelStateInterface", class_Async_ChannelStateInterface_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Async_ProducerInterface(zend_class_entry *class_entry_Async_ChannelStateInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ProducerInterface", class_Async_ProducerInterface_methods);
	class_entry = zend_register_internal_interface(&ce);
	zend_class_implements(class_entry, 1, class_entry_Async_ChannelStateInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_ConsumerInterface(zend_class_entry *class_entry_Async_ChannelStateInterface, zend_class_entry *class_entry_Iterator)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ConsumerInterface", class_Async_ConsumerInterface_methods);
	class_entry = zend_register_internal_interface(&ce);
	zend_class_implements(class_entry, 2, class_entry_Async_ChannelStateInterface, class_entry_Iterator);

	return class_entry;
}

static zend_class_entry *register_class_Async_ChannelInterface(zend_class_entry *class_entry_Async_ProducerInterface, zend_class_entry *class_entry_Async_ConsumerInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelInterface", class_Async_ChannelInterface_methods);
	class_entry = zend_register_internal_interface(&ce);
	zend_class_implements(class_entry, 2, class_entry_Async_ProducerInterface, class_entry_Async_ConsumerInterface);

	return class_entry;
}

static zend_class_entry *register_class_Async_ChannelException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}

static zend_class_entry *register_class_Async_ChannelWasClosed(zend_class_entry *class_entry_Async_ChannelException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelWasClosed", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_ChannelException, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_ChannelIsFull(zend_class_entry *class_entry_Async_ChannelException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelIsFull", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_ChannelException, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_ChannelNotifier(zend_class_entry *class_entry_Async_Notifier)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ChannelNotifier", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Notifier, ZEND_ACC_FINAL);

	return class_entry;
}

static zend_class_entry *register_class_Async_Channel(zend_class_entry *class_entry_Async_ChannelInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Channel", class_Async_Channel_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);
	zend_class_implements(class_entry, 1, class_entry_Async_ChannelInterface);

	zval property_owner_default_value;
	ZVAL_NULL(&property_owner_default_value);
	zend_string *property_owner_name = zend_string_init("owner", sizeof("owner") - 1, 1);
	zend_string *property_owner_class_Fiber = zend_string_init("Fiber", sizeof("Fiber")-1, 1);
	zend_declare_typed_property(class_entry, property_owner_name, &property_owner_default_value, ZEND_ACC_PUBLIC|ZEND_ACC_READONLY, NULL, (zend_type) ZEND_TYPE_INIT_CLASS(property_owner_class_Fiber, 0, MAY_BE_NULL));
	zend_string_release(property_owner_name);

	return class_entry;
}

static zend_class_entry *register_class_Async_ThreadChannel(zend_class_entry *class_entry_Async_Channel)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ThreadChannel", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Channel, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}

static zend_class_entry *register_class_Async_ProcessChannel(zend_class_entry *class_entry_Async_Channel)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "ProcessChannel", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Async_Channel, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES|ZEND_ACC_NOT_SERIALIZABLE);

	return class_entry;
}
