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
#include "functions.h"

#include <async/php_reactor.h>

#include "zend_common.h"
#include "zend_smart_str.h"
#include "zend_fibers.h"
#include "ext/standard/info.h"
#include "callback.h"
#include "channel.h"
#include "ev_handles.h"
#include "exceptions.h"
#include "notifier.h"
#include "../php_async.h"
#include "functions_arginfo.h"

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

	async_await(ZVAL_IS_NULL(resume) ? NULL: (async_resume_t *) Z_OBJ_P(resume));
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

	zval zval_fiber_handle;

	if (object_init_with_constructor(&zval_fiber_handle, async_ce_fiber_handle, 1, params, NULL) == FAILURE) {
		zval_ptr_dtor(&zval_fiber);
		RETURN_THROWS();
	}

	async_fiber_handle_t *fiber_handle = (async_fiber_handle_t *) Z_OBJ_P(&zval_fiber_handle);

	fiber_handle->fiber = (zend_fiber *) Z_OBJ_P(&zval_fiber);

	async_resume_t * resume = async_resume_new((zend_fiber *)Z_OBJ(zval_fiber));

	async_resume_fiber(resume, NULL, NULL);

	// Resume GC counter = 1 after this operation
	OBJ_RELEASE(&resume->std);
	RETURN_OBJ(&fiber_handle->handle.std);
}

PHP_FUNCTION(Async_defer)
{
	if (UNEXPECTED(IS_ASYNC_OFF)) {
		async_throw_error("Cannot defer execution outside of an async context");
		RETURN_THROWS();
	}

	zval * callable;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable);
	ZEND_PARSE_PARAMETERS_END();

	async_scheduler_add_microtask(callable);
}

PHP_FUNCTION(Async_delay)
{
	if (UNEXPECTED(IS_ASYNC_OFF)) {
		async_throw_error("Cannot delay execution outside of an async context");
		RETURN_THROWS();
	}

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
		zval_ptr_dtor(&callback);
	}

	if (EG(exception) != NULL) {
		RETURN_THROWS();
	}
}

PHP_FUNCTION(Async_repeat)
{
	if (UNEXPECTED(IS_ASYNC_OFF)) {
		async_throw_error("Cannot repeat execution outside of an async context");
		RETURN_THROWS();
	}

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
	if (UNEXPECTED(IS_ASYNC_OFF)) {
		async_throw_error("Cannot listen for signals outside of an async context");
		RETURN_THROWS();
	}

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

	return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(async)
{
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

static zend_module_entry async_module = { /* {{{ */
	STANDARD_MODULE_HEADER,
	"PHP True Asynchrony",
	ext_functions,
	ZEND_MINIT(async),
	ZEND_MSHUTDOWN(async),
	NULL,
	NULL,
	PHP_MINFO(async_info),
	PHP_ASYNC_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */


zend_result async_register_module(void) /* {{{ */
{
	zend_module_entry *module;

	EG(current_module) = module = zend_register_module_ex(&async_module, MODULE_PERSISTENT);

	if (UNEXPECTED(module == NULL)) {
		return FAILURE;
	}

	ZEND_ASSERT(module->module_number == 0);

	return SUCCESS;
}
/* }}} */