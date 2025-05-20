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

#include <php.h>

#include "coroutine.h"
#include "internal/circular_buffer.h"

#ifdef PHP_WIN32

#else

#endif

extern zend_module_entry async_module_entry;
#define phpext_async_ptr &async_module_entry

extern zend_class_entry * async_ce_awaitable;
extern zend_class_entry * async_ce_timeout;

#define PHP_ASYNC_NAME "TrueAsync"
#define PHP_ASYNC_VERSION "0.5.0"
#define PHP_ASYNC_NAME_VERSION "TrueAsync v0.5.0"

typedef struct
{
	zend_async_event_t event;
	/* Reactor original handle */
	zend_async_event_dispose_t reactor_dispose;
	zend_object std;
} async_timeout_t;

#define ASYNC_TIMEOUT_FROM_OBJ(obj) ((async_timeout_t *)((char *)(obj) - XtOffsetOf(async_timeout_t, std)))
#define Z_ASYNC_TIMEOUT_P(zv)  ASYNC_TIMEOUT_FROM_OBJ(Z_OBJ_P(zv))

ZEND_BEGIN_MODULE_GLOBALS(async)
	/* Number of active coroutines */
	unsigned int active_coroutine_count;
	/* Number of active event handles */
	unsigned int active_event_count;
	// Microtask queue
	circular_buffer_t microtasks;
	/* Queue of coroutine_queue */
	circular_buffer_t coroutine_queue;
	/* List of coroutines  */
	HashTable coroutines;
	/* The transfer structure is used to return to the main execution context. */
	zend_fiber_transfer *main_transfer;
	/* The main flow stack */
	zend_vm_stack main_vm_stack;
	/* Scheduler coroutine */
	zend_coroutine_t *scheduler;

	/* Link to the reactor structure */
	void * reactor;

	#ifdef PHP_WIN32
	#endif
ZEND_END_MODULE_GLOBALS(async)

ZEND_EXTERN_MODULE_GLOBALS(async)

#define ASYNC_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(async, v)

# if defined(ZTS) && defined(COMPILE_DL_ASYNC)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#define DECREASE_EVENT_HANDLE_COUNT  if (ASYNC_G(active_event_count) > 0) { \
		ASYNC_G(active_event_count)--; \
	} else { \
		ZEND_ASSERT("The event handle count is already zero."); \
	}

#define DECREASE_COROUTINE_COUNT  if (ASYNC_G(active_coroutine_count) > 0) { \
		ASYNC_G(active_coroutine_count)--; \
	} else { \
		ZEND_ASSERT("The coroutine count is already zero."); \
	}

#endif //ASYNC_H