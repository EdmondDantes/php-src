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
#include "scheduler.h"

#include <Zend/zend_fibers.h>

#include "internal/zval_circular_buffer.h"
#include "async.h"
#include "event_loop.h"
#include "php_layer/exceptions.h"

//
// The coefficient of the maximum number of microtasks that can be executed
// without suspicion of an infinite loop (a task that creates microtasks).
//
#define MICROTASK_CYCLE_THRESHOLD_C 4

#ifdef ZTS
TSRMLS_MAIN_CACHE_DEFINE()
#else
ZEND_API async_globals_t* async_globals;
#endif

static async_ex_globals_fn async_ex_globals_handler = NULL;

ZEND_API async_ex_globals_fn async_set_ex_globals_handler(const async_ex_globals_fn handler)
{
	const async_ex_globals_fn old = async_ex_globals_handler;
	async_ex_globals_handler = handler;
	return old;
}

/**
 * Async globals destructor.
 */
static void async_globals_dtor(async_globals_t *async_globals)
{
	if (!async_globals->is_async) {
        return;
    }

	async_globals->is_async = false;
	async_globals->is_scheduler_running = false;

	if (async_ex_globals_handler != NULL) {
		async_ex_globals_handler(async_globals, sizeof(async_globals_t), true);
	}

	circular_buffer_dtor(&async_globals->microtasks);
	circular_buffer_dtor(&async_globals->pending_fibers);
	zend_hash_destroy(&async_globals->fibers_state);
}

/**
 * Async globals constructor.
 */
static void async_globals_ctor(async_globals_t *async_globals)
{
	if (async_globals->is_async) {
		return;
	}

	async_globals->is_async = true;
	async_globals->is_scheduler_running = false;

	circular_buffer_ctor(&async_globals->microtasks, 32, sizeof(zval), &zend_std_persistent_allocator);
	circular_buffer_ctor(&async_globals->pending_fibers, 128, sizeof(async_resume_t *), &zend_std_persistent_allocator);
	zend_hash_init(&async_globals->fibers_state, 128, NULL, NULL, 1);

	if (EG(exception) != NULL) {
		async_globals_dtor(async_globals);
		return;
	}

	if (async_ex_globals_handler != NULL) {
		async_ex_globals_handler(async_globals, sizeof(async_globals_t), false);
	}

	if (EG(exception) != NULL) {
		async_globals_dtor(async_globals);
	}
}

/**
 * Activate the scheduler context.
 */
static void async_scheduler_startup(void)
{
	size_t globals_size = sizeof(async_globals_t);

	if (async_ex_globals_handler != NULL) {
		globals_size += async_ex_globals_handler(NULL, globals_size, false);
	}

#ifdef ZTS

	if (async_globals_id != 0) {
		return;
	}

	ts_allocate_fast_id(
		&async_globals_id,
		&async_globals_offset,
		globals_size,
		(ts_allocate_ctor) async_globals_ctor,
		(ts_allocate_dtor) async_globals_dtor
	);

	async_globals_t *async_globals = ts_resource(async_globals_id);
	async_globals_ctor(async_globals);
#else
	if (async_globals != NULL) {
        return;
    }

	async_globals = pecalloc(1, globals_size, 1);
	async_globals_ctor(async_globals);
#endif
}

/**
 * Activate the scheduler context.
 */
static void async_scheduler_shutdown(void)
{
#ifdef ZTS
	if (async_globals_id == 0) {
        return;
    }

	ts_free_id(async_globals_id);
	async_globals_id = 0;
#else
	if (async_globals == NULL) {
        return;
    }

	async_globals_dtor(async_globals);
	pefree(async_globals, 1);
#endif
}

static void invoke_microtask(zval *task)
{
	if (Z_TYPE(task) == IS_PTR) {
		// Call the function directly
		((void (*)(void)) Z_PTR(*task))();
	} else if (zend_is_callable(task, 0, NULL)) {
		zval retval;
		ZVAL_UNDEF(&retval);
		call_user_function(CG(function_table), NULL, task, &retval, 0, NULL);
		zval_ptr_dtor(&retval);
	}
}

static zend_result execute_microtasks_stage(circular_buffer_t *buffer, const size_t max_count)
{
	if (circular_buffer_is_empty(buffer)) {
		return SUCCESS;
	}

	for (size_t i = 0; i < max_count; i++) {
		zval task;

		zval_c_buffer_pop(buffer, &task);
		invoke_microtask(&task);
		zval_ptr_dtor(&task);

		if (circular_buffer_is_empty(buffer)) {
			return SUCCESS;
		}
	}

	return FAILURE;
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

static void handle_callbacks(void)
{
	async_throw_error("Event Loop API method handle_callbacks not implemented");
}

static void resume_next_fiber(void)
{
	if (circular_buffer_is_empty(&ASYNC_G(pending_fibers))) {
        return;
    }

	async_resume_t *resume;
	circular_buffer_pop(&ASYNC_G(pending_fibers), &resume);

	if (resume->status == ASYNC_RESUME_PENDING) {
		zend_error(E_ERROR, "Attempt to resume a fiber that has not been resolved");
		GC_DELREF(&resume->std);
		return;
	}

	zval retval;
	ZVAL_UNDEF(&retval);

	if (resume->status == ASYNC_RESUME_SUCCESS) {
		zend_fiber_resume(resume->fiber, resume->value, &retval);
	} else {
		zend_fiber_resume_exception(resume->fiber, resume->error, &retval);
	}

	GC_DELREF(&resume->std);
	zval_ptr_dtor(&retval);
}

static zend_bool check_deadlocks(void)
{
	zval *value;
	async_fiber_state_t *fiber_state;

	ZEND_HASH_FOREACH_VAL(&ASYNC_G(fibers_state), value)
		fiber_state = (async_fiber_state_t *) Z_PTR_P(value);
	ZEND_HASH_FOREACH_END();

	return false;
}

/**
 * Handlers for the scheduler.
 * This functions pointer will be set to the actual functions.
 */
static  async_callbacks_handler_t execute_callbacks_fn = NULL;
static  async_microtasks_handler_t execute_microtasks_fn = execute_microtasks;
static  async_next_fiber_handler_t resume_next_fiber_fn = resume_next_fiber;
static  async_fiber_exception_handler_t fiber_exception_fn = NULL;

ZEND_API async_callbacks_handler_t async_scheduler_set_callbacks_handler(const async_callbacks_handler_t handler)
{
    const async_callbacks_handler_t prev = execute_callbacks_fn;
    execute_callbacks_fn = handler ? handler : handle_callbacks;
    return prev;
}

ZEND_API async_next_fiber_handler_t async_scheduler_set_next_fiber_handler(const async_next_fiber_handler_t handler)
{
	const async_next_fiber_handler_t prev = resume_next_fiber_fn;
	resume_next_fiber_fn = handler ? handler : resume_next_fiber;
	return prev;
}

ZEND_API async_microtasks_handler_t async_scheduler_set_microtasks_handler(const async_microtasks_handler_t handler)
{
	const async_microtasks_handler_t prev = execute_microtasks_fn;
	execute_microtasks_fn = handler ? handler : execute_microtasks;
	return prev;
}

ZEND_API async_fiber_exception_handler_t async_scheduler_set_exception_handler(const async_fiber_exception_handler_t handler)
{
    const async_fiber_exception_handler_t prev = fiber_exception_fn;
    fiber_exception_fn = handler;
    return prev;
}

/**
 * The main loop of the scheduler.
 */
void async_scheduler_run(void)
{
	if (EG(active_fiber) != NULL) {
        async_throw_error("The scheduler cannot be started from a Fiber");
        return;
    }

	async_scheduler_startup();

	zend_try
	{
		zend_bool has_handles = true;

		do {

			ASYNC_G(is_scheduler_running) = true;

			execute_microtasks_fn();

			if (EG(exception) != NULL) {
				break;
			}

			has_handles = execute_callbacks_fn();

			if (EG(exception) != NULL) {
				break;
			}

			execute_microtasks_fn();

			if (EG(exception) != NULL) {
				break;
			}

			ASYNC_G(is_scheduler_running) = false;

			resume_next_fiber_fn();

			if (EG(exception) != NULL && fiber_exception_fn != NULL) {
                fiber_exception_fn();
            }

			if (EG(exception) != NULL) {
                break;
            }

			if (false == has_handles && circular_buffer_is_empty(&ASYNC_G(pending_fibers)) && check_deadlocks()) {
				break;
            }

		} while (zend_hash_num_elements(&ASYNC_G(fibers_state)) > 0);

	} zend_catch {
		ASYNC_G(is_scheduler_running) = false;
		async_scheduler_shutdown();
		zend_bailout();
	} zend_end_try();
}

