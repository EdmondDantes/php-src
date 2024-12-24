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
#ifndef PHP_ASYNC_H
#define PHP_ASYNC_H

#include "php.h"
#include "internal/circular_buffer.h"
#include "php_layer/resume.h"

#ifdef PHP_ASYNC_LIBUV
#include <uv.h>
#endif

#define ASYNC_READABLE 1
#define ASYNC_WRITABLE 2
#define ASYNC_DISCONNECT 4
#define ASYNC_PRIORITIZED 8

/**
 * Global asynchronous context.
 */
typedef struct _async_globals_s async_globals_t;
/**
 * Fiber state structure.
 * The structure describes the relationship between Fiber and the resume state.
 * The resume state can be NULL, in which case the Fiber is considered active.
*/
typedef struct _async_fiber_state_s async_fiber_state_t;

struct _async_globals_s {
	/* Equal TRUE if the asynchronous context is enabled */
	bool is_async;
	/* Equal TRUE if the scheduler is running */
	bool is_scheduler_running;
	// Microtask and fiber queues
	circular_buffer_t microtasks;
	/* Queue of resume objects: async_resume_t */
	circular_buffer_t pending_fibers;
	/* List of async_fiber_state_t  */
	HashTable fibers_state;
#ifdef PHP_ASYNC_TRACK_HANDLES
	/* List of linked handles to fibers */
	HashTable linked_handles;
#endif
	/* Extension of async_fiber_state_t */
	char extend[];
};

struct _async_fiber_state_s {
	zend_fiber *fiber;
	/* Fiber resume object. Can be NULL */
	async_resume_t *resume;
};

/* Async global */
#ifdef ZTS
ZEND_API int async_globals_id = 0;
ZEND_API size_t async_globals_offset;
# define ASYNC_G(v) ZEND_TSRMG_FAST(async_globals_id, async_globals_t *, v)
#else
# define ASYNC_G(v) (async_globals->v)
ZEND_API async_globals_t* async_globals;
#endif

#define IS_ASYNC_ON (ASYNC_G(is_async) == true)
#define IS_ASYNC_OFF (ASYNC_G(is_async) == false)

/**
 * Async globals Ex-constructor.
 * The method is called in three cases:
 * 1. When it is necessary to return the size of the extended memory.
 * 2. When it is necessary to create and initialize the global Async structure.
 * 3. When the destructor needs to be called.
 */
typedef size_t (* async_ex_globals_fn)(async_globals_t *async_globals, size_t current_size, zend_bool is_destroy);

void async_startup(void);
void async_shutdown(void);
ZEND_API async_fiber_state_t * async_find_fiber_state(const zend_fiber *fiber);
ZEND_API async_ex_globals_fn async_set_ex_globals_handler(async_ex_globals_fn handler);

#endif //ASYNC_H
