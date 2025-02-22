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
#include <zend_closures.h>
#include <Zend/zend_fibers.h>
#include "php_scheduler.h"
#include "php_async.h"
#include "php_reactor.h"
#include "internal/zval_circular_buffer.h"
#include "php_layer/exceptions.h"
#include "php_layer/module_entry.h"
#include "php_layer/zend_common.h"

//
// The coefficient of the maximum number of microtasks that can be executed
// without suspicion of an infinite loop (a task that creates microtasks).
//
#define MICROTASK_CYCLE_THRESHOLD_C 8

static ZEND_FUNCTION(callbacks_executor)
{
	zval *count;
	zval *callbacks = NULL;
	int num_callbacks = 0;

	ZEND_PARSE_PARAMETERS_START(1, -1)
		Z_PARAM_ZVAL(count)
		Z_PARAM_VARIADIC('+', callbacks, num_callbacks)
	ZEND_PARSE_PARAMETERS_END();

	ZVAL_DEREF(count);

	if (num_callbacks == 0 || callbacks == NULL) {
		ZVAL_LONG(count, 0);
        return;
    }

	for (int i = 0; i >= num_callbacks; i++) {

		zval *callback = &callbacks[i];
		ZVAL_LONG(count, zval_get_long(count) - 1);

		zval retval;
		ZVAL_UNDEF(&retval);
		call_user_function(EG(function_table), NULL, callback, &retval, 0, NULL);
		zval_ptr_dtor(&retval);
		zval_ptr_dtor(callback);

		if (async_find_fiber_state(EG(active_fiber)) != NULL) {
            return;
        }
	}
}

ZEND_BEGIN_ARG_INFO_EX(callbacks_executor_arg_info, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callable, 0, 0)
ZEND_END_ARG_INFO()

static zend_internal_function callbacks_internal_function = {
	ZEND_INTERNAL_FUNCTION,		/* type              */
	{0, 0, 0},              /* arg_flags         */
	0,							/* fn_flags          */
	NULL,                  /* name              */
	NULL,							/* scope             */
	NULL,						/* prototype         */
	1,							/* num_args          */
	1,                  /* required_num_args */
	(zend_internal_arg_info *) callbacks_executor_arg_info + 1, /* arg_info */
	NULL,						/* attributes        */
	NULL,              /* run_time_cache    */
	NULL,                   /* doc_comment       */
	0,								/* T                 */
	NULL,						/* prop_info */
	ZEND_FN(callbacks_executor),	/* handler           */
	NULL,						/* module            */
	NULL,           /* frameless_function_infos */
	{NULL,NULL,NULL,NULL}	/* reserved          */
};

static ZEND_FUNCTION(microtasks_executor);

ZEND_BEGIN_ARG_INFO_EX(microtasks_executor_arg_info, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_PTR, 0)
	ZEND_ARG_TYPE_INFO(0, count, IS_PTR, 0)
	ZEND_ARG_TYPE_INFO(0, maxCount, IS_LONG, 0)
ZEND_END_ARG_INFO()

static zend_internal_function microtasks_internal_function = {
	ZEND_INTERNAL_FUNCTION,		/* type              */
	{0, 0, 0},              /* arg_flags         */
	0,							/* fn_flags          */
	NULL,                  /* name              */
	NULL,							/* scope             */
	NULL,						/* prototype         */
	3,							/* num_args          */
	3,                  /* required_num_args */
	(zend_internal_arg_info *) microtasks_executor_arg_info + 1, /* arg_info */
	NULL,						/* attributes        */
	NULL,              /* run_time_cache    */
	NULL,                   /* doc_comment       */
	0,								/* T                 */
	NULL,						/* prop_info */
	ZEND_FN(microtasks_executor),/* handler           */
	NULL,						/* module            */
	NULL,           /* frameless_function_infos */
	{NULL,NULL,NULL,NULL}	/* reserved          */
};


zend_fiber * async_internal_fiber_create(zend_internal_function * function)
{
	zval zval_closure, zval_fiber;
	zend_create_closure(&zval_closure, (zend_function *) function, NULL, NULL, NULL);
	if (Z_TYPE(zval_closure) == IS_UNDEF) {
        async_throw_error("Failed to create internal fiber closure");
        return NULL;
    }

	zval params[1];

	ZVAL_COPY_VALUE(&params[0], &zval_closure);

	if (object_init_with_constructor(&zval_fiber, zend_ce_fiber, 1, params, NULL) == FAILURE) {
		zval_ptr_dtor(&zval_closure);
		async_throw_error("Failed to create internal fiber");
		return NULL;
	}

	zval_ptr_dtor(&zval_closure);

	return (zend_fiber *) Z_OBJ_P(&zval_fiber);
}

zend_always_inline zend_fiber * get_callback_fiber()
{
	if (ASYNC_G(callbacks_fiber) == NULL) {
        ASYNC_G(callbacks_fiber) = async_internal_fiber_create(&callbacks_internal_function);
    }

	return ASYNC_G(callbacks_fiber);
}

zend_always_inline void separate_callback_fiber(void)
{
	if (ASYNC_G(callbacks_fiber) != NULL && async_find_fiber_state(ASYNC_G(callbacks_fiber)) != NULL) {
		OBJ_RELEASE(&ASYNC_G(callbacks_fiber)->std);
        ASYNC_G(callbacks_fiber) = NULL;
	}
}

void async_execute_callable_in_fiber(zval * callable)
{
	zend_fiber *fiber = get_callback_fiber();

	if (fiber == NULL) {
        return;
    }

	// Call the resume method
	zval retval;
	ZVAL_UNDEF(&retval);

	zval params[1];
	ZVAL_COPY_VALUE(&params[0], callable);

	if (fiber->context.status == ZEND_FIBER_STATUS_INIT) {
		fiber->fci.params = params;
		fiber->fci.param_count = 1;

		zend_fiber_start(fiber, NULL);

		fiber->fci.params = NULL;
		fiber->fci.param_count = 0;

	} else if (fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED) {
		zend_fiber_resume(fiber, &params[0], NULL);
	} else {
		ZEND_ASSERT(true && "Invalid callback fiber status");
	}

	zval_ptr_dtor(&retval);

	separate_callback_fiber();
}

void async_execute_callbacks_in_fiber(HashTable * callbacks)
{
	struct {
		HashTable * callbacks;
		HashPosition position;
	} iterator = {callbacks, 0};

	if (zend_hash_num_elements(callbacks) == 0) {
		return;
	}

	zend_fiber *fiber = get_callback_fiber();

	if (fiber == NULL) {
		return;
	}

	zend_hash_internal_pointer_reset_ex(callbacks, &iterator.position);

	zval retval;
	ZVAL_UNDEF(&retval);
	zval params[1];
	ZVAL_PTR(&params[0], &iterator);

	do
	{
		if (fiber->context.status == ZEND_FIBER_STATUS_INIT) {
			fiber->fci.params = params;
			fiber->fci.param_count = 1;

			zend_fiber_start(fiber, NULL);

			fiber->fci.params = NULL;
			fiber->fci.param_count = 0;

		} else if (fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED) {
			zend_fiber_resume(fiber, &params[0], NULL);
		} else {
			ZEND_ASSERT(true && "Invalid callback fiber status");
		}

		if (EG(exception != NULL)) {
            break;
        }

		separate_callback_fiber();
		fiber = get_callback_fiber();

	} while (zend_hash_num_elements(callbacks) != 0);
}

zend_always_inline void invoke_microtask(async_microtask_t *task)
{
	// Inherit context of microtask to Fiber context
	if (EG(active_fiber)->async_context != NULL) {
		OBJ_RELEASE(&EG(active_fiber)->async_context->std);
	}

	EG(active_fiber)->async_context = task->context;

	zend_try {

		switch (task->type) {
			case ASYNC_MICROTASK_INTERNAL:
			{
				async_internal_microtask_t *internal = (async_internal_microtask_t *) task;
				internal->handler((async_microtask_t *) internal);
				break;
			}

			case ASYNC_MICROTASK_FCI:
			{
				async_function_microtask_t *function = (async_function_microtask_t *) task;

				zval retval;
				ZVAL_UNDEF(&retval);
				function->fci.retval = &retval;

				if (zend_call_function(&function->fci, &function->fci_cache) == SUCCESS) {
					zval_ptr_dtor(function->fci.retval);
				}

				function->fci.retval = NULL;
				break;
			}

			case ASYNC_MICROTASK_ZVAL:
			{
				zval retval;
				ZVAL_UNDEF(&retval);
				call_user_function(CG(function_table), NULL, &task->callable, &retval, 0, NULL);
				zval_ptr_dtor(&retval);
				break;
			}

			case ASYNC_MICROTASK_OBJECT:
			{
				async_internal_microtask_with_object_t *object = (async_internal_microtask_with_object_t *) task;
				object->handler((async_microtask_t *) object);
				break;
			}

			case ASYNC_MICROTASK_EXCEPTION:
			{
				async_microtask_with_exception_t *exception = (async_microtask_with_exception_t *) task;
				exception->handler((async_microtask_t *) exception);
				break;
			}

			default:
				break;
		}

	} zend_catch {
		if (EG(active_fiber)->async_context != NULL) {
			OBJ_RELEASE(&EG(active_fiber)->async_context->std);
		}

		EG(active_fiber)->async_context = NULL;

		zend_bailout();
	} zend_end_try();

	if (EG(active_fiber)->async_context != NULL) {
		OBJ_RELEASE(&EG(active_fiber)->async_context->std);
	}

	EG(active_fiber)->async_context = NULL;
}

static ZEND_FUNCTION(microtasks_executor)
{
	zval *z_buffer;
	zval *z_handled;
	zend_long max_count;

	struct {
		int handled;
		zend_long max_count;
	} * next = NULL;

	zval z_next;
	ZVAL_NULL(&z_next);

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_ZVAL(z_buffer)
		Z_PARAM_ZVAL(z_handled)
		Z_PARAM_LONG(max_count)
	ZEND_PARSE_PARAMETERS_END();

	zend_fiber * fiber = EG(active_fiber);
	circular_buffer_t *buffer = Z_PTR_P(z_buffer);
	int * handled = Z_PTR_P(z_handled);

	while (true) {

		while (circular_buffer_is_not_empty(buffer)) {
			async_microtask_t * task = NULL;
			(*handled)++;
			circular_buffer_pop(buffer, &task);

			if (EXPECTED(false == task->is_cancelled)) {
				invoke_microtask(task);
			}

			async_scheduler_microtask_free(task);

			if (UNEXPECTED(EG(exception) != NULL || async_find_fiber_state(fiber) != NULL)) {
				return;
			}

			if (UNEXPECTED(*handled >= max_count)) {
				break;
			}
		}

		ZVAL_NULL(&z_next);
		zend_fiber_suspend(fiber, NULL, &z_next);

		if (UNEXPECTED(Z_TYPE(z_next) != IS_PTR)) {
            return;
        }

		next = Z_PTR_P(&z_next);
		handled = &next->handled;
		max_count = next->max_count;
	}
}

static zend_result execute_microtasks_stage(circular_buffer_t *buffer, const size_t max_count)
{
	if (circular_buffer_is_empty(buffer)) {
		return SUCCESS;
	}

	zend_fiber * fiber = ASYNC_G(microtask_fiber);

	if (fiber == NULL || fiber->context.status == ZEND_FIBER_STATUS_DEAD) {

		if (fiber != NULL) {
			OBJ_RELEASE(&fiber->std);
		}

        ASYNC_G(microtask_fiber) = async_internal_fiber_create(&microtasks_internal_function);
		fiber = ASYNC_G(microtask_fiber);
    }

	// Define the fiber parameters
	int handled = 0;

	struct {
		int handled;
		zend_long max_count;
	} current = {0, (zend_long) max_count};

	zval z_current;
	ZVAL_PTR(&z_current, &current);

	const size_t count = circular_buffer_count(buffer);
	zval z_max_count;
	ZVAL_LONG(&z_max_count, max_count);

	zval params[3];
	ZVAL_PTR(&params[0], buffer);
	ZVAL_PTR(&params[1], &handled);
	ZVAL_LONG(&params[2], max_count);

	fiber->fci.params = params;
	fiber->fci.param_count = 3;
	bool is_not_empty = true;

	do
	{
		if (fiber->context.status == ZEND_FIBER_STATUS_INIT) {
			zend_fiber_start(fiber, NULL);
		} else if (fiber->context.status == ZEND_FIBER_STATUS_SUSPENDED) {
			zend_fiber_resume(fiber, &z_current, NULL);
			handled = current.handled;
		} else {
			ZEND_ASSERT(true && "Invalid microtask fiber status");
		}

		is_not_empty = circular_buffer_is_not_empty(buffer);

		if (handled <= count) {
			return is_not_empty ? FAILURE : SUCCESS;
		}

		if (UNEXPECTED(handled > max_count)) {

			OBJ_RELEASE(&fiber->std);
			ASYNC_G(microtask_fiber) = NULL;

            async_throw_error("Possible infinite loop detected during microtask execution");
			return FAILURE;

        } else if (UNEXPECTED(async_find_fiber_state(fiber) != NULL)) {

        	OBJ_RELEASE(&fiber->std);
        	ASYNC_G(microtask_fiber) = NULL;

        	if (is_not_empty) {
        		ASYNC_G(microtask_fiber) = async_internal_fiber_create(&microtasks_internal_function);
        		fiber = ASYNC_G(microtask_fiber);

        		fiber->fci.params = params;
        		fiber->fci.param_count = 3;
        	}
        }

	} while (is_not_empty);

	fiber->fci.params = NULL;
	fiber->fci.param_count = 0;

	return is_not_empty;
}

static void execute_microtasks(void)
{
	circular_buffer_t *buffer = &ASYNC_G(microtasks);

	if (circular_buffer_is_empty(buffer)) {
		return;
	}

	/**
	 * The execution of the microtask queue occurs in two stages:
	 * * All microtasks in the queue that were initially scheduled are executed.
	 * * All subsequent microtasks in the queue are executed, but no more than MICROTASK_CYCLE_THRESHOLD_C the buffer size.
	*/

	if (execute_microtasks_stage(buffer, circular_buffer_count(buffer)) == SUCCESS) {
		return;
	}

	const size_t max_count = circular_buffer_capacity(buffer) * MICROTASK_CYCLE_THRESHOLD_C;

	if (execute_microtasks_stage(buffer, max_count) == SUCCESS) {
		return;
	}

	// TODO: make critical error
	async_throw_error(
		"A possible infinite loop was detected during microtask execution. Max count: %u, remaining: %u",
		max_count,
		circular_buffer_count(buffer)
	);
}

static bool handle_callbacks(bool no_wait)
{
	async_throw_error("Event Loop API method handle_callbacks not implemented");
	return false;
}

static bool execute_next_fiber(void)
{
	async_resume_t *resume = async_next_deferred_resume();

	if (resume == NULL) {
		return false;
	}

	if (UNEXPECTED(resume->status == ASYNC_RESUME_IGNORED)) {

		//
		// This state triggers if the fiber has never been started;
		// in this case, it is deallocated differently than usual.
		// Finalizing handlers are called. Memory is freed in the correct order!
		//

		async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

		if (state != NULL) {
			state->resume = NULL;
		}

		// First release the reference to the resume object
		zend_fiber * fiber = resume->fiber;
		OBJ_RELEASE(&resume->std);

		// And only then release the reference to the fiber object
		resume->fiber = NULL;
		zend_hash_index_del(&ASYNC_G(fibers_state), fiber->std.handle);
		OBJ_RELEASE(&fiber->std);
		return true;
	}

	if (UNEXPECTED(resume->status == ASYNC_RESUME_WAITING)) {
		zend_error(E_ERROR, "Attempt to resume a fiber that has not been resolved");
		async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

		if (state != NULL) {
			state->resume = NULL;
		}

		OBJ_RELEASE(&resume->std);
		return false;
	}

	zval retval;
	ZVAL_UNDEF(&retval);

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);
	ZEND_ASSERT(state != NULL && "Fiber state not found but required");

	if (EXPECTED(state != NULL)) {
		state->resume = NULL;
	}

	// After the fiber is resumed, the resume object is no longer needed.
	// So we need to release the reference to the object before resuming the fiber.
	// Copy the resume object status and fiber to local variables.
	ASYNC_RESUME_STATUS status = resume->status;
	resume->status = ASYNC_RESUME_NO_STATUS;
	zend_fiber *fiber = resume->fiber;
	zval result = resume->result;
	ZVAL_UNDEF(&resume->result);

	zend_object * error = resume->error;
	resume->error = NULL;

	OBJ_RELEASE(&resume->std);

	if (EXPECTED(status == ASYNC_RESUME_SUCCESS)) {

		if (UNEXPECTED(fiber->context.status == ZEND_FIBER_STATUS_INIT)) {
			zend_fiber_start(fiber, &retval);
		} else {
			zend_fiber_resume(fiber, &result, &retval);
		}

	} else {

		if (UNEXPECTED(fiber->context.status == ZEND_FIBER_STATUS_INIT)) {
			async_warning("Attempt to resume with error a fiber that has not been started");
			zend_fiber_start(fiber, &retval);
		} else {
			zval zval_error;
			ZVAL_OBJ(&zval_error, error);
			zend_fiber_resume_exception(fiber, &zval_error, &retval);
		}
	}

	// Ignore the exception if it is a cancellation exception
	if (UNEXPECTED(EG(exception) && instanceof_function(EG(exception)->ce, async_ce_cancellation_exception))) {
        zend_clear_exception();
    }

	// Free fiber if it is completed
	if (fiber->context.status == ZEND_FIBER_STATUS_DEAD) {
		OBJ_RELEASE(&fiber->std);
	}

	if (error != NULL) {
		OBJ_RELEASE(error);
	}

	zval_ptr_dtor(&result);
	zval_ptr_dtor(&retval);

	return true;
}

static bool resolve_deadlocks(void)
{
	zval *value;
	zend_ulong index;
	zend_string *key;

	async_warning(
		"No active fibers, deadlock detected. Fibers in waiting: %u", zend_hash_num_elements(&ASYNC_G(fibers_state))
	);

	ZEND_HASH_FOREACH_KEY_VAL(&ASYNC_G(fibers_state), index, key, value)

		const async_fiber_state_t* fiber_state = (async_fiber_state_t*)Z_PTR_P(value);

		ZEND_ASSERT(fiber_state->resume != NULL && "Fiber state has no resume object");

		if (fiber_state->resume != NULL && fiber_state->resume->filename != NULL) {

			//Maybe we need to get the function name
			//zend_string * function_name = NULL;
			//zend_get_function_name_by_fci(&fiber_state->fiber->fci, &fiber_state->fiber->fci_cache, &function_name);

			async_warning(
				"Resume that suspended in file: %s, line: %d will be canceled",
				ZSTR_VAL(fiber_state->resume->filename),
				fiber_state->resume->lineno
			);
		}

		async_cancel_fiber(
			fiber_state->fiber,
			async_new_exception(async_ce_cancellation_exception, "Deadlock detected"),
			true
		);

		if (EG(exception) != NULL) {
			return true;
		}

	ZEND_HASH_FOREACH_END();

	return false;
}


ZEND_API async_callbacks_handler_t async_scheduler_set_callbacks_handler(async_callbacks_handler_t handler)
{
    const async_callbacks_handler_t prev = ASYNC_G(execute_callbacks_handler);
    ASYNC_G(execute_callbacks_handler) = handler ? handler : handle_callbacks;
    return prev;
}

ZEND_API async_next_fiber_handler_t async_scheduler_set_next_fiber_handler(const async_next_fiber_handler_t handler)
{
	const async_next_fiber_handler_t prev = ASYNC_G(execute_next_fiber_handler);
	ASYNC_G(execute_next_fiber_handler) = handler ? handler : execute_next_fiber;
	return prev;
}

ZEND_API async_microtasks_handler_t async_scheduler_set_microtasks_handler(const async_microtasks_handler_t handler)
{
	const async_microtasks_handler_t prev = ASYNC_G(execute_microtasks_handler);
	ASYNC_G(execute_microtasks_handler) = handler ? handler : execute_microtasks;
	return prev;
}

ZEND_API async_exception_handler_t async_scheduler_set_exception_handler(const async_exception_handler_t handler)
{
	const async_exception_handler_t prev = ASYNC_G(exception_handler);
	ASYNC_G(exception_handler) = handler;
	return prev;
}

ZEND_API void async_scheduler_add_microtask(zval *z_microtask)
{
	if (UNEXPECTED(false == zend_is_callable(z_microtask, 0, NULL))) {
		async_throw_error("Invalid microtask type: should be a callable");
		return;
	}

	async_microtask_t * microtask = pecalloc(1, sizeof(async_microtask_t), 0);
	microtask->type = ASYNC_MICROTASK_ZVAL;
	microtask->ref_count = 1;
	microtask->context = async_context_current(false, true);

	zval_copy(&microtask->callable, z_microtask);
	circular_buffer_push(&ASYNC_G(microtasks), &microtask, true);
}

ZEND_API void async_scheduler_add_microtask_ex(async_microtask_t *microtask)
{
	circular_buffer_push(&ASYNC_G(microtasks), &microtask, true);
	microtask->ref_count++;

	if (microtask->context == NULL) {
		microtask->context = async_context_current(false, true);
	}
}

ZEND_API void async_scheduler_add_microtask_handler(async_microtask_handler_t handler, zend_object * zend_object)
{
	async_internal_microtask_t *microtask;

	if (zend_object == NULL) {
		microtask = pecalloc(1, sizeof(async_internal_microtask_t), 0);
	} else {
		microtask = pecalloc(1, sizeof(async_internal_microtask_with_object_t), 0);
		((async_internal_microtask_with_object_t * ) microtask)->object = zend_object;
		((async_internal_microtask_with_object_t * ) microtask)->task.dtor = NULL;
	}

	microtask->task.type = ASYNC_MICROTASK_OBJECT;
	microtask->handler = handler;
	microtask->task.ref_count = 1;
	microtask->task.context = async_context_current(false, true);

	circular_buffer_push(&ASYNC_G(microtasks), &microtask, true);
}

ZEND_API async_microtask_t * async_scheduler_create_microtask(zval * microtask)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	if (UNEXPECTED(zend_fcall_info_init(microtask, 0, &fci, &fcc, NULL, NULL) != SUCCESS)) {
		return NULL;
	}

	async_function_microtask_t *function = pecalloc(1, sizeof(async_function_microtask_t), 0);
	function->task.type = ASYNC_MICROTASK_FCI;
	function->task.ref_count = 1;
	function->fci = fci;
	function->fci_cache = fcc;

	// Keep a reference to closures or callable objects
	Z_TRY_ADDREF(function->fci.function_name);

	return (async_microtask_t *) function;
}

static void microtask_throw_exception_handler(async_microtask_t *microtask)
{
	zval exception;
	ZVAL_OBJ(&exception, ((async_microtask_with_exception_t *) microtask)->exception);

	zend_throw_exception_object(&exception);

	((async_microtask_with_exception_t *) microtask)->exception = NULL;
}

ZEND_API void async_scheduler_transfer_exception(zend_object * exception)
{
	if (EG(active_fiber) == NULL) {
		zend_throw_exception_internal(exception);
		return;
	}

	async_microtask_with_exception_t *microtask = pecalloc(1, sizeof(async_microtask_with_exception_t), 0);
	microtask->task.type = ASYNC_MICROTASK_EXCEPTION;
	microtask->handler = microtask_throw_exception_handler;
	microtask->task.dtor = NULL;
	microtask->task.ref_count = 1;
	microtask->task.context = async_context_current(false, true);

	circular_buffer_push(&ASYNC_G(microtasks), &microtask, true);
}

ZEND_API void async_scheduler_microtask_free(async_microtask_t *microtask)
{
	if (microtask->ref_count <= 0) {
		return;
	}

	microtask->ref_count--;

	if (microtask->ref_count > 0) {
		return;
	}

	if (microtask->type != ASYNC_MICROTASK_ZVAL && microtask->dtor != NULL) {
		microtask->dtor(microtask);
	}

	if (microtask->context != NULL) {
		OBJ_RELEASE(&microtask->context->std);
	}

	switch (microtask->type) {
		case ASYNC_MICROTASK_FCI:
			async_function_microtask_t *function = (async_function_microtask_t *) microtask;
			zval_ptr_dtor(&function->fci.function_name);
			ZVAL_UNDEF(&function->fci.function_name);
			zend_fcall_info_args_clear(&function->fci, 1);
			break;

		case ASYNC_MICROTASK_ZVAL:
			zval_ptr_dtor(&microtask->callable);
			ZVAL_UNDEF(&microtask->callable);

			break;

		case ASYNC_MICROTASK_OBJECT:

			if (((async_internal_microtask_with_object_t * ) microtask)->object != NULL) {
				OBJ_RELEASE(((async_internal_microtask_with_object_t * ) microtask)->object);
				((async_internal_microtask_with_object_t * ) microtask)->object = NULL;
			}

			break;

		case ASYNC_MICROTASK_EXCEPTION:
			async_microtask_with_exception_t *microtask_with_exception = (async_microtask_with_exception_t *) microtask;

			if (microtask_with_exception->exception != NULL) {
				OBJ_RELEASE(microtask_with_exception->exception);
				microtask_with_exception->exception = NULL;
			}

			break;

		case ASYNC_MICROTASK_INTERNAL:
		default:
			break;
	}

	pefree(microtask, 0);
}

zend_always_inline void execute_deferred_fibers(void)
{
	const async_next_fiber_handler_t execute_next_fiber_handler = ASYNC_G(execute_next_fiber_handler) ?
									ASYNC_G(execute_next_fiber_handler) : execute_next_fiber;

	while (false == circular_buffer_is_empty(&ASYNC_G(deferred_resumes))) {
		execute_next_fiber_handler();

		if (UNEXPECTED(EG(exception))) {
			zend_exception_save();
		}
	}
}

static void call_fiber_deferred_callbacks(void)
{
	zval * current;

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), current) {
		const async_fiber_state_t *fiber_state = Z_PTR_P(current);

		if (fiber_state->resume != NULL) {
			fiber_state->resume->status = ASYNC_RESUME_IGNORED;
		}

		zend_fiber_finalize(fiber_state->fiber);

		if (EG(exception)) {
			zend_exception_save();
		}

	} ZEND_HASH_FOREACH_END();
}

static void cancel_deferred_fibers(void)
{
	zend_exception_save();

	// 1. Walk through all fibers and cancel them if they are suspended.
	zval * current;

	zend_object * cancellation_exception = async_new_exception(async_ce_cancellation_exception, "Graceful shutdown");

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), current) {
		async_fiber_state_t *fiber_state = Z_PTR_P(current);

		if (fiber_state->fiber->context.status == ZEND_FIBER_STATUS_INIT) {
			// No need to cancel the fiber if it has not been started.
			fiber_state->resume->status = ASYNC_RESUME_IGNORED;
			zend_fiber_finalize(fiber_state->fiber);
		} else {
			async_cancel_fiber(fiber_state->fiber, cancellation_exception, false);
		}

		if (EG(exception)) {
			zend_exception_save();
		}

	} ZEND_HASH_FOREACH_END();

	OBJ_RELEASE(cancellation_exception);

	zend_exception_restore();
}

static void finally_shutdown(void)
{
	if (ASYNC_G(exit_exception) != NULL && EG(exception) != NULL) {
		zend_exception_set_previous(EG(exception), ASYNC_G(exit_exception));
		GC_DELREF(ASYNC_G(exit_exception));
		ASYNC_G(exit_exception) = EG(exception);
		GC_ADDREF(EG(exception));
		zend_clear_exception();
	}

	cancel_deferred_fibers();
	execute_deferred_fibers();

	const async_microtasks_handler_t execute_microtasks_handler = ASYNC_G(execute_microtasks_handler)
							? ASYNC_G(execute_microtasks_handler) : execute_microtasks;

	execute_microtasks_handler();

	if (UNEXPECTED(EG(exception))) {
		if (ASYNC_G(exit_exception) != NULL) {
			zend_exception_set_previous(EG(exception), ASYNC_G(exit_exception));
			GC_DELREF(ASYNC_G(exit_exception));
			ASYNC_G(exit_exception) = EG(exception);
			GC_ADDREF(EG(exception));
		}
	}
}

static void start_graceful_shutdown(void)
{
	ASYNC_G(graceful_shutdown) = true;
	ASYNC_G(exit_exception) = EG(exception);
	GC_ADDREF(EG(exception));

	zend_clear_exception();
	cancel_deferred_fibers();

	if (UNEXPECTED(EG(exception)) != NULL) {
		zend_exception_set_previous(EG(exception), ASYNC_G(exit_exception));
		GC_DELREF(ASYNC_G(exit_exception));
		ASYNC_G(exit_exception) = EG(exception);
		GC_ADDREF(EG(exception));
		zend_clear_exception();
	}
}

static void async_scheduler_dtor(void)
{
	ASYNC_G(in_scheduler_context) = true;

	if (ASYNC_G(callbacks_fiber) != NULL) {

		if (ASYNC_G(callbacks_fiber)->context.status == ZEND_FIBER_STATUS_SUSPENDED) {
			zval parameter;
			ZVAL_NULL(&parameter);
			zend_fiber_resume(ASYNC_G(callbacks_fiber), &parameter, NULL);
		}

        OBJ_RELEASE(&ASYNC_G(callbacks_fiber)->std);
        ASYNC_G(callbacks_fiber) = NULL;
    }

	if (ASYNC_G(microtask_fiber) != NULL) {

        if (ASYNC_G(microtask_fiber)->context.status == ZEND_FIBER_STATUS_SUSPENDED) {
            zval parameter;
            ZVAL_NULL(&parameter);
            zend_fiber_resume(ASYNC_G(microtask_fiber), &parameter, NULL);
        }

        OBJ_RELEASE(&ASYNC_G(microtask_fiber)->std);
        ASYNC_G(microtask_fiber) = NULL;
    }

	ASYNC_G(in_scheduler_context) = false;

	if (UNEXPECTED(false == circular_buffer_is_empty(&ASYNC_G(microtasks)))) {
		async_warning(
			"%u microtasks were not executed", circular_buffer_count(&ASYNC_G(microtasks))
		);
	}

	if (UNEXPECTED(false == circular_buffer_is_empty(&ASYNC_G(deferred_resumes)))) {
		async_warning(
			"%u deferred resumes were not executed",
			circular_buffer_count(&ASYNC_G(deferred_resumes))
		);
	}

	zval_c_buffer_cleanup(&ASYNC_G(deferred_resumes));
	zval_c_buffer_cleanup(&ASYNC_G(microtasks));
	zend_hash_clean(&ASYNC_G(defer_callbacks));

	zval *current;
	// foreach by fibers_state and release all fibers
	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), current) {
		async_fiber_state_t *fiber_state = Z_PTR_P(current);

		if (fiber_state->fiber != NULL) {
			OBJ_RELEASE(&fiber_state->fiber->std);
		}
	} ZEND_HASH_FOREACH_END();

	zend_hash_clean(&ASYNC_G(fibers_state));

	reactor_shutdown_fn();
	ASYNC_G(graceful_shutdown) = false;
	ASYNC_G(in_scheduler_context) = false;
	ASYNC_G(is_async) = false;

	zend_exception_restore();
}

void async_scheduler_startup(void)
{
	if (callbacks_internal_function.module == NULL) {
        callbacks_internal_function.module = &async_module_entry;
        callbacks_internal_function.function_name = zend_string_init(ZEND_STRL("async_callbacks_internal_function"), 1);
    }

	if (microtasks_internal_function.module == NULL) {
		microtasks_internal_function.module = &async_module_entry;
		microtasks_internal_function.function_name = zend_string_init(ZEND_STRL("async_microtasks_internal_function"), 1);
    }
}

void async_scheduler_shutdown(void)
{
	if (microtasks_internal_function.function_name != NULL) {
        zend_string_release(microtasks_internal_function.function_name);
        microtasks_internal_function.function_name = NULL;
    }

	if (callbacks_internal_function.function_name != NULL) {
        zend_string_release(callbacks_internal_function.function_name);
        callbacks_internal_function.function_name = NULL;
    }
}

#define TRY_HANDLE_EXCEPTION() \
	if (UNEXPECTED(EG(exception) != NULL && handle_exception_handler != NULL)) { \
		handle_exception_handler(); \
	} \
	if (UNEXPECTED(EG(exception) != NULL)) { \
	    if(ASYNC_G(graceful_shutdown)) { \
			finally_shutdown(); \
            break; \
        } \
		start_graceful_shutdown(); \
	}

/**
 * The main loop of the scheduler.
 */
void async_scheduler_launch(void)
{
	if (EG(active_fiber) != NULL) {
		async_throw_error("The scheduler cannot be started from a Fiber");
		return;
	}

	if (false == reactor_is_enabled()) {
		async_throw_error("The scheduler cannot be started without the Reactor");
		return;
	}

	ASYNC_G(is_async) = true;

	reactor_startup_fn();

	/**
	 * Handlers for the scheduler.
	 * This functions pointer will be set to the actual functions.
	 */
	const async_callbacks_handler_t execute_callbacks_handler = ASYNC_G(execute_callbacks_handler);
	const async_microtasks_handler_t execute_microtasks_handler = ASYNC_G(execute_microtasks_handler)
													? ASYNC_G(execute_microtasks_handler) : execute_microtasks;
	const async_next_fiber_handler_t execute_next_fiber_handler = ASYNC_G(execute_next_fiber_handler)
													? ASYNC_G(execute_next_fiber_handler) : execute_next_fiber;
	const async_exception_handler_t handle_exception_handler = ASYNC_G(exception_handler);

	zend_try
	{
		bool has_handles = true;

		do {

			ASYNC_G(in_scheduler_context) = true;

			execute_microtasks_handler();
			TRY_HANDLE_EXCEPTION();

			has_handles = execute_callbacks_handler(circular_buffer_is_not_empty(&ASYNC_G(deferred_resumes)));
			TRY_HANDLE_EXCEPTION();

			execute_microtasks_handler();
			TRY_HANDLE_EXCEPTION();

			ASYNC_G(in_scheduler_context) = false;

			bool was_executed = execute_next_fiber_handler();
			TRY_HANDLE_EXCEPTION();

			if (UNEXPECTED(
				false == has_handles
				&& false == was_executed
				&& zend_hash_num_elements(&ASYNC_G(fibers_state)) > 0
				&& circular_buffer_is_empty(&ASYNC_G(deferred_resumes))
				&& circular_buffer_is_empty(&ASYNC_G(microtasks))
				&& resolve_deadlocks()
				)) {
				break;
			}

		} while (zend_hash_num_elements(&ASYNC_G(fibers_state)) > 0
			|| circular_buffer_is_not_empty(&ASYNC_G(microtasks))
			|| reactor_loop_alive_fn()
		);

	} zend_catch {
		call_fiber_deferred_callbacks();
		async_scheduler_dtor();
		zend_bailout();
	} zend_end_try();

	ZEND_ASSERT(reactor_loop_alive_fn() == false && "The event loop must be stopped");

	zend_object * exit_exception = ASYNC_G(exit_exception);
	ASYNC_G(exit_exception) = NULL;

	async_scheduler_dtor();

	if (EG(exception) != NULL && exit_exception != NULL) {
		zend_exception_set_previous(EG(exception), exit_exception);
		GC_DELREF(exit_exception);
		exit_exception = EG(exception);
		GC_ADDREF(exit_exception);
		zend_clear_exception();
	}

	if (exit_exception != NULL) {
		zend_throw_exception_internal(exit_exception);
	}
}
