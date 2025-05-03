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
#include "php_scheduler.h"
#include "exceptions.h"
#include "scope.h"
#include "functions.h"
#include "async_API.h"
#ifdef PHP_ASYNC_LIBUV
#include "libuv_reactor.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(async)

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