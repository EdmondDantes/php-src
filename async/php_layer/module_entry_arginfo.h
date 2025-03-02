/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 678c00c0d48a1c5ba05a31d4504b60c710836f80 */

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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_await, 0, 1, IS_MIXED, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, awaitable, Async\\FiberHandle|Async\\Future|Fiber, MAY_BE_CALLABLE, NULL)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_awaitFirst, 0, 1, IS_MIXED, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, futures, Traversable, MAY_BE_ARRAY, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ignoreErrors, _IS_BOOL, 0, "false")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_awaitAnyN, 0, 2, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, count, IS_LONG, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, futures, Traversable, MAY_BE_ARRAY, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ignoreErrors, _IS_BOOL, 0, "false")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_awaitAll, 0, 1, IS_ARRAY, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, futures, Traversable, MAY_BE_ARRAY, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ignoreErrors, _IS_BOOL, 0, "false")
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellation, Async\\\116otifier, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_defer, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, microtask, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_delay, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, callback, Async\\Closure, MAY_BE_CALLABLE, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_trapSignal, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_MASK(0, sigNumber, MAY_BE_LONG|MAY_BE_ARRAY, NULL)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_getSupportedSignals, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_exec, 0, 1, Async\\Future, 0)
	ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cwd, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, env, IS_ARRAY, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, returnAll, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

#define arginfo_Async_getFibers arginfo_Async_getSupportedSignals

#define arginfo_Async_getResumes arginfo_Async_getSupportedSignals

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_gracefulShutdown, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, throwable, Throwable, 1, "null")
ZEND_END_ARG_INFO()

ZEND_FUNCTION(Async_launchScheduler);
ZEND_FUNCTION(Async_wait);
ZEND_FUNCTION(Async_run);
ZEND_FUNCTION(Async_async);
ZEND_FUNCTION(Async_await);
ZEND_FUNCTION(Async_awaitFirst);
ZEND_FUNCTION(Async_awaitAnyN);
ZEND_FUNCTION(Async_awaitAll);
ZEND_FUNCTION(Async_defer);
ZEND_FUNCTION(Async_delay);
ZEND_FUNCTION(Async_trapSignal);
ZEND_FUNCTION(Async_getSupportedSignals);
ZEND_FUNCTION(Async_exec);
ZEND_FUNCTION(Async_getFibers);
ZEND_FUNCTION(Async_getResumes);
ZEND_FUNCTION(Async_gracefulShutdown);

static const zend_function_entry ext_functions[] = {
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "launchScheduler"), zif_Async_launchScheduler, arginfo_Async_launchScheduler, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "wait"), zif_Async_wait, arginfo_Async_wait, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "run"), zif_Async_run, arginfo_Async_run, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "async"), zif_Async_async, arginfo_Async_async, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "await"), zif_Async_await, arginfo_Async_await, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "awaitFirst"), zif_Async_awaitFirst, arginfo_Async_awaitFirst, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "awaitAnyN"), zif_Async_awaitAnyN, arginfo_Async_awaitAnyN, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "awaitAll"), zif_Async_awaitAll, arginfo_Async_awaitAll, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "defer"), zif_Async_defer, arginfo_Async_defer, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "delay"), zif_Async_delay, arginfo_Async_delay, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "trapSignal"), zif_Async_trapSignal, arginfo_Async_trapSignal, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "getSupportedSignals"), zif_Async_getSupportedSignals, arginfo_Async_getSupportedSignals, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "exec"), zif_Async_exec, arginfo_Async_exec, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "getFibers"), zif_Async_getFibers, arginfo_Async_getFibers, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "getResumes"), zif_Async_getResumes, arginfo_Async_getResumes, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "gracefulShutdown"), zif_Async_gracefulShutdown, arginfo_Async_gracefulShutdown, 0, NULL, NULL)
	ZEND_FE_END
};
