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

#include "php_layer/functions.h"

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

void async_await(async_resume_t *resume)
{
	async_fiber_state_t *state = async_find_fiber_state(resume->fiber);

	if (state == NULL) {
		zend_throw_exception(NULL, "Attempt to manipulate a Fiber that was created outside of the Async API.", 0);
		return;
	}

	if (state->resume != NULL) {
		zend_throw_exception(NULL, "Attempt to stop a Fiber that already has a Resume object.", 0);
		return;
	}

	GC_ADDREF(&resume->std);
	state->resume = resume;


}