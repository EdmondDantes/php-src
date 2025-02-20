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

#include <zend_closures.h>

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
#include "walker.h"
#include "module_entry_arginfo.h"


#define THROW_IF_REACTOR_DISABLED if (UNEXPECTED(false == reactor_is_enabled())) {			\
		async_throw_error("The operation is not available without an enabled reactor");		\
		RETURN_THROWS();																	\
	}

#define THROW_IF_SHUTDOWN if (UNEXPECTED(ASYNC_G(graceful_shutdown))) {						\
		async_throw_error("The operation cannot be executed during a graceful shutdown");	\
		RETURN_THROWS();																	\
	}

PHP_FUNCTION(Async_launchScheduler)
{
	async_scheduler_launch();
}

PHP_FUNCTION(Async_wait)
{
	THROW_IF_SHUTDOWN;

	zval *resume = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_OBJECT_OF_CLASS_OR_NULL(resume, async_ce_resume)
	ZEND_PARSE_PARAMETERS_END();

	if (UNEXPECTED(IS_ASYNC_OFF && resume != NULL)) {
		async_throw_error("Cannot await a resume object outside of an async context");
		RETURN_THROWS();
	}

	async_wait((resume == NULL || ZVAL_IS_NULL(resume)) ? NULL: (async_resume_t *) Z_OBJ_P(resume));
}

PHP_FUNCTION(Async_run)
{
	if (UNEXPECTED(ASYNC_G(graceful_shutdown))) {
		zend_error(E_CORE_WARNING, "Cannot run the scheduler during a graceful shutdown");
		return;
	}

	zval * callable;

	zval *args = NULL;
	int args_count = 0;
	HashTable *named_args = NULL;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_ZVAL(callable);
		Z_PARAM_VARIADIC_WITH_NAMED(args, args_count, named_args);
	ZEND_PARSE_PARAMETERS_END();

	zval zval_fiber;
	zval params[1];

	ZVAL_COPY_VALUE(&params[0], callable);

	if (object_init_with_constructor(&zval_fiber, zend_ce_fiber, 1, params, NULL) == FAILURE) {
		RETURN_THROWS();
	}

	// Transfer fiber ownership to the scheduler
	// (no need to release the fiber handle in this case)
	async_start_fiber((zend_fiber *) Z_OBJ(zval_fiber), args, args_count, named_args);
}

zend_always_inline void create_fiber_with_handle(INTERNAL_FUNCTION_PARAMETERS)
{
	THROW_IF_SHUTDOWN;

	zval * callable;

	zval *args = NULL;
	int args_count = 0;
	HashTable *named_args = NULL;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_ZVAL(callable);
		Z_PARAM_VARIADIC_WITH_NAMED(args, args_count, named_args);
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

	// Transfer fiber ownership to the scheduler
	// (no need to release the fiber handle in this case)
	async_start_fiber(fiber_handle->fiber, args, args_count, named_args);

	RETURN_OBJ(&fiber_handle->handle.std);
}

PHP_FUNCTION(Async_async)
{
    create_fiber_with_handle(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_FUNCTION(Async_await)
{
	THROW_IF_SHUTDOWN;

	bool is_fiber_handle_owned = false;
	zval * callable;
	reactor_fiber_handle_t * fiber_handle = NULL;

	zval *args = NULL;
	int args_count = 0;
	HashTable *named_args = NULL;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_ZVAL(callable);
		Z_PARAM_VARIADIC_WITH_NAMED(args, args_count, named_args);
	ZEND_PARSE_PARAMETERS_END();

	if (Z_OBJCE_P(callable) == zend_ce_fiber) {

		if (((zend_fiber *) Z_OBJ_P(callable))->context.status == ZEND_FIBER_STATUS_DEAD) {
            async_throw_error("Cannot await a terminated fiber. Use FiberHandle instead");
            RETURN_THROWS();
        }

		fiber_handle = async_fiber_handle_new((zend_fiber *) Z_OBJ_P(callable));
		is_fiber_handle_owned = true;
	} else if (Z_OBJCE_P(callable) == async_ce_fiber_handle) {
		fiber_handle = (reactor_fiber_handle_t *) Z_OBJ_P(callable);
    } else {
    	create_fiber_with_handle(INTERNAL_FUNCTION_PARAM_PASSTHRU);

    	if (EG(exception) == NULL && Z_TYPE_P(return_value) == IS_OBJECT && Z_OBJCE_P(return_value) == async_ce_fiber_handle) {
    		fiber_handle = (reactor_fiber_handle_t *) Z_OBJ_P(return_value);
    		ZVAL_UNDEF(return_value);
    	}

    	is_fiber_handle_owned = true;
    }

	if (UNEXPECTED(EG(exception) != NULL)) {
		RETURN_THROWS();
	}

	if (Z_TYPE(fiber_handle->handle.is_closed) == IS_TRUE) {
		if (fiber_handle->exception != NULL) {
			zend_throw_exception_internal(fiber_handle->exception);
		} else {
			RETURN_ZVAL(&fiber_handle->fiber->result, 1, 0);
		}
    }

	async_resume_t * resume = async_resume_new(NULL);

	if (UNEXPECTED(EG(exception) != NULL)) {
		OBJ_RELEASE(&fiber_handle->handle.std);
        RETURN_THROWS();
    }

	async_resume_when(resume, &fiber_handle->handle, false, async_resume_when_callback_resolve);

	async_wait(resume);

	// Return the result of the fiber execution
	if (EXPECTED(EG(exception) == NULL)) {
		ZVAL_COPY(return_value, &fiber_handle->fiber->result);
	}

	OBJ_RELEASE(&resume->std);

	if (is_fiber_handle_owned && fiber_handle != NULL) {
        OBJ_RELEASE(&fiber_handle->handle.std);
    }
}

PHP_FUNCTION(Async_awaitFirst)
{
	THROW_IF_REACTOR_DISABLED

	zval * futures;
	zend_bool ignoreErrors = false;
	zend_object * cancellation = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_ZVAL(futures);
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(ignoreErrors);
		Z_PARAM_OBJ_OF_CLASS_OR_NULL(cancellation, async_ce_cancellation);
	ZEND_PARSE_PARAMETERS_END();

	HashTable * results = zend_new_array(8);

	async_await_future_list(
		futures,
		1,
		ignoreErrors,
		(reactor_notifier_t *) cancellation,
		0,
		results,
		NULL
	);

	if (EG(exception)) {
		zend_array_release(results);
		RETURN_THROWS();
	}

	if (zend_hash_num_elements(results) == 0) {
		zend_array_release(results);
		RETURN_NULL();
	}

	zval result;
	ZVAL_COPY(&result, zend_hash_index_find(results, 0));
	zend_array_release(results);

	RETURN_ZVAL(&result, 0, 0);
}

PHP_FUNCTION(Async_awaitAnyN)
{
	THROW_IF_REACTOR_DISABLED

	zend_long count;
	zval * futures;
	zend_bool ignoreErrors = false;
	zend_object * cancellation = NULL;

	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_LONG(count);
		Z_PARAM_ZVAL(futures);
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(ignoreErrors);
		Z_PARAM_OBJ_OF_CLASS_OR_NULL(cancellation, async_ce_cancellation);
	ZEND_PARSE_PARAMETERS_END();

	if (count < 0) {
		zend_argument_value_error(1, "The count value must be greater than or equal to 0");
		RETURN_THROWS();
	}

	HashTable * results = zend_new_array(8);
	HashTable * errors = zend_new_array(8);

	async_await_future_list(
		futures,
		(int) count,
		ignoreErrors,
		(reactor_notifier_t *) cancellation,
		0,
		results,
		errors
	);

	if (EG(exception)) {
		zend_array_release(results);
		zend_array_release(errors);
		RETURN_THROWS();
	}

	HashTable * return_array = zend_new_array(2);

	zval val;
	ZVAL_ARR(&val, results);
	zend_hash_next_index_insert_new(return_array, &val);

	ZVAL_ARR(&val, errors);
	zend_hash_next_index_insert_new(return_array, &val);

	RETURN_ARR(return_array);
}

PHP_FUNCTION(Async_awaitAll)
{
	THROW_IF_REACTOR_DISABLED

	zval * futures;
	zend_bool ignoreErrors = false;
	zend_object * cancellation = NULL;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_ZVAL(futures);
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(ignoreErrors);
		Z_PARAM_OBJ_OF_CLASS_OR_NULL(cancellation, async_ce_cancellation);
	ZEND_PARSE_PARAMETERS_END();

	HashTable * results = zend_new_array(8);
	HashTable * errors = zend_new_array(8);

	async_await_future_list(
		futures,
		0,
		ignoreErrors,
		(reactor_notifier_t *) cancellation,
		0,
		results,
		errors
	);

	if (EG(exception)) {
		zend_array_release(results);
		zend_array_release(errors);
		RETURN_THROWS();
	}

	HashTable * return_array = zend_new_array(2);

	zval val;
	ZVAL_ARR(&val, results);
	zend_hash_next_index_insert_new(return_array, &val);

	ZVAL_ARR(&val, errors);
	zend_hash_next_index_insert_new(return_array, &val);

	RETURN_ARR(return_array);
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
		zend_argument_value_error(1, "The timeout value must be greater than or equal to 0");
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

	reactor_add_handle(handle);
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
		zend_argument_value_error(1, "The timeout value must be greater than or equal to 0");
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

PHP_FUNCTION(Async_exec)
{

}

PHP_FUNCTION(Async_getFibers)
{

}

PHP_FUNCTION(Async_getResumes)
{

}

PHP_FUNCTION(Async_gracefulShutdown)
{

}

ZEND_MINIT_FUNCTION(async)
{
	async_register_callback_ce();
	async_register_notifier_ce();
	async_register_handlers_ce();
	async_register_resume_ce();
	async_register_exceptions_ce();
	async_register_channel_ce();
	async_register_walker_ce();

	async_scheduler_startup();

#ifdef PHP_ASYNC_LIBUV
	async_libuv_startup();
#endif

	return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(async)
{
	async_scheduler_shutdown();

#ifdef PHP_ASYNC_LIBUV
	async_libuv_shutdown();
#endif
	return SUCCESS;
}

PHP_MINFO_FUNCTION(async_info) {
	php_info_print_table_start();
	php_info_print_table_header(2, "Module", "TrueAsync");
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
	async_globals->graceful_shutdown = false;
	async_globals->event_handle_count = 0;
	async_globals->exit_exception = NULL;
	async_globals->reactor = NULL;
	async_globals->exception_handler = NULL;
	async_globals->execute_callbacks_handler = NULL;
	async_globals->execute_next_fiber_handler = NULL;
	async_globals->execute_microtasks_handler = NULL;
	async_globals->callbacks_fiber = NULL;
	async_globals->microtask_fiber = NULL;

	async_globals_ctor(async_globals);
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION */
static PHP_GSHUTDOWN_FUNCTION(async)
{
}

PHP_RINIT_FUNCTION(async) /* {{{ */
{
	async_host_name_list_ctor();
	return SUCCESS;
} /* }}} */

PHP_RSHUTDOWN_FUNCTION(async) /* {{{ */
{
	async_globals_dtor(ASYNC_GLOBAL);
	async_host_name_list_dtor();
	return SUCCESS;
} /* }}} */

zend_module_entry async_module_entry = { /* {{{ */
	STANDARD_MODULE_HEADER,
	"PHP TrueAsync",
	ext_functions,
	ZEND_MINIT(async),
	ZEND_MSHUTDOWN(async),
	PHP_RINIT(async),
	PHP_RSHUTDOWN(async),
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
