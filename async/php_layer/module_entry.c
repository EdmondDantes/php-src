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
#include "module_entry.h"

#include "async/php_reactor.h"
#include "async/libuv/libuv_reactor.h"

#include "zend_smart_str.h"
#include "zend_fibers.h"
#include "ext/standard/info.h"
#include "callback.h"
#include "channel.h"
#include "reactor_handles.h"
#include "exceptions.h"
#include "notifier.h"
#include "../php_async.h"
#include "module_entry_arginfo.h"

#define THROW_IF_REACTOR_DISABLED if (UNEXPECTED(false == reactor_is_enabled())) {			\
		async_throw_error("The operation is not available without an enabled reactor");	\
		RETURN_THROWS();																\
	}

PHP_FUNCTION(Async_launchScheduler)
{
	async_scheduler_launch();
}

PHP_FUNCTION(Async_await)
{
	zval *resume = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_OR_NULL(resume, async_ce_resume)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(IS_ASYNC_OFF && resume != NULL)) {
		async_throw_error("Cannot await a resume object outside of an async context");
		RETURN_THROWS();
	}

	async_await((resume == NULL || ZVAL_IS_NULL(resume)) ? NULL: (async_resume_t *) Z_OBJ_P(resume));
}

PHP_FUNCTION(Async_async)
{
	zval * callable;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable);
	ZEND_PARSE_PARAMETERS_END();

	zval zval_fiber;
	zval params[1];

	ZVAL_COPY_VALUE(&params[0], callable);

	if (object_init_with_constructor(&zval_fiber, zend_ce_fiber, 1, params, NULL) == FAILURE) {
		RETURN_THROWS();
	}

	reactor_fiber_handle_t *fiber_handle = async_fiber_handle_new((zend_fiber *) Z_OBJ(zval_fiber));

	if (fiber_handle == NULL) {
		zval_ptr_dtor(&zval_fiber);
		RETURN_THROWS();
	}

	zval_ptr_dtor(&zval_fiber);

	async_start_fiber(fiber_handle->fiber);

	RETURN_OBJ(&fiber_handle->handle.std);
}

PHP_FUNCTION(Async_defer)
{
	THROW_IF_REACTOR_DISABLED

	zval * callable;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable);
	ZEND_PARSE_PARAMETERS_END();

	async_scheduler_add_microtask(callable);
}

PHP_FUNCTION(Async_delay)
{
	THROW_IF_REACTOR_DISABLED

	zend_long timeout;
	zval * callable;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(timeout);
		Z_PARAM_ZVAL(callable);
	ZEND_PARSE_PARAMETERS_END();

	if (timeout < 0) {
		async_throw_error("The timeout value must be greater than or equal to 0");
		RETURN_THROWS();
	}

	zval callback;
	bool is_callback_owned = false;

	if (Z_TYPE_P(callable) != IS_OBJECT || Z_OBJ_P(callable)->ce != async_ce_callback) {
		is_callback_owned = true;
		zval params[1];
		ZVAL_COPY_VALUE(&params[0], callable);

		if (object_init_with_constructor(&callback, async_ce_callback, 1, params, NULL) == FAILURE) {
			RETURN_THROWS();
		}
	} else {
		ZVAL_COPY_VALUE(&callback, callable);
	}

	reactor_handle_t * handle = reactor_timer_new_fn(timeout, false);

	if (handle == NULL) {
		if (is_callback_owned) {
			zval_ptr_dtor(&callback);
		}

		RETURN_THROWS();
	}

	async_notifier_add_callback(&handle->std, &callback);

	if (is_callback_owned) {
		if (UNEXPECTED(zend_hash_index_update(&ASYNC_G(defer_callbacks), Z_OBJ(callback)->handle, &callback) == NULL)) {
			async_throw_error("Failed to store the callback");
			RETURN_THROWS();
		}
	}

	OBJ_RELEASE(&handle->std);

	if (EG(exception) != NULL) {
		RETURN_THROWS();
	}
}

PHP_FUNCTION(Async_repeat)
{
	THROW_IF_REACTOR_DISABLED

	zend_long timeout;
	zend_object * callback;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(timeout);
		Z_PARAM_OBJ_OF_CLASS(callback, async_ce_callback);
	ZEND_PARSE_PARAMETERS_END();

	if (timeout < 0) {
		async_throw_error("The timeout value must be greater than or equal to 0");
		RETURN_THROWS();
	}

	reactor_handle_t * handle = reactor_timer_new_fn(timeout, true);

	if (handle == NULL) {
		RETURN_THROWS();
	}

	zval zval_callback;
	ZVAL_OBJ(&zval_callback, callback);

	async_notifier_add_callback(&handle->std, &zval_callback);

	if (EG(exception) != NULL) {
		RETURN_THROWS();
	}
}

PHP_FUNCTION(Async_onSignal)
{
	THROW_IF_REACTOR_DISABLED

	zend_long signal;
	zend_object * callback;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(signal);
		Z_PARAM_OBJ_OF_CLASS(callback, async_ce_callback);
	ZEND_PARSE_PARAMETERS_END();

	reactor_handle_t * handle = reactor_signal_new_fn(signal);

	if (handle == NULL) {
		RETURN_THROWS();
	}

	zval zval_callback;
	ZVAL_OBJ(&zval_callback, callback);

	async_notifier_add_callback(&handle->std, &zval_callback);

	if (EG(exception) != NULL) {
		RETURN_THROWS();
	}
}

ZEND_MINIT_FUNCTION(async)
{
	async_register_callback_ce();
	async_register_notifier_ce();
	async_register_handlers_ce();
	async_register_resume_ce();
	async_register_exceptions_ce();
	async_register_channel_ce();

#ifdef PHP_ASYNC_LIBUV
	async_libuv_startup();
#endif

	return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(async)
{
#ifdef PHP_ASYNC_LIBUV
	async_libuv_shutdown();
#endif
	return SUCCESS;
}

PHP_MINFO_FUNCTION(async_info) {
	php_info_print_table_start();
	php_info_print_table_header(2, "Module", "True Asynchrony");
	php_info_print_table_row(2, "Version", PHP_ASYNC_VERSION);
	php_info_print_table_row(2, "Support", "Enabled");
#ifdef PHP_ASYNC_LIBUV
	php_info_print_table_row(2, "LibUv Reactor", "Enabled");
#else
	php_info_print_table_row(2, "LibUv Reactor", "Disabled");
#endif
	php_info_print_table_end();
}

/* {{{ PHP_GINIT_FUNCTION */
static PHP_GINIT_FUNCTION(async)
{
#if defined(COMPILE_DL_ASYNC) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	async_globals->is_async = false;
	async_globals->in_scheduler_context = false;
	async_globals->reactor = NULL;
	async_globals->exception_handler = NULL;
	async_globals->execute_callbacks_handler = NULL;
	async_globals->execute_next_fiber_handler = NULL;
	async_globals->execute_microtasks_handler = NULL;

	async_globals_ctor(async_globals);
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION */
static PHP_GSHUTDOWN_FUNCTION(async)
{
	async_globals_dtor(async_globals);
}

static zend_module_entry async_module_entry = { /* {{{ */
	STANDARD_MODULE_HEADER,
	"PHP True Asynchrony",
	ext_functions,
	ZEND_MINIT(async),
	ZEND_MSHUTDOWN(async),
	NULL,
	NULL,
	PHP_MINFO(async_info),
	PHP_ASYNC_VERSION,
	PHP_MODULE_GLOBALS(async),
	PHP_GINIT(async),
	PHP_GSHUTDOWN(async),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */


zend_result async_register_module(void) /* {{{ */
{
	zend_module_entry *module;

	EG(current_module) = module = zend_register_module_ex(&async_module_entry, MODULE_PERSISTENT);

	if (UNEXPECTED(module == NULL)) {
		return FAILURE;
	}

	ZEND_ASSERT(module->module_number != 0);

	return SUCCESS;
}
/* }}} */