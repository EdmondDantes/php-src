/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 3a0e1be61d7dd5283264902466ca3e9c5d4a13e6 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_launchScheduler, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_wait, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, resume, Async\\Resume, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_run, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, task, IS_CALLABLE, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_async, 0, 1, Async\\FiberHandle, 0)
	ZEND_ARG_TYPE_INFO(0, task, IS_CALLABLE, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_defer, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, microtask, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_delay, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, callback, Async\\Callback, MAY_BE_CALLABLE, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_repeat, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, interval, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Async\\Callback, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_onSignal, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, sigNumber, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, callback, Async\\Callback, 0)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(Async_launchScheduler);
ZEND_FUNCTION(Async_wait);
ZEND_FUNCTION(Async_run);
ZEND_FUNCTION(Async_async);
ZEND_FUNCTION(Async_defer);
ZEND_FUNCTION(Async_delay);
ZEND_FUNCTION(Async_repeat);
ZEND_FUNCTION(Async_onSignal);

static const zend_function_entry ext_functions[] = {
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "launchScheduler"), zif_Async_launchScheduler, arginfo_Async_launchScheduler, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "wait"), zif_Async_wait, arginfo_Async_wait, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "run"), zif_Async_run, arginfo_Async_run, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "async"), zif_Async_async, arginfo_Async_async, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "defer"), zif_Async_defer, arginfo_Async_defer, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "delay"), zif_Async_delay, arginfo_Async_delay, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "repeat"), zif_Async_repeat, arginfo_Async_repeat, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "onSignal"), zif_Async_onSignal, arginfo_Async_onSignal, 0, NULL, NULL)
	ZEND_FE_END
};
