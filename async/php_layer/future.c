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
#include "future.h"

#include <async/php_async.h>
#include <async/php_scheduler.h>

#include "closure.h"
#include "exceptions.h"
#include "future_arginfo.h"
#include "zend_common.h"

#define FUTURE_STATE_METHOD(name) PHP_METHOD(Async_FutureState, name)
#define FUTURE_METHOD(name) PHP_METHOD(Async_Future, name)
#define THIS_FUTURE_STATE ((async_future_state_t *) Z_OBJ_P(ZEND_THIS))
#define THIS_FUTURE ((async_future_t *) Z_OBJ_P(ZEND_THIS))

static reactor_notifier_handlers_t async_future_state_handlers;
static zend_object_handlers async_future_handlers;

typedef struct {
    async_internal_microtask_t task;
	async_future_state_t *future_state;
	bool started;
	HashPosition position;
} future_state_callbacks_task;

typedef struct
{
	int total;
	int waiting_count;
	int resolved_count;
	bool ignore_errors;
} future_await_conditions_t;

typedef struct
{
	async_resume_notifier_t * resume_notifier;
	future_await_conditions_t * conditions;
} future_resume_callback_t;

typedef struct {
	async_internal_microtask_t task;
	async_future_state_t *future_state;
	async_resume_t *resume;
	zend_object_iterator *zend_iterator;
	HashTable *futures;
	future_await_conditions_t * conditions;
	bool started;
	unsigned int concurrency;
	unsigned int active_fibers;
} concurrent_iterator_task;

static void future_resume_callback(
	async_resume_t *resume,
	reactor_notifier_t *notifier,
	zval* event,
	zval* error,
	async_resume_notifier_t *resume_notifier
);

static void future_state_callbacks_task_handler(async_microtask_t *microtask)
{
	future_state_callbacks_task *task = (future_state_callbacks_task *) microtask;
	HashTable * callbacks = Z_ARR(task->future_state->notifier.callbacks);
	reactor_notifier_t * notifier = &task->future_state->notifier;

	if (task->started == false) {
		task->started = true;
		zend_hash_internal_pointer_reset_ex(callbacks, &task->position);
	}

	task->future_state->in_microtask_queue = false;

	zval result, error;
	ZVAL_NULL(&result);
	ZVAL_NULL(&error);

	zval resolved_callback;
	ZVAL_UNDEF(&resolved_callback);

	if (task->future_state->throwable != NULL) {
		ZVAL_OBJ(&error, task->future_state->throwable);
	} else {
		ZVAL_COPY_VALUE(&result, &task->future_state->result);
	}

	// Add a microtask to the queue for execution if the Fiber unexpectedly stops due to an asynchronous operation.
	// In this case, we will continue execution in a different Fiber.
	if (zend_hash_num_elements(callbacks) > 0) {
		async_scheduler_add_microtask_ex(microtask);
	} else {
		microtask->is_cancelled = true;
	}

	while (false == microtask->is_cancelled) {		
		zval *current = zend_hash_get_current_data_ex(callbacks, &task->position);

		if (current == NULL) {
			microtask->is_cancelled = true;
			break;
		}

		zend_string * key;
		zend_ulong index;

		int key_type = zend_hash_get_current_key_ex(callbacks, &key, &index, &task->position);

		if (key_type == HASH_KEY_NON_EXISTENT) {
			break;
		}

		zend_hash_move_forward_ex(callbacks, &task->position);

		if (EXPECTED(Z_TYPE_P(current) == IS_OBJECT)) {
			zend_resolve_weak_reference(current, &resolved_callback);

			// Remove from the callbacks array.
			if (key_type == HASH_KEY_IS_STRING) {
				zend_hash_del(callbacks, key);
			} else if (key_type == HASH_KEY_IS_LONG) {
				zend_hash_index_del(callbacks, index);
			}

			// Resume object and Callback object use different handlers.
			if (Z_TYPE(resolved_callback) == IS_OBJECT && Z_OBJ_P(&resolved_callback)->ce == async_ce_resume) {
				async_resume_notify((async_resume_t *) Z_OBJ(resolved_callback), notifier, &result, &error);
			} else if (Z_TYPE(resolved_callback) == IS_OBJECT) {
				async_closure_notify(Z_OBJ(resolved_callback), &notifier->std, &result, &error);
			}

			zval_ptr_dtor(&resolved_callback);

			if (EG(exception) != NULL) {
				zend_exception_save();
			}
		}
	}
}

static void future_state_callbacks_task_dtor(async_microtask_t *microtask)
{
	future_state_callbacks_task *task = (future_state_callbacks_task *) microtask;

	if (task->future_state != NULL) {
		OBJ_RELEASE(&task->future_state->notifier.std);
		task->future_state = NULL;
	}
}

static void concurrent_iterator_task_handler(async_microtask_t *microtask)
{
	concurrent_iterator_task *iterator_task = (concurrent_iterator_task *) microtask;

	if (iterator_task->started == false)
	{
		if (iterator_task->zend_iterator->funcs->rewind) {
			iterator_task->zend_iterator->funcs->rewind(iterator_task->zend_iterator);

			if (EG(exception) != NULL) {
				microtask->is_cancelled = true;
				async_future_state_reject(iterator_task->future_state, EG(exception));
				zend_clear_exception();
				return;
			}
		}
	}

	async_scheduler_add_microtask_ex(microtask);

	zval *current;

	do
	{
		if (EXPECTED(SUCCESS == iterator_task->zend_iterator->funcs->valid(iterator_task->zend_iterator))) {
			current = iterator_task->zend_iterator->funcs->get_current_data(iterator_task->zend_iterator);
		} else {
			current = NULL;
		}

		if (UNEXPECTED(EG(exception) != NULL)) {
			microtask->is_cancelled = true;
			async_future_state_reject(iterator_task->future_state, EG(exception));
			zend_clear_exception();
			return;
		}

		if (EXPECTED(current != NULL)) {

			zval key;
			iterator_task->zend_iterator->funcs->get_current_key(iterator_task->zend_iterator, &key);
			iterator_task->zend_iterator->funcs->move_forward(iterator_task->zend_iterator);

			if (Z_TYPE_P(current) != IS_OBJECT || instanceof_function(Z_OBJCE_P(current), async_ce_future) == 0) {
                async_throw_error("The iterator must return only Async\\Future objects");
                microtask->is_cancelled = true;
                async_future_state_reject(iterator_task->future_state, EG(exception));
                zend_clear_exception();
                return;
            }

			zval * res = NULL;

			if (Z_TYPE(key) == IS_LONG) {
				res = zend_hash_index_update(iterator_task->futures, Z_LVAL(key), current);
			} else if(Z_TYPE(key) == IS_STRING) {
                res = zend_hash_update(iterator_task->futures, Z_STR(key), current);
            } else {
	            res = zend_hash_next_index_insert(iterator_task->futures, current);
            }

			if (EXPECTED(res != NULL)) {
				Z_TRY_ADDREF_P(current);
			} else {
			    async_throw_error("Failed to add Future to the array");
                microtask->is_cancelled = true;
                async_future_state_reject(iterator_task->future_state, EG(exception));
                zend_clear_exception();
                return;
			}

			async_future_state_t * state = (async_future_state_t *) ((async_future_t *) Z_OBJ_P(current))->future_state;

			future_resume_callback_t * callback = emalloc(sizeof(future_resume_callback_t));
			ZVAL_PTR(&callback->resume_notifier->callback, future_resume_callback);
			callback->conditions = iterator_task->conditions;

			async_resume_when_ex(iterator_task->resume, (reactor_notifier_t *) state, true, (async_resume_notifier_t *) callback);
		}

	} while (current != NULL && microtask->is_cancelled == false);

	if (microtask->is_cancelled == false) {
		microtask->is_cancelled = true;
		async_future_state_resolve(iterator_task->future_state, NULL);
	}
}

static void concurrent_iterator_task_dtor(async_microtask_t *microtask)
{
	concurrent_iterator_task *iterator_task = (concurrent_iterator_task *) microtask;

	if (iterator_task->zend_iterator != NULL) {
		zend_iterator_dtor(iterator_task->zend_iterator);
		iterator_task->zend_iterator = NULL;
	}

	if (iterator_task->future_state != NULL) {
		OBJ_RELEASE(&iterator_task->future_state->notifier.std);
		iterator_task->future_state = NULL;
	}

	if (iterator_task->resume != NULL) {
        OBJ_RELEASE(&iterator_task->resume->std);
		iterator_task->resume = NULL;
    }

	if (iterator_task->futures != NULL) {
		zend_hash_release(iterator_task->futures);
		iterator_task->futures = NULL;
	}

	if (iterator_task->conditions != NULL) {
		efree(iterator_task->conditions);
		iterator_task->conditions = NULL;
	}
}

zend_always_inline void add_future_state_callbacks_microtask(async_future_state_t *future_state)
{
	if (UNEXPECTED(future_state->in_microtask_queue
		|| zend_hash_num_elements(Z_ARR(future_state->notifier.callbacks)) == 0)) {

		if (future_state->remove_after_notify) {
			OBJ_RELEASE(&future_state->notifier.std);
		}

		return;
	}

	future_state_callbacks_task *microtask = pecalloc(1, sizeof(future_state_callbacks_task), 0);
	microtask->task.task.type = ASYNC_MICROTASK_INTERNAL;
	microtask->task.handler = future_state_callbacks_task_handler;
	microtask->task.task.dtor = future_state_callbacks_task_dtor;
	microtask->future_state = future_state;

	if (false == future_state->remove_after_notify) {
		GC_ADDREF(&future_state->notifier.std);
	}

	microtask->position = -1;
	microtask->started = false;
	microtask->task.task.context = future_state->context;

	if (ASYNC_G(in_scheduler_context)) {
		future_state_callbacks_task_handler((async_microtask_t *) microtask);

		if (UNEXPECTED(microtask->task.task.ref_count == 0)) {
			microtask->task.task.ref_count = 1;
			async_scheduler_microtask_free((async_microtask_t *) microtask);
		}
	} else {
		async_scheduler_add_microtask_ex((async_microtask_t *) microtask);
	}
}

zend_always_inline bool throw_if_future_state_completed(async_future_state_t *future_state)
{
	if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE) {
		async_throw_error(
			"The Future has already been completed at %s:%d",
			future_state->filename ? ZSTR_VAL(future_state->filename) : "<unknown>",
			future_state->lineno
		);

		return true;
	}

	return false;
}

zend_always_inline void start_concurrent_iterator(
	async_future_state_t *future_state,
	zend_object_iterator *zend_iterator,
	async_resume_t *resume,
	HashTable *futures,
	future_await_conditions_t * conditions
)
{
	concurrent_iterator_task *microtask = pecalloc(1, sizeof(concurrent_iterator_task), 0);

	microtask->task.task.type = false;
	microtask->task.handler = concurrent_iterator_task_handler;
	microtask->task.task.dtor = concurrent_iterator_task_dtor;
	microtask->future_state = future_state;
	microtask->zend_iterator = zend_iterator;
	microtask->resume = resume;
	GC_ADDREF(&resume->std);
	microtask->conditions = conditions;
	microtask->futures = futures;
	GC_ADDREF(futures);
	microtask->started = false;

	async_scheduler_add_microtask_ex((async_microtask_t *) microtask);
}

static void notifier_handler(reactor_notifier_t* notifier, zval* event, zval* error)
{
	add_future_state_callbacks_microtask((async_future_state_t *) notifier);
}

static void future_state_callback(zend_object * callback, zend_object *notifier, zval* z_event, zval* error)
{
	async_future_state_t * future_state = (async_future_state_t *) ((async_closure_t *) callback)->owner;
	async_future_state_t * from_future_state = (async_future_state_t *) notifier;

	if (from_future_state->linked_future_states != NULL
		&& zend_hash_index_exists(from_future_state->linked_future_states, future_state->notifier.std.handle)) {

		if (GC_REFCOUNT(&future_state->notifier.std) == 1) {
			future_state->remove_after_notify = true;
			GC_ADDREF(&future_state->notifier.std);
		}

		zend_hash_index_del(from_future_state->linked_future_states, future_state->notifier.std.handle);
    }

	if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE) {
		async_warning(
			"The Future state has already been completed at %s:%d (called from other Future state created at %s:%d)",
			future_state->completed_filename != NULL ? ZSTR_VAL(future_state->completed_filename) : "<unknown>",
			future_state->completed_lineno,
			from_future_state->filename != NULL ? ZSTR_VAL(from_future_state->filename) : "<unknown>",
			from_future_state->lineno
		);

		if (future_state->remove_after_notify) {
			OBJ_RELEASE(&future_state->notifier.std);
		}

		return;
	}

	ZVAL_TRUE(&future_state->notifier.is_closed);

	if (Z_TYPE(future_state->mapper) != IS_UNDEF && Z_TYPE(future_state->mapper) != IS_NULL) {

		switch (future_state->mapper_type) {
			case ASYNC_FUTURE_MAPPER_SUCCESS:
				if (error == NULL || Z_TYPE_P(error) != IS_OBJECT) {

					zval retval;
					zval args[1];
					ZVAL_COPY(&args[0], z_event);
					ZVAL_UNDEF(&retval);

					call_user_function(EG(function_table), NULL, &future_state->mapper, &retval, 1, args);

					if (EG(exception) == NULL && Z_TYPE(retval) != IS_UNDEF) {
						zval_copy(&future_state->result, &retval);
					} else if (EG(exception) != NULL) {
						future_state->throwable = EG(exception);
						GC_ADDREF(future_state->throwable);
						zend_clear_exception();
					} else {
						zval_copy(&future_state->result, z_event);
					}

					zval_ptr_dtor(&retval);
				} else {
					future_state->throwable = Z_OBJ_P(error);
					GC_ADDREF(future_state->throwable);
				}

				break;

			case ASYNC_FUTURE_MAPPER_CATCH: {

				if (error != NULL && Z_TYPE_P(error) == IS_OBJECT) {
					zval retval;
					zval args[1];
					ZVAL_COPY_VALUE(&args[0], error);
					call_user_function(EG(function_table), NULL, &future_state->mapper, &retval, 1, args);

					if (EG(exception) == NULL) {
						zval_copy(&future_state->result, &retval);
					} else {
						future_state->throwable = EG(exception);
						GC_ADDREF(future_state->throwable);
						zend_clear_exception();
					}

					zval_ptr_dtor(&retval);
				} else {
					zval_copy(&future_state->result, z_event);
					future_state->throwable = NULL;
				}

				break;
			}

			case ASYNC_FUTURE_MAPPER_FINALLY: {
				zval retval;
				call_user_function(EG(function_table), NULL, &future_state->mapper, &retval, 0, NULL);

				if (EG(exception) != NULL) {
					future_state->throwable = EG(exception);
					GC_ADDREF(future_state->throwable);
					zend_clear_exception();
				} else if (error != NULL && Z_TYPE_P(error) == IS_OBJECT) {
					future_state->throwable = Z_OBJ_P(error);
					GC_ADDREF(future_state->throwable);
				} else if (Z_TYPE_P(z_event) != IS_UNDEF) {
					zval_copy(&future_state->result, z_event);
					future_state->throwable = NULL;
				}

				break;
			}
		}

	}

	if (Z_TYPE(future_state->result) == IS_UNDEF && future_state->throwable == NULL) {
		ZVAL_NULL(&future_state->result);
	}

	add_future_state_callbacks_microtask(future_state);
}

zend_always_inline void future_state_subscribe_to(async_future_state_t * from_future_state, async_future_state_t * to_future_state)
{
	if (from_future_state->callback == NULL) {
		from_future_state->callback = (zend_object *) async_closure_new_with_owner(
			future_state_callback, (zend_object *)from_future_state
		);
	}

	//
	// Object Ownership and Reference Count Scheme.
	//
	// Future2 - from_future_state
	// Future1 - to_future_state
	//
	// Future1 (1 ref)
	//   └─ owns (1 ref) ─► FutureState1 (1 ref total: 1 from Future1)
	// 						 │ emits events (1 ref)
	// 						 ▼
	// 					    FutureState2 (2 refs total: 1 from FutureState1, 1 from Future2)
	// 						   └─ holds (1 ref) ─► FutureState1
	//
	// Future2 (1 ref, returned by map(), catch() or finally())
	//   └─ owns (1 ref) ─► FutureState2
	//
	// In this scheme, FutureState1 owns FutureState2.
	// As soon as FutureState1 is deleted, it also removes the reference to FutureState2
	// and marks it as "is_closed", since no one will be able to trigger an event on this Future anymore.
	//

	zval callback;
	ZVAL_OBJ(&callback, from_future_state->callback);

	to_future_state->is_used = true;

	//
	// Subscribe to to_future_state without increasing the reference count on it.
	//
	async_notifier_add_callback_without_backref(&to_future_state->notifier.std, &callback);

	if (UNEXPECTED(EG(exception))) {
		return;
	}

	if (to_future_state->linked_future_states == NULL) {
		to_future_state->linked_future_states = zend_new_array(0);
	}

	zval z_future_state;
	ZVAL_OBJ(&z_future_state, &from_future_state->notifier.std);

	if (zend_hash_index_update(to_future_state->linked_future_states, from_future_state->notifier.std.handle, &z_future_state) != NULL) {
		//
		// Increase the reference count on the FutureState2 because it is linked to FutureState1.
		//
		GC_ADDREF(&from_future_state->notifier.std);
	}
}

FUTURE_STATE_METHOD(__construct)
{
	ZEND_PARSE_PARAMETERS_NONE();
}

FUTURE_STATE_METHOD(complete)
{
	if (throw_if_future_state_completed(THIS_FUTURE_STATE)) {
		RETURN_THROWS();
	}

	zval *result;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(result)
	ZEND_PARSE_PARAMETERS_END();

	async_future_state_resolve(THIS_FUTURE_STATE, result);
}

FUTURE_STATE_METHOD(error)
{
	if (throw_if_future_state_completed(THIS_FUTURE_STATE)) {
		RETURN_THROWS();
	}

	zend_object * throwable;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OF_CLASS(throwable, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	async_future_state_reject(THIS_FUTURE_STATE, throwable);
}

FUTURE_STATE_METHOD(isComplete)
{
	RETURN_BOOL(Z_TYPE(THIS_FUTURE_STATE->notifier.is_closed) == IS_TRUE);
}

FUTURE_STATE_METHOD(addCallback)
{
	zval* callback;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(callback, async_ce_closure)
	ZEND_PARSE_PARAMETERS_END();

	//
	// Future counted as used if it has at least one callback.
	//
	THIS_FUTURE_STATE->is_used = true;

	async_notifier_add_callback(Z_OBJ_P(ZEND_THIS), callback);

	if (Z_TYPE(THIS_FUTURE_STATE->notifier.is_closed) == IS_TRUE) {
		add_future_state_callbacks_microtask(THIS_FUTURE_STATE);
	}

	RETURN_ZVAL(ZEND_THIS, 1, 0);
}

FUTURE_STATE_METHOD(ignore)
{
	ZVAL_TRUE(&THIS_FUTURE_STATE->notifier.is_closed);
	THIS_FUTURE_STATE->is_used = true;
}

FUTURE_STATE_METHOD(toString)
{
	RETURN_STR(zend_strpprintf(0,
		"FutureObject started at %s:%d",
		THIS_FUTURE_STATE->filename != NULL ? ZSTR_VAL(THIS_FUTURE_STATE->filename) : "<unknown>",
		THIS_FUTURE_STATE->lineno)
    );
}

FUTURE_METHOD(complete)
{

}

FUTURE_METHOD(error)
{

}

FUTURE_METHOD(__construct)
{
	zval* future_state;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(future_state, async_ce_future_state)
	ZEND_PARSE_PARAMETERS_END();

	THIS_FUTURE->future_state = Z_OBJ_P(future_state);
	GC_ADDREF(THIS_FUTURE->future_state);
}

FUTURE_METHOD(isComplete)
{
	async_future_state_t * future_state = (async_future_state_t *) THIS_FUTURE->future_state;
	RETURN_BOOL(Z_TYPE(future_state->notifier.is_closed) == IS_TRUE);
}

FUTURE_METHOD(ignore)
{
	async_future_state_t * future_state = (async_future_state_t *) THIS_FUTURE->future_state;
	ZVAL_TRUE(&future_state->notifier.is_closed);
	future_state->is_used = true;
}

static void new_mapper(INTERNAL_FUNCTION_PARAMETERS, const ASYNC_FUTURE_MAPPER mapper_type)
{
	zval* callable;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(callable)
	ZEND_PARSE_PARAMETERS_END();

	if (!zend_is_callable(callable, 0, NULL)) {
		zend_argument_value_error(1, "Expected parameter to be a valid callable");
		RETURN_THROWS();
	}

	async_future_state_t * future_state = (async_future_state_t *)async_future_state_new();
	zval_copy(&future_state->mapper, callable);
	future_state->mapper_type = mapper_type;

	future_state->is_used = true;

	if (mapper_type == ASYNC_FUTURE_MAPPER_CATCH) {
		// mark that the exception was caught
		future_state->will_exception_caught = true;
	}

	future_state_subscribe_to(future_state, (async_future_state_t *) THIS_FUTURE->future_state);

	if (GC_REFCOUNT(&future_state->notifier.std) == 1) {
		async_throw_error("Future state has wrong reference count");
		RETURN_THROWS();
    }

	GC_DELREF(&future_state->notifier.std);

	RETURN_OBJ(async_future_new(&future_state->notifier.std));
}

FUTURE_METHOD(map)
{
	new_mapper(INTERNAL_FUNCTION_PARAM_PASSTHRU, ASYNC_FUTURE_MAPPER_SUCCESS);
}

FUTURE_METHOD(catch)
{
	new_mapper(INTERNAL_FUNCTION_PARAM_PASSTHRU, ASYNC_FUTURE_MAPPER_CATCH);
}

FUTURE_METHOD(finally)
{
	new_mapper(INTERNAL_FUNCTION_PARAM_PASSTHRU, ASYNC_FUTURE_MAPPER_FINALLY);
}

FUTURE_METHOD(await)
{
	async_await_future((async_future_state_t *) THIS_FUTURE->future_state, return_value);
}

static void future_state_mark_as_closed(async_future_state_t *future_state)
{
	ZVAL_TRUE(&future_state->notifier.is_closed);

	if (future_state->linked_future_states == NULL) {
		return;
	}

	zval *value;
	HashTable * linked_future_states = future_state->linked_future_states;

	// Prevent the forever loop if the linked Future states are linked to each other.
	future_state->linked_future_states = NULL;

	ZEND_HASH_FOREACH_VAL(linked_future_states, value)
	{
		if (EXPECTED(Z_TYPE_P(value) == IS_OBJECT)) {
			future_state_mark_as_closed((async_future_state_t *) Z_OBJ_P(value));
		}
	}
	ZEND_HASH_FOREACH_END();

	zend_array_release(linked_future_states);
}

static void async_future_state_object_destroy(zend_object *object)
{
	async_future_state_t *future_state = (async_future_state_t *) object;

	// Add exception if the future state is not handled but was completed.
	// Ignore if the shutdown is in progress.
	if (false == future_state->will_exception_caught && future_state->throwable != NULL) {
		async_scheduler_transfer_exception(future_state->throwable);
		future_state->throwable = NULL;
		async_warning(
			"Unhandled exception (caught in %s:%d) in the Future state (has been created at %s:%d); %s",
			future_state->completed_filename ? ZSTR_VAL(future_state->completed_filename) : "<unknown>",
			future_state->completed_lineno,
			future_state->filename ? ZSTR_VAL(future_state->filename) : "<unknown>",
			future_state->lineno,
			"Use catch() to handle the exception."
		);
	} else if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE
		&& future_state->is_used == false
		&& ASYNC_G(graceful_shutdown) == false
		) {
		async_scheduler_transfer_exception(
			async_new_exception(
					async_ce_async_exception,
					"Unhandled Future (has been created at %s:%d); %s",
					future_state->filename ? ZSTR_VAL(future_state->filename) : "<unknown>",
					future_state->lineno,
					"Await the Future or use ignore() to suppress this exception."
				)
		);
	}

	zval_ptr_dtor(&future_state->result);
	zval_ptr_dtor(&future_state->mapper);

	if (future_state->linked_future_states != NULL) {

		// Notify all linked Future states about the destruction of the current Future state.
		zval *value;
		HashTable * linked_future_states = future_state->linked_future_states;
		future_state->linked_future_states = NULL;

		ZEND_HASH_FOREACH_VAL(linked_future_states, value)
        {
            if (EXPECTED(Z_TYPE_P(value) == IS_OBJECT)) {
                async_future_state_t * linked_future_state = (async_future_state_t *) Z_OBJ_P(value);
            	zval z_callback;
				ZVAL_OBJ(&z_callback, future_state->callback);
            	async_notifier_remove_callback(&linked_future_state->notifier.std, &z_callback);
            	future_state_mark_as_closed(linked_future_state);
            }
        }
		ZEND_HASH_FOREACH_END();

		zend_array_release(linked_future_states);
	}

	if (future_state->callback != NULL) {
		OBJ_RELEASE(future_state->callback);
	}

	if (future_state->throwable != NULL) {
		OBJ_RELEASE(future_state->throwable);
	}

	if (future_state->context != NULL) {
		OBJ_RELEASE(&future_state->context->std);
	}

	if (future_state->filename != NULL) {
		zend_string_release(future_state->filename);
	}

	if (future_state->completed_filename != NULL) {
		zend_string_release(future_state->completed_filename);
	}

	async_ce_notifier->default_object_handlers->dtor_obj(object);
}

static zend_object *async_future_state_object_create(zend_class_entry *class_entry)
{
	// Allocate memory for the object and initialize it with zero bytes.
	DEFINE_ZEND_RAW_OBJECT(async_future_state_t, future_state, class_entry);

	zend_object_std_init(&future_state->notifier.std, class_entry);
	object_properties_init(&future_state->notifier.std, class_entry);

	ZVAL_UNDEF(&future_state->result);
	ZVAL_UNDEF(&future_state->mapper);

	future_state->linked_future_states = NULL;
	future_state->remove_after_notify = false;

	future_state->mapper_type = ASYNC_FUTURE_MAPPER_SUCCESS;
	future_state->callback = NULL;

	zend_apply_current_filename_and_line(&future_state->filename, &future_state->lineno);
	future_state->is_used = false;
	future_state->context = async_context_current(false, true);

	return &future_state->notifier.std;
}

static void async_future_object_destroy(zend_object *object)
{
	async_future_t *future = (async_future_t *) object;

	if (future->future_state != NULL) {
		OBJ_RELEASE(future->future_state);
		future->future_state = NULL;
	}

	async_ce_notifier->default_object_handlers->dtor_obj(object);
}

static zend_object * async_future_object_create(zend_class_entry *class_entry)
{
	// Allocate memory for the object and initialize it with zero bytes.
	DEFINE_ZEND_RAW_OBJECT(async_future_t, future, class_entry);

	zend_object_std_init(&future->std, class_entry);
	object_properties_init(&future->std, class_entry);

	return &future->std;
}

void async_register_future_ce(void)
{
	async_ce_future_state = register_class_Async_FutureState(async_ce_notifier);
	async_ce_future_state->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_future_state->create_object = async_future_state_object_create;

	memcpy(&async_future_state_handlers, async_ce_notifier->default_object_handlers, sizeof(reactor_notifier_handlers_t));
	async_future_state_handlers.std.dtor_obj = async_future_state_object_destroy;

	// Redefine the handler function.
	async_future_state_handlers.handler_fn = notifier_handler;

	async_ce_future_state->default_object_handlers = &async_future_state_handlers.std;

	async_ce_future = register_class_Async_Future();
	async_ce_future->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_future->create_object = async_future_object_create;

	async_ce_future->default_object_handlers = &async_future_handlers;

	async_future_handlers = std_object_handlers;
	async_future_handlers.dtor_obj = async_future_object_destroy;
	async_future_handlers.clone_obj = NULL;
}

zend_object * async_future_state_new(void)
{
	zval object;

	// Create a new object without calling the constructor
	if (object_init_ex(&object, async_ce_future_state) == FAILURE) {
		return NULL;
	}

	return Z_OBJ_P(&object);
}

zend_object * async_future_new(zend_object * future_state)
{
	zval object;

	// Create a new object without calling the constructor
	if (object_init_ex(&object, async_ce_future) == FAILURE) {
		return NULL;
	}

	((async_future_t *)Z_OBJ_P(&object))->future_state = future_state;
	GC_ADDREF(future_state);

	return Z_OBJ_P(&object);
}

void async_future_state_resolve(async_future_state_t *future_state, zval * retval)
{
	if (UNEXPECTED(Z_TYPE(future_state->notifier.is_closed) == IS_TRUE)) {
		return;
	}

	ZVAL_TRUE(&future_state->notifier.is_closed);

	if (retval != NULL) {
		zval_copy(&future_state->result, retval);
	} else {
		ZVAL_NULL(&future_state->result);
	}

	zend_apply_current_filename_and_line(&future_state->completed_filename, &future_state->completed_lineno);

	add_future_state_callbacks_microtask(future_state);
}

void async_future_state_reject(async_future_state_t *future_state, zend_object * error)
{
	if (UNEXPECTED(Z_TYPE(future_state->notifier.is_closed) == IS_TRUE)) {
		return;
	}

	future_state->throwable = error;
	GC_ADDREF(error);
	ZVAL_TRUE(&future_state->notifier.is_closed);
	zend_apply_current_filename_and_line(&future_state->completed_filename, &future_state->completed_lineno);

	add_future_state_callbacks_microtask(future_state);
}

void async_await_future(async_future_state_t *future_state, zval * retval)
{
	if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE) {
		async_future_state_to_retval(future_state, retval);
		return;
	}

	async_resume_t * resume = async_resume_new(NULL);

	if (resume == NULL) {
		return;
	}

	async_resume_when(resume, &future_state->notifier, false, async_resume_when_callback_resolve);
	async_wait(resume);

	async_future_state_to_retval(future_state, retval);
}

static void future_resume_callback(
	async_resume_t *resume,
	reactor_notifier_t *notifier,
	zval* event,
	zval* error,
	async_resume_notifier_t *resume_notifier
)
{
	future_resume_callback_t * callback = (future_resume_callback_t *) resume_notifier;

	callback->conditions->resolved_count++;

	if (Z_TYPE_P(error) == IS_OBJECT && callback->conditions->ignore_errors == false) {
		async_resume_fiber(resume, NULL, Z_OBJ_P(error));
		return;
	}

	if (callback->conditions->resolved_count >= callback->conditions->total
		|| callback->conditions->resolved_count >= callback->conditions->waiting_count) {
        async_resume_fiber(resume, NULL, NULL);
    }
}

ZEND_API void async_await_future_list(
	zval * iterable,
	int count,
	bool ignore_errors,
	reactor_notifier_t *cancellation,
	zend_ulong timeout,
	HashTable * results,
	HashTable * errors
)
{
	HashTable * futures = NULL;
	zend_object_iterator *zend_iterator = NULL;

	if (Z_TYPE_P(iterable) == IS_ARRAY) {
		futures = Z_ARR_P(iterable);
	} else if (Z_TYPE_P(iterable) == IS_OBJECT && Z_OBJCE_P(iterable)->get_iterator) {
		zend_iterator = Z_OBJCE_P(iterable)->get_iterator(Z_OBJCE_P(iterable), iterable, 0);

		if (EG(exception) == NULL && zend_iterator == NULL) {
			async_throw_error("Failed to create iterator");
		}

    } else {
	    async_throw_error("Expected parameter 'iterable' to be an array or an object implementing Traversable");
    }

	if (UNEXPECTED(EG(exception))) {
		return;
	}

	zend_ulong index;
	zend_string *key;
	zval * z_future_state;

	async_resume_t * resume;

	if (futures != NULL)
	{
		if (zend_hash_num_elements(futures) == 0) {
			return;
		}

		resume = async_new_resume_with_timeout(NULL, timeout, cancellation);

		if (resume == NULL) {
			return;
		}

		future_await_conditions_t * conditions = emalloc(sizeof(future_await_conditions_t));
		conditions->total = (int) zend_hash_num_elements(futures);
		conditions->waiting_count = count > 0 ? count : conditions->total;
		conditions->resolved_count = 0;
		conditions->ignore_errors = ignore_errors;

		ZEND_HASH_FOREACH_KEY_VAL(futures, index, key, z_future_state) {
			if (Z_TYPE_P(z_future_state) != IS_OBJECT) {
				continue;
			}

			// Resolve the Future object to the FutureState object.
			if (false == instanceof_function(Z_OBJCE_P(z_future_state), async_ce_future_state)) {

				if (instanceof_function(Z_OBJCE_P(z_future_state), async_ce_future)) {
					ZVAL_OBJ(z_future_state, ((async_future_t *) Z_OBJ_P(z_future_state))->future_state);
				} else {
					continue;
				}
			}

			async_future_state_t * future_state = (async_future_state_t *) Z_OBJ_P(z_future_state);

			if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE) {
				continue;
			}

			future_resume_callback_t * callback = emalloc(sizeof(future_resume_callback_t));
			ZVAL_PTR(&callback->resume_notifier->callback, future_resume_callback);
			callback->conditions = conditions;

			async_resume_when_ex(resume, &future_state->notifier, false, (async_resume_notifier_t *)callback);

		} ZEND_HASH_FOREACH_END();
	} else {

		resume = async_new_resume_with_timeout(NULL, timeout, cancellation);

		if (resume == NULL) {
			return;
		}

		futures = zend_new_array(8);
		async_future_state_t * future_state = (async_future_state_t *) async_future_state_new();
		async_resume_when(resume, &future_state->notifier, false, async_resume_when_callback_resolve);

		future_await_conditions_t * conditions = emalloc(sizeof(future_await_conditions_t));
		conditions->total = (int) zend_hash_num_elements(futures);
		conditions->waiting_count = count > 0 ? count : conditions->total;
		conditions->resolved_count = 0;
		conditions->ignore_errors = ignore_errors;

		start_concurrent_iterator(future_state, zend_iterator, resume, futures, conditions);
	}

	async_wait(resume);

	// Save the results and errors.

	ZEND_HASH_FOREACH_KEY_VAL(futures, index, key, z_future_state) {

		if (Z_TYPE_P(z_future_state) != IS_OBJECT) {
			continue;
		}

		// Resolve the Future object to the FutureState object.
		if (false == instanceof_function(Z_OBJCE_P(z_future_state), async_ce_future_state)) {

			if (instanceof_function(Z_OBJCE_P(z_future_state), async_ce_future)) {
				ZVAL_OBJ(z_future_state, ((async_future_t *) Z_OBJ_P(z_future_state))->future_state);
			} else {
				continue;
			}
		}

		async_future_state_t * future_state = (async_future_state_t *) Z_OBJ_P(z_future_state);

		if (Z_TYPE(future_state->notifier.is_closed) == IS_TRUE) {

			if (future_state->throwable != NULL) {
				if (errors != NULL) {
					zval error;
					ZVAL_OBJ(&error, future_state->throwable);

					if (key != NULL) {
						zend_hash_update(errors, key, &error);
					} else {
						zend_hash_index_update(errors, index, &error);
					}

					GC_ADDREF(future_state->throwable);
                }
            } else {
                if (results != NULL) {
                    if (key != NULL) {
                    	zend_hash_update(errors, key, &future_state->result);
                    } else {
                    	zend_hash_index_update(results, index, &future_state->result);
                    }

                	Z_TRY_ADDREF_P(&future_state->result);
                }
			}
		}

	} ZEND_HASH_FOREACH_END();

	OBJ_RELEASE(&resume->std);

	if (zend_iterator != NULL) {
		zend_array_release(futures);
	}
}
