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
#include "module_entry_arginfo.h"
#include "walker_arginfo.h"

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

PHP_METHOD(Async_Walker, walk)
{
	zval * iterable;
	zval * function;
	zval * custom_data = NULL;
	zval * defer = NULL;

	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_ZVAL(iterable)
		Z_PARAM_ZVAL(function)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(custom_data)
		Z_PARAM_ZVAL(defer)
	ZEND_PARSE_PARAMETERS_END();

	if (!zend_is_callable(function, 0, NULL)) {
		zend_argument_value_error(2, "Expected parameter to be a valid callable");
		RETURN_THROWS();
	}

	if (defer != NULL && Z_TYPE_P(defer) != IS_NULL) {
		if (!zend_is_callable(defer, 0, NULL)) {
			zend_argument_value_error(3, "Expected parameter to be a valid callable");
			RETURN_THROWS();
		}
	}

	HashPosition position	= 0;
	uint32_t hash_iterator	= -1;
	zend_object_iterator *zend_iterator = NULL;

	if (Z_TYPE_P(iterable) == IS_ARRAY) {
		if (zend_hash_num_elements(Z_ARR_P(iterable)) == 0) {
			return;
		}

		zend_hash_internal_pointer_reset_ex(Z_ARR_P(iterable), &position);
		hash_iterator = zend_hash_iterator_add(Z_ARR_P(iterable), position);

	} else if (Z_TYPE_P(iterable) == IS_OBJECT && Z_OBJCE_P(iterable)->get_iterator) {
		zend_iterator = Z_OBJCE_P(iterable)->get_iterator(Z_OBJCE_P(iterable), iterable, 0);

		if (UNEXPECTED(EG(exception) || zend_iterator == NULL)) {
			RETURN_THROWS();
		}

		if (zend_iterator->funcs->rewind) {
			zend_iterator->funcs->rewind(zend_iterator);
		}
	} else {
		zend_argument_value_error(1, "Expected parameter to be an array or an object implementing Traversable");
		RETURN_THROWS();
	}

	async_walker_t * walker = zend_object_alloc_ex(sizeof(async_walker_t), async_ce_walker);
	zend_object_std_init(&walker->std, async_ce_walker);
	object_properties_init(&walker->std, async_ce_walker);

	if (Z_TYPE_P(iterable) == IS_ARRAY) {
		ZVAL_COPY(&walker->iterator, iterable);
		walker->target_hash = Z_ARRVAL_P(iterable);
		walker->position = position;
		walker->hash_iterator = hash_iterator;
	} else {
		ZVAL_COPY(&walker->iterator, iterable);
		walker->zend_iterator = zend_iterator;
	}

	if (custom_data != NULL) {
		ZVAL_COPY(&walker->custom_data, custom_data);
	} else {
		ZVAL_NULL(&walker->custom_data);
	}

	if (defer != NULL) {
		ZVAL_COPY(&walker->defer, defer);
	} else {
		ZVAL_NULL(&walker->defer);
	}

	if (UNEXPECTED(zend_fcall_info_init(function, 0, &walker->fci, &walker->fcc, NULL, NULL) != SUCCESS)) {
		zend_throw_error(NULL, "Failed to initialize fcall info");
		RETURN_THROWS();
	}

	// Keep a reference to closures or callable objects while the walker is running.
	Z_TRY_ADDREF(walker->fci.function_name);

	if (UNEXPECTED(walker->fcc.function_handler->common.num_args > 3)) {
		zend_argument_value_error(1, "The callback function must have at most 3 parameters");
		RETURN_THROWS();
	}

	zend_function * run_method = zend_hash_str_find_ptr(&async_ce_walker->function_table, "run", sizeof("run") - 1);

	ZEND_ASSERT(run_method != NULL && "Method run not found");

	zval z_closure;
	zval this_ptr;
	ZVAL_OBJ(&this_ptr, &walker->std);
	zend_create_closure(&z_closure, run_method, async_ce_walker, async_ce_walker, &this_ptr);
	walker->run_closure = Z_OBJ(z_closure);

	zend_function * next_method = zend_hash_str_find_ptr(&async_ce_walker->function_table, "next", sizeof("next") - 1);

	ZEND_ASSERT(run_method != NULL && "Method next not found");

	zend_create_closure(&z_closure, next_method, async_ce_walker, async_ce_walker, &this_ptr);
	walker->next_microtask = async_scheduler_create_microtask(&z_closure);
	zval_ptr_dtor(&z_closure);

	if (UNEXPECTED(walker->next_microtask == NULL)) {
		zend_throw_error(NULL, "Failed to create microtask for next method");
		RETURN_THROWS();
	}

	zval zval_fiber;
	zval params[1];

	ZVAL_OBJ(&params[0], walker->run_closure);

	if (object_init_with_constructor(&zval_fiber, zend_ce_fiber, 1, params, NULL) == FAILURE) {
		RETURN_THROWS();
	}

	// Transfer run_closure ownership to the fiber
	GC_DELREF(walker->run_closure);
	GC_DELREF(&walker->std);
	GC_DELREF(&walker->std);

	async_start_fiber((zend_fiber *) Z_OBJ(zval_fiber), NULL, 0, NULL);
}

PHP_METHOD(Async_Walker, run)
{
	async_walker_t * walker = (async_walker_t *) Z_OBJ_P(getThis());

	if (Z_TYPE(walker->is_finished) == IS_TRUE) {
        return;
    }

	/**
	 * callback function
	 * function(mixed $value, mixed $key, mixed $custom_data) : bool {}
	 **/

	bool is_continue = true;
	zend_result result = SUCCESS;
	zval args[3], retval;
	zval * current;
	ZVAL_UNDEF(&args[0]);
	ZVAL_UNDEF(&args[1]);
	ZVAL_COPY_VALUE(&args[2], &walker->custom_data);

	// Copy the fci to avoid overwriting the original
	// Because the another fiber may be started in the callback function
	zend_fcall_info fci = walker->fci;

	fci.retval = &retval;
	fci.param_count = walker->fcc.function_handler->common.num_args;
	fci.params = args;

	if (walker->next_microtask != NULL) {
		async_scheduler_add_microtask_ex(walker->next_microtask);
	}

	/* Reload array and position */
	if (walker->target_hash != NULL) {
		walker->position = zend_hash_iterator_pos_ex(walker->hash_iterator, &walker->iterator);
		walker->target_hash = Z_ARRVAL(walker->iterator);
	}

	while (Z_TYPE(walker->is_finished) != IS_TRUE && is_continue) {

		if (walker->target_hash != NULL) {
			current = zend_hash_get_current_data_ex(walker->target_hash, &walker->position);
		} else if (SUCCESS == walker->zend_iterator->funcs->valid(walker->zend_iterator)) {
			current = walker->zend_iterator->funcs->get_current_data(walker->zend_iterator);
		} else {
			current = NULL;
		}

		if (current == NULL) {
			ZVAL_TRUE(&walker->is_finished);
			break;
		}

		/* Skip undefined indirect elements */
		if (Z_TYPE_P(current) == IS_INDIRECT) {
			current = Z_INDIRECT_P(current);
			if (Z_TYPE_P(current) == IS_UNDEF) {
				if (Z_TYPE(walker->iterator) == IS_ARRAY) {
                    zend_hash_move_forward(Z_ARR(walker->iterator));
                } else {
                    walker->zend_iterator->funcs->move_forward(walker->zend_iterator);
                }

				continue;
			}
		}

		/* Ensure the value is a reference. Otherwise, the location of the value may be freed. */
		ZVAL_MAKE_REF(current);
		ZVAL_COPY(&args[0], current);

		/* Retrieve key */
		if (walker->target_hash != NULL) {
			zend_hash_get_current_key_zval_ex(walker->target_hash, &args[1], &walker->position);
        } else {
            walker->zend_iterator->funcs->get_current_key(walker->zend_iterator, &args[1]);
        }

		/* Move to next element already now -- this mirrors the approach used by foreach
		 * and ensures proper behavior with regard to modifications. */
	    if (walker->target_hash != NULL) {
            zend_hash_move_forward_ex(walker->target_hash, &walker->position);
	    	// And update the iterator position
	    	EG(ht_iterators)[walker->hash_iterator].pos = walker->position;
        } else {
            walker->zend_iterator->funcs->move_forward(walker->zend_iterator);
        }

		/* Call the userland function */
		result = zend_call_function(&fci, &walker->fcc);

		if (result == SUCCESS) {

			if (Z_TYPE(retval) == IS_FALSE) {
                is_continue = false;
            }

			zval_ptr_dtor(&retval);

			/* Reload array and position */
			if (walker->target_hash != NULL) {
				walker->position = zend_hash_iterator_pos_ex(walker->hash_iterator, &walker->iterator);
				walker->target_hash = Z_ARRVAL(walker->iterator);
			}
		}

		zval_ptr_dtor(&args[0]);

		if (Z_TYPE(args[1]) != IS_UNDEF) {
			zval_ptr_dtor(&args[1]);
			ZVAL_UNDEF(&args[1]);
		}

		if (UNEXPECTED(result == FAILURE || EG(exception) != NULL)) {
            break;
        }
	}

	if (walker->hash_iterator != -1) {
        zend_hash_iterator_del(walker->hash_iterator);
		walker->hash_iterator = -1;
		walker->target_hash = NULL;
    }

	if (Z_TYPE(walker->defer) != IS_NULL) {

		zval defer;
		ZVAL_COPY_VALUE(&defer, &walker->defer);
		ZVAL_NULL(&walker->defer);

		zend_exception_save();

		// Call user callable function
		ZVAL_UNDEF(&retval);
		ZVAL_COPY_VALUE(&args[0], &walker->custom_data);
		call_user_function(EG(function_table), NULL, &defer, &retval, 1, args);
		zval_ptr_dtor(&args[0]);
		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&defer);

		zend_exception_restore();
	}
}

PHP_METHOD(Async_Walker, next)
{
	// This method is called by the scheduler to continue the foreach loop
	async_walker_t * walker = (async_walker_t *) Z_OBJ_P(getThis());

	if (Z_TYPE(walker->is_finished) == IS_TRUE) {
		return;
	}

	// We just need to create a new fiber to continue the loop
	zval zval_fiber;
	zval params[1];

	ZVAL_OBJ(&params[0], walker->run_closure);

	if (object_init_with_constructor(&zval_fiber, zend_ce_fiber, 1, params, NULL) == FAILURE) {
		RETURN_THROWS();
	}

	async_start_fiber((zend_fiber *) Z_OBJ(zval_fiber), NULL, 0, NULL);
}

PHP_METHOD(Async_Walker, cancel)
{
	async_walker_t * executor = (async_walker_t *) Z_OBJ_P(getThis());

    if (Z_TYPE(executor->is_finished) == IS_TRUE) {
        return;
    }

    ZVAL_TRUE(&executor->is_finished);
}

static void async_walker_object_destroy(zend_object* object)
{
	async_walker_t * walker = (async_walker_t *) object;

	/* Cleanup callback and unset field to prevent GC / duplicate dtor issues. */
	zval_ptr_dtor(&walker->fci.function_name);
	ZVAL_UNDEF(&walker->fci.function_name);

	if (walker->zend_iterator != NULL) {
		zend_iterator_dtor(walker->zend_iterator);
		walker->zend_iterator = NULL;
	}

	if (walker->run_closure != NULL) {
		/*
		if (!(OBJ_FLAGS(walker->run_closure) & IS_OBJ_DESTRUCTOR_CALLED)) {
			// Add fake reference to prevent double free
			GC_ADDREF(&walker->std);
			OBJ_RELEASE(walker->run_closure);
		}
		*/

		walker->run_closure = NULL;
    }

	if (walker->next_microtask != NULL) {
		GC_ADDREF(&walker->std);
		async_scheduler_microtask_free(walker->next_microtask);
		walker->next_microtask = NULL;
	}

	ZEND_ASSERT(walker->hash_iterator == -1 && "Iterator should be removed");
}

static zend_object_handlers async_walker_handlers;

static void async_register_walker_ce(void)
{
	async_ce_walker = register_class_Async_Walker();

	async_ce_walker->default_object_handlers = &async_walker_handlers;

	async_walker_handlers = std_object_handlers;
	async_walker_handlers.dtor_obj = async_walker_object_destroy;
	async_walker_handlers.clone_obj = NULL;
}

ZEND_MINIT_FUNCTION(async)
{
	async_register_walker_ce();
	async_register_callback_ce();
	async_register_notifier_ce();
	async_register_handlers_ce();
	async_register_resume_ce();
	async_register_exceptions_ce();
	async_register_channel_ce();

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
