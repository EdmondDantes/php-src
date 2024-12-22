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

#include "internal/scheduler.h"
#include "php_layer/functions.h"
#include "php_layer/notifier.h"

#ifdef ZTS
TSRMLS_MAIN_CACHE_DEFINE()
#else
ZEND_API async_globals_t* async_globals;
#endif

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

#ifdef PHP_ASYNC_LIBUV
	uv_loop_init(&async_globals->uv_loop);
#endif
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

#ifdef PHP_ASYNC_LIBUV
	uv_loop_close(&async_globals->uv_loop);
#endif

	circular_buffer_dtor(&async_globals->microtasks);
	circular_buffer_dtor(&async_globals->pending_fibers);
	zend_hash_destroy(&async_globals->fibers_state);
}

/**
 * Activate the scheduler context.
 */
void async_scheduler_startup(void)
{
#ifdef ZTS

	if (async_globals_id != 0) {
		return;
	}

	ts_allocate_fast_id(
		&async_globals_id,
		&async_globals_offset,
		sizeof(async_globals_t),
		(ts_allocate_ctor) async_globals_ctor,
		(ts_allocate_dtor) async_globals_dtor
	);

	async_globals_t *async_globals = ts_resource(async_globals_id);
	async_globals_ctor(async_globals);
#else
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

	if (resume->status != ASYNC_RESUME_PENDING) {
        zend_throw_exception(NULL, "Attempt to use a Resume object that is not in the ASYNC_RESUME_PENDING state.", 0);
        return;
    }

	if (resume->fiber != EG(active_fiber)) {
        zend_throw_exception(NULL, "Attempt to use a Resume object that is not associated with the current Fiber.", 0);
        return;
    }

	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

	if (state == NULL) {
		state = async_add_fiber_state(resume->fiber, NULL);

		if (state == NULL) {
            zend_throw_exception(NULL, "Failed to create Fiber state", 0);
            return;
        }
	}

	if (state->resume != NULL) {
		zend_throw_exception(NULL, "Attempt to stop a Fiber that already has a Resume object.", 0);
		return;
	}

	GC_ADDREF(&resume->std);
	state->resume = resume;

	zval *notifier;

	ZEND_HASH_FOREACH_VAL(&resume->notifiers, notifier)
		if (Z_TYPE_P(notifier) == IS_OBJECT) {
	        if (async_scheduler_add_handle(Z_OBJ_P(notifier)) == FAILURE) {
                zend_throw_exception(NULL, "Failed to add notifier to the scheduler", 0);
                return;
            }
		}
	ZEND_HASH_FOREACH_END();

	resume->status = ASYNC_RESUME_WAITING;

	zend_fiber_suspend(EG(active_fiber), NULL, NULL);
}

void async_build_resume_with(zend_ulong timeout, async_notifier_t * cancellation)
{

}

void async_await_socket()
{

}

void async_await_resource(
	zend_resource * resource, zend_ulong actions, zend_ulong timeout, async_notifier_t * cancellation
)
{

}

void async_await_readable(zend_resource * resource)
{

}