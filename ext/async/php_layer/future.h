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
#ifndef FUTURE_H
#define FUTURE_H

#include <zend_exceptions.h>

#include "context.h"
#include "notifier.h"

BEGIN_EXTERN_C()

extern ZEND_API zend_class_entry *async_ce_future_state;
extern ZEND_API zend_class_entry *async_ce_future;

typedef enum {
	ASYNC_FUTURE_MAPPER_SUCCESS,
	ASYNC_FUTURE_MAPPER_CATCH,
	ASYNC_FUTURE_MAPPER_FINALLY
} ASYNC_FUTURE_MAPPER;

typedef struct _async_future_state_s
{
	reactor_notifier_t notifier;
	/* List of linked future states */
	HashTable * linked_future_states;
	/* Context of the future state. Can be NULL */
	async_context_t *context;
	/* Result of the future state */
	zval result;
	/* Error of the future state */
	zend_object * throwable;
	/* Function that processes the result of FutureState */
	zval mapper;
	/* Type of mapper */
	ASYNC_FUTURE_MAPPER mapper_type;
	/** Callback for listen other future states */
	zend_object * callback;
	/* Flag indicates that the future state is used. */
	bool is_used;
	/* Filename of the future object creation point. */
	zend_string *filename;
	/* Line number of the future object creation point. */
	uint32_t lineno;
	/* Filename of the future object completion point. */
	zend_string *completed_filename;
	/* Line number of the future object completion point. */
	uint32_t completed_lineno;
	/* The flag indicates that the future state is in the microtask queue. */
	bool in_microtask_queue;
	bool remove_after_notify;
	bool will_exception_caught;
} async_future_state_t;

typedef struct _async_future_s
{
	zend_object std;
	struct
	{
		char _padding[sizeof(zend_object) - sizeof(zval)];
		/**
		 * PHP object Async\FutureState
		 */
		zend_object * future_state;
	};

} async_future_t;

zend_always_inline static void async_future_state_to_retval(async_future_state_t * future_state, zval * retval)
{
	ZVAL_UNDEF(retval);

	if (EG(exception) != NULL) {
		return;
	}

	if (future_state->throwable != NULL) {
		zend_throw_exception_internal(future_state->throwable);
	} else {
        ZVAL_COPY(retval, &future_state->result);
    }
}

void async_register_future_ce(void);

zend_object * async_future_state_new(void);
zend_object * async_future_new(zend_object * future_state);

void async_future_state_resolve(async_future_state_t *future_state, zval * retval);
void async_future_state_reject(async_future_state_t *future_state, zend_object * error);

ZEND_API void async_await_future(async_future_state_t *future_state, zval * retval);

ZEND_API void async_await_future_list(
	zval *iterable,
	int count,
	bool ignore_errors,
	reactor_notifier_t *cancellation,
	zend_ulong timeout,
	HashTable * results,
	HashTable * errors
);

END_EXTERN_C()

#endif //FUTURE_H
