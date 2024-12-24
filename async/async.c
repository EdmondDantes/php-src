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
#include "php.h"
#include "async.h"

#include <zend_fibers.h>

#include "event_loop.h"
#include "scheduler.h"
#include "php_layer/functions.h"
#include "php_layer/notifier.h"
#include "php_layer/ev_handles.h"
#include "php_layer/exceptions.h"

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
void async_scheduler_startup(void)
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
	async_globals = pecalloc(1, globals_size, 1);
	async_globals_ctor(async_globals);
#endif
}

/**
 * Activate the scheduler context.
 */
void async_scheduler_shutdown(void)
{
#ifdef ZTS
	if (async_globals_id == 0) {
        return;
    }

	ts_free_id(async_globals_id);
	async_globals_id = 0;
#else
	async_globals_dtor(async_globals);
	pefree(async_globals, 1);
#endif
}

/**
 * Async startup function.
 */
void async_startup(void)
{
	if (async_register_module() == FAILURE) {
		zend_error(E_CORE_WARNING, "Failed to register the 'True Asynchrony' module.");
	}
}

/**
 * Async shutdown function.
 */
void async_shutdown(void)
{
}

/**
 * Fetches the fiber state for the given fiber.
 */
ZEND_API async_fiber_state_t * async_find_fiber_state(const zend_fiber *fiber)
{
	if (IS_ASYNC_OFF) {
		return NULL;
	}

	zval *result = zend_hash_index_find(&ASYNC_G(fibers_state), fiber->std.handle);

	if (UNEXPECTED(result == NULL)) {
		return NULL;
	}

	async_fiber_state_t *resume = (async_fiber_state_t *) Z_PTR_P(result);
	zval_ptr_dtor(result);
	return resume;
}

static async_fiber_state_t * async_add_fiber_state(zend_fiber * fiber, async_resume_t *resume)
{
	async_fiber_state_t * state = pecalloc(1, sizeof(async_fiber_state_t), 1);

	if (state == NULL) {
        return NULL;
    }

	state->fiber = fiber;
	state->resume = resume;

	// Add reference to the fiber object and the resume object.
	GC_ADDREF(&fiber->std);

	if (resume != NULL) {
        GC_ADDREF(&resume->std);
    }

	zval zv;
	ZVAL_PTR(&zv, state);

	zend_hash_index_update(&ASYNC_G(fibers_state), fiber->std.handle, &zv);

	return state;
}

/**
 * Suspend the current fiber.
 * The method puts the current Fiber into a waiting state for descriptors and events described by the Resume object.
 * Calling this method essentially returns control to the Scheduler.
 *
 * @param resume - Resume object. If NULL, a new object will be created.
 */
void async_await(async_resume_t *resume)
{
	if (UNEXPECTED(IS_ASYNC_OFF)) {
		return;
	}

	if (EG(active_fiber) == NULL) {
		zend_error(E_CORE_WARNING, "Cannot await in a non-fiber context");
        return;
    }

	bool is_owned_resume = false;

	if (resume == NULL) {
		is_owned_resume = true;
		resume = async_resume_new();
	}

	if (resume == NULL) {
		async_throw_error("Failed to create a new Resume object");
		goto finally;
	}

	if (resume->status != ASYNC_RESUME_PENDING) {
		async_throw_error("Attempt to use a Resume object that is not in the ASYNC_RESUME_PENDING state.");
		goto finally;
    }

	if (resume->fiber != EG(active_fiber)) {
		async_throw_error("Attempt to use a Resume object that is not associated with the current Fiber.");
	    goto finally;
    }

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

	if (state == NULL) {
		state = async_add_fiber_state(resume->fiber, NULL);

		if (state == NULL) {
			async_throw_error("Failed to create Fiber state");
			goto finally;
        }
	}

	if (state->resume != NULL) {
		async_throw_error("Attempt to stop a Fiber that already has a Resume object.");
		goto finally;
	}

	GC_ADDREF(&resume->std);
	state->resume = resume;

	zval *notifier;

	ZEND_HASH_FOREACH_VAL(&resume->notifiers, notifier)
		if (Z_TYPE_P(notifier) == IS_OBJECT) {

			async_ev_add_handle((async_ev_handle_t *) Z_OBJ_P(notifier));

			if (EG(exception) != NULL) {
				goto finally;
			}
		}
	ZEND_HASH_FOREACH_END();

	resume->status = ASYNC_RESUME_WAITING;

	zend_fiber_suspend(EG(active_fiber), NULL, NULL);

finally:

	if (is_owned_resume) {
        GC_DELREF(&resume->std);
    }
}

void async_build_resume_with(zend_ulong timeout, async_notifier_t * cancellation)
{

}

void async_await_socket()
{

}

/**
 * The method stops Fiber execution for a Zend resource.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_await.
 */
void async_await_resource(
	zend_resource * resource, const zend_ulong actions, const zend_ulong timeout, async_notifier_t * cancellation
)
{
	const async_ev_handle_t *handle = async_ev_handle_from_resource_fn(resource, actions);

	if (handle == NULL) {
		async_throw_error("I can't create an event handle from the resource.");
		return;
	}

	async_resume_t *resume = async_resume_new();

	if(resume == NULL) {
		async_throw_error("Failed to create a new Resume object");
        return;
    }

	// Add timer handle if a timeout is specified.
	if (timeout > 0) {
		async_resume_when(resume, async_ev_timeout_new_fn(timeout), async_resume_when_callback_cancel);
	}

	// Add cancellation handle if it is specified.
	if (cancellation != NULL) {
        async_resume_when(resume, cancellation, async_resume_when_callback_cancel);
    }

	async_await(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}

/**
 * The method stops Fiber execution for a specified signal.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_await.
 */
void async_await_signal(const zend_long sig_number, async_notifier_t * cancellation)
{
	async_resume_t *resume = async_resume_new();

	async_resume_when(resume, async_ev_signal_new_fn(sig_number), async_resume_when_callback_resolve);

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, async_resume_when_callback_cancel);
	}

	async_await(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}

/**
 * The method stops Fiber execution for a specified time.
 * The method creates a Resume descriptor, a timeout handle if needed, and calls async_await.
 * It's like a sleep()/usleep() function.
 */
void async_await_timeout(const zend_ulong timeout, async_notifier_t * cancellation)
{
	if (timeout == 0) {
		async_await(NULL);
		return;
	}

	async_resume_t *resume = async_resume_new();

	async_resume_when(resume, async_ev_timeout_new_fn(timeout), async_resume_when_callback_resolve);

	if (cancellation != NULL) {
		async_resume_when(resume, cancellation, async_resume_when_callback_cancel);
	}

	async_await(resume);

	// Release the reference to the resume object.
	GC_DELREF(&resume->std);
}