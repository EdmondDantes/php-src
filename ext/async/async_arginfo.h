/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 1f0881e18bfcd8c8b30286b9f4e4c4c93ae0854c */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_suspend, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_spawn, 0, 1, Async\\Coroutine, 0)
	ZEND_ARG_TYPE_INFO(0, task, IS_CALLABLE, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_protect, 0, 1, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO(0, closure, Closure, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_any, 0, 1, Async\\Awaitable, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, triggers, Traversable, MAY_BE_ARRAY, NULL)
ZEND_END_ARG_INFO()

#define arginfo_Async_all arginfo_Async_any

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_anyOf, 0, 2, Async\\Awaitable, 0)
	ZEND_ARG_TYPE_INFO(0, count, IS_LONG, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, triggers, Traversable, MAY_BE_ARRAY, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_captureErrors, 0, 1, Async\\Awaitable, 0)
	ZEND_ARG_OBJ_INFO(0, awaitable, Async\\Awaitable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_delay, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, ms, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_timeout, 0, 1, Async\\Awaitable, 0)
	ZEND_ARG_TYPE_INFO(0, ms, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_currentContext, 0, 0, Async\\Context, 0)
ZEND_END_ARG_INFO()

#define arginfo_Async_coroutineContext arginfo_Async_currentContext

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_currentCoroutine, 0, 0, Async\\Coroutine, 0)
ZEND_END_ARG_INFO()

#define arginfo_Async_rootContext arginfo_Async_currentContext

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_getCoroutines, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Async_gracefulShutdown, 0, 0, IS_VOID, 0)
	ZEND_ARG_OBJ_INFO_WITH_DEFAULT_VALUE(0, cancellationException, Async\\CancellationException, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_Async_exec, 0, 1, Async\\Future, 0)
	ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cwd, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, env, IS_ARRAY, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, returnAll, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_FUNCTION(Async_suspend);
ZEND_FUNCTION(Async_spawn);
ZEND_FUNCTION(Async_protect);
ZEND_FUNCTION(Async_any);
ZEND_FUNCTION(Async_all);
ZEND_FUNCTION(Async_anyOf);
ZEND_FUNCTION(Async_captureErrors);
ZEND_FUNCTION(Async_delay);
ZEND_FUNCTION(Async_timeout);
ZEND_FUNCTION(Async_currentContext);
ZEND_FUNCTION(Async_coroutineContext);
ZEND_FUNCTION(Async_currentCoroutine);
ZEND_FUNCTION(Async_rootContext);
ZEND_FUNCTION(Async_getCoroutines);
ZEND_FUNCTION(Async_gracefulShutdown);
ZEND_FUNCTION(Async_exec);

static const zend_function_entry ext_functions[] = {
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "suspend"), zif_Async_suspend, arginfo_Async_suspend, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "spawn"), zif_Async_spawn, arginfo_Async_spawn, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "protect"), zif_Async_protect, arginfo_Async_protect, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "any"), zif_Async_any, arginfo_Async_any, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "all"), zif_Async_all, arginfo_Async_all, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "anyOf"), zif_Async_anyOf, arginfo_Async_anyOf, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "captureErrors"), zif_Async_captureErrors, arginfo_Async_captureErrors, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "delay"), zif_Async_delay, arginfo_Async_delay, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "timeout"), zif_Async_timeout, arginfo_Async_timeout, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "currentContext"), zif_Async_currentContext, arginfo_Async_currentContext, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "coroutineContext"), zif_Async_coroutineContext, arginfo_Async_coroutineContext, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "currentCoroutine"), zif_Async_currentCoroutine, arginfo_Async_currentCoroutine, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "rootContext"), zif_Async_rootContext, arginfo_Async_rootContext, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "getCoroutines"), zif_Async_getCoroutines, arginfo_Async_getCoroutines, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "gracefulShutdown"), zif_Async_gracefulShutdown, arginfo_Async_gracefulShutdown, 0, NULL, NULL)
	ZEND_RAW_FENTRY(ZEND_NS_NAME("Async", "exec"), zif_Async_exec, arginfo_Async_exec, 0, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Async_Awaitable(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Async", "Awaitable", NULL);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
