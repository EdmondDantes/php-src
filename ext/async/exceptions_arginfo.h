/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f9b012c7b81b32e93b79ad410ec63c94ed2c360a */

static zend_class_entry *register_class_Async_CancellationException(zend_class_entry *class_entry_Throwable)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "CancellationException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, 0);
	zend_class_implements(class_entry, 1, class_entry_Throwable);

	return class_entry;
}

static zend_class_entry *register_class_Async_AsyncException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "AsyncException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}

static zend_class_entry *register_class_Async_InputOutputException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "InputOutputException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}

static zend_class_entry *register_class_Async_TimeoutException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "TimeoutException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}

static zend_class_entry *register_class_Async_PollException(zend_class_entry *class_entry_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "PollException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Exception, 0);

	return class_entry;
}
