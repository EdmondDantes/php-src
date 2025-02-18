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
#include "walker.h"

#include <zend_closures.h>
#include <zend_fibers.h>
#include <async/php_scheduler.h>
#include <async/php_async.h>

#include "future.h"
#include "walker_arginfo.h"
#include "zend_common.h"

PHP_METHOD(Async_Walker, walk)
{
	zval * iterable;
	zval * function;
	zval * custom_data = NULL;
	zval * defer = NULL;
	zend_long concurrency = 0;

	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_ZVAL(iterable)
		Z_PARAM_ZVAL(function)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(custom_data)
		Z_PARAM_ZVAL(defer)
		Z_PARAM_LONG(concurrency)
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

	if (concurrency > 32000) {
		zend_argument_value_error(4, "The concurrency value must be less than or equal to 32000");
		RETURN_THROWS();
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

	walker->concurrency = (int) concurrency;

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

	if (walker->next_microtask != NULL && (walker->concurrency > 0 && walker->concurrency < walker->active_fibers)) {
		async_scheduler_add_microtask_ex(walker->next_microtask);
	}

	walker->active_fibers++;

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

	if (walker->active_fibers != 0) {
		walker->active_fibers--;
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

PHP_METHOD(Async_Walker, getFuture)
{
	async_walker_t * walker = (async_walker_t *) Z_OBJ_P(getThis());

	if (walker->future_state != NULL) {
		RETURN_OBJ(async_future_new(&walker->future_state->notifier.std));
	}

	async_future_state_t * state = (async_future_state_t *) async_future_state_new();
	walker->future_state = state;

	if (Z_TYPE(walker->is_finished) == IS_TRUE) {
		async_future_state_resolve(state, &walker->is_finished);
	}

	RETURN_OBJ(async_future_new(&state->notifier.std));
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