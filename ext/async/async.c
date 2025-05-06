/*
+----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Edmond                                                       |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "php_async.h"
#include "ext/standard/info.h"
#include "scheduler.h"
#include "exceptions.h"
#include "scope.h"
#include "async_API.h"
#include "async_arginfo.h"
#ifdef PHP_ASYNC_LIBUV
#include "libuv_reactor.h"
#endif

///////////////////////////////////////////////////////////////
/// Module functions
///////////////////////////////////////////////////////////////

#define THROW_IF_SCHEDULER_CONTEXT if (UNEXPECTED(ZEND_IN_SCHEDULER_CONTEXT)) {				\
		async_throw_error("The operation cannot be executed in the scheduler context");		\
		RETURN_THROWS();																	\
	}

#define THROW_IF_ASYNC_OFF if (UNEXPECTED(ZEND_IS_ASYNC_OFF)) {								\
		async_throw_error("The operation cannot be executed while async is off");			\
		RETURN_THROWS();																	\
	}

PHP_FUNCTION(Async_spawn)
{
	THROW_IF_ASYNC_OFF;
	THROW_IF_SCHEDULER_CONTEXT;

	zval *args = NULL;
	int args_count = 0;
	HashTable *named_args = NULL;

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_FUNC(fci, fcc);
		Z_PARAM_VARIADIC_WITH_NAMED(args, args_count, named_args);
	ZEND_PARSE_PARAMETERS_END();

	async_coroutine_t * coroutine = (async_coroutine_t *) ZEND_ASYNC_SPAWN(NULL);

	if (UNEXPECTED(EG(exception))) {
		return;
	}

	zend_fcall_t * fcall = ecalloc(1, sizeof(zend_fcall_t));
	fcall->fci = fci;
	fcall->fci_cache = fcc;

	if (args_count) {
		fcall->fci.param_count = args_count;
		fcall->fci.params = safe_emalloc(args_count, sizeof(zval), 0);

		for (uint32_t i = 0; i < args_count; i++) {
			ZVAL_COPY(&fcall->fci.params[i], &args[i]);
		}
	}

	if (named_args) {
		fcall->fci.named_params = named_args;
		GC_ADDREF(named_args);
	}

	coroutine->coroutine.fcall = fcall;

	RETURN_OBJ(&coroutine->std);
}

PHP_FUNCTION(Async_spawnWith)
{
	THROW_IF_ASYNC_OFF;
	THROW_IF_SCHEDULER_CONTEXT;

	zval *args = NULL;
	int args_count = 0;
	HashTable *named_args = NULL;

	zend_object * scope = NULL;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	ZEND_PARSE_PARAMETERS_START(2, -1)
		Z_PARAM_OBJ_OF_CLASS(scope, async_ce_scope_provider)
		Z_PARAM_FUNC(fci, fcc);
		Z_PARAM_VARIADIC_WITH_NAMED(args, args_count, named_args);
	ZEND_PARSE_PARAMETERS_END();

	// @TODO: Check if scope is a valid scope provider

	async_coroutine_t * coroutine = (async_coroutine_t *) ZEND_ASYNC_SPAWN(scope);

	if (UNEXPECTED(EG(exception))) {
		return;
	}

	zend_fcall_t * fcall = ecalloc(1, sizeof(zend_fcall_t));
	fcall->fci = fci;
	fcall->fci_cache = fcc;

	if (args_count) {
		fcall->fci.param_count = args_count;
		fcall->fci.params = safe_emalloc(args_count, sizeof(zval), 0);

		for (uint32_t i = 0; i < args_count; i++) {
			ZVAL_COPY(&fcall->fci.params[i], &args[i]);
		}
	}

	if (named_args) {
		fcall->fci.named_params = named_args;
		GC_ADDREF(named_args);
	}

	coroutine->coroutine.fcall = fcall;

	RETURN_OBJ(&coroutine->std);
}

PHP_FUNCTION(Async_suspend)
{
	THROW_IF_ASYNC_OFF;
	THROW_IF_SCHEDULER_CONTEXT;
}

PHP_FUNCTION(Async_protect)
{
	THROW_IF_ASYNC_OFF;
	THROW_IF_SCHEDULER_CONTEXT;
}

PHP_FUNCTION(Async_any)
{

}

PHP_FUNCTION(Async_all)
{

}

PHP_FUNCTION(Async_anyOff)
{

}

PHP_FUNCTION(Async_captureErrors)
{

}

PHP_FUNCTION(Async_delay)
{

}

PHP_FUNCTION(Async_timeout)
{

}

PHP_FUNCTION(Async_currentContext)
{

}

PHP_FUNCTION(Async_coroutineContext)
{

}

PHP_FUNCTION(Async_currentCoroutine)
{

}

PHP_FUNCTION(Async_rootContext)
{

}

PHP_FUNCTION(Async_getCoroutines)
{

}

PHP_FUNCTION(Async_gracefulShutdown)
{

}

/*
PHP_FUNCTION(Async_exec)
{

}
*/

///////////////////////////////////////////////////////////////
/// Register Async Module
///////////////////////////////////////////////////////////////

ZEND_DECLARE_MODULE_GLOBALS(async)

void async_register_awaitable_ce(void)
{
	async_ce_awaitable = register_class_Async_Awaitable();
}

static PHP_GINIT_FUNCTION(async)
{
#if defined(COMPILE_DL_ASYNC) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
}

/* {{{ PHP_GSHUTDOWN_FUNCTION */
static PHP_GSHUTDOWN_FUNCTION(async)
{
#ifdef PHP_WIN32
	//zend_hash_destroy(&async_globals);
#endif
}
/* }}} */

/* Module registration */

ZEND_MINIT_FUNCTION(async)
{
	async_register_awaitable_ce();
	async_register_scope_ce();
	async_register_coroutine_ce();
	async_register_exceptions_ce();
	//async_register_notifier_ce();
	//async_register_handlers_ce();
	//async_register_channel_ce();
	//async_register_iterator_ce();
	//async_register_context_ce();
	//async_register_future_ce();

	async_scheduler_startup();

	async_api_register();

#ifdef PHP_ASYNC_LIBUV
	async_libuv_reactor_register();
#endif

	return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(async)
{
	//async_scheduler_shutdown();

#ifdef PHP_ASYNC_LIBUV
	//async_libuv_shutdown();
#endif
	return SUCCESS;
}

PHP_MINFO_FUNCTION(async) {
	php_info_print_table_start();
	php_info_print_table_header(2, "Module", PHP_ASYNC_NAME);
	php_info_print_table_row(2, "Version", PHP_ASYNC_VERSION);
	php_info_print_table_row(2, "Support", "Enabled");
#ifdef PHP_ASYNC_LIBUV
	php_info_print_table_row(2, "LibUv Reactor", "Enabled");
#else
	php_info_print_table_row(2, "LibUv Reactor", "Disabled");
#endif
	php_info_print_table_end();
}

PHP_RINIT_FUNCTION(async) /* {{{ */
{
	//async_host_name_list_ctor();
	return SUCCESS;
} /* }}} */

PHP_RSHUTDOWN_FUNCTION(async) /* {{{ */
{
	//async_globals_dtor(ASYNC_GLOBAL);
	//async_host_name_list_dtor();
	return SUCCESS;
} /* }}} */

zend_module_entry async_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_ASYNC_NAME,
	ext_functions,
	PHP_MINIT(async),
	PHP_MSHUTDOWN(async),
	NULL,
	PHP_RSHUTDOWN(async),
	PHP_MINFO(async),
	PHP_ASYNC_VERSION,
	PHP_MODULE_GLOBALS(async),
	PHP_GINIT(async),
	PHP_GSHUTDOWN(async),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};