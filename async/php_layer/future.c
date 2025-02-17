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

#include "callback.h"
#include "exceptions.h"
#include "future_arginfo.h"
#include "zend_common.h"

#define FUTURE_STATE_METHOD(name) PHP_METHOD(Async_FutureState, name)
#define FUTURE_METHOD(name) PHP_METHOD(Async_Future, name)
#define THIS_FUTURE_STATE ((async_future_state_t *) Z_OBJ_P(ZEND_THIS))
#define THIS_FUTURE ((async_future_t *) Z_OBJ_P(ZEND_THIS))

static zend_object_handlers async_future_state_handlers;
static zend_object_handlers async_future_handlers;

typedef struct {
    async_internal_microtask_t task;
	async_future_state_t *future_state;
	bool started;
	HashPosition position;
} future_state_callbacks_task;

static void future_state_callbacks_task_handler(async_microtask_t *microtask)
{
	future_state_callbacks_task *task = (future_state_callbacks_task *) microtask;
	HashTable * callbacks = Z_ARR(task->future_state->notifier.callbacks);
	reactor_notifier_t * notifier = &task->future_state->notifier;

	if (task->started == false) {
		task->started = true;
		zend_hash_internal_pointer_reset_ex(callbacks, &task->position);
	}

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
	async_scheduler_add_microtask_ex(microtask);

	while (false == microtask->is_cancelled) {
		zval *current = zend_hash_get_current_data_ex(callbacks, &task->position);

		if (current == NULL) {
			microtask->is_cancelled = true;
			break;
		}

		zend_hash_move_forward_ex(callbacks, &task->position);

		if (EXPECTED(Z_TYPE_P(current) == IS_OBJECT)) {
			zend_resolve_weak_reference(current, &resolved_callback);

			// Resume object and Callback object use different handlers.
			if (Z_TYPE(resolved_callback) == IS_OBJECT && Z_OBJ_P(&resolved_callback)->ce == async_ce_resume) {
				async_resume_notify((async_resume_t *) Z_OBJ(resolved_callback), notifier, &result, &error);
			} else if (Z_TYPE(resolved_callback) == IS_OBJECT) {
				async_callback_notify(Z_OBJ(resolved_callback), &notifier->std, &result, &error);
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

zend_always_inline void add_future_state_callbacks_microtask(async_future_state_t *future_state)
{
	future_state_callbacks_task *microtask = pecalloc(1, sizeof(future_state_callbacks_task), 0);
	microtask->task.task.is_fci = false;
	microtask->task.handler = future_state_callbacks_task_handler;
	microtask->task.task.dtor = future_state_callbacks_task_dtor;
	microtask->future_state = future_state;
	microtask->position = -1;
	microtask->started = false;

	if (ASYNC_G(in_scheduler_context)) {
		future_state_callbacks_task_handler((async_microtask_t *) microtask);
		async_scheduler_microtask_free((async_microtask_t *) microtask);
	} else {
		async_scheduler_add_microtask_ex((async_microtask_t *) microtask);
	}
}

zend_always_inline bool throw_if_future_state_completed(async_future_state_t *future_state)
{
	if (Z_TYPE(future_state->notifier.is_terminated) == IS_TRUE) {
		async_throw_error(
			"The Future has already been completed at %s:%d",
			future_state->filename ? ZSTR_VAL(future_state->filename) : "<unknown>",
			future_state->lineno
		);

		return true;
	}

	return false;
}

static void notifier_handler(reactor_notifier_t* notifier, zval* event, zval* error)
{
	add_future_state_callbacks_microtask((async_future_state_t *) notifier);
}

FUTURE_STATE_METHOD(__construct)
{
	ZEND_PARSE_PARAMETERS_NONE();

	async_future_state_t* future_state = THIS_FUTURE_STATE;
	zend_apply_current_filename_and_line(&future_state->filename, &future_state->lineno);

	// Redefine the notifier handler: change behavior async_notifier_notify to future_state_callbacks_task_handler
	future_state->notifier.handler_fn = notifier_handler;
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

	ZVAL_TRUE(&THIS_FUTURE_STATE->notifier.is_terminated);
	zval_copy(&THIS_FUTURE_STATE->result, result);
	zend_apply_current_filename_and_line(&THIS_FUTURE_STATE->completed_filename, &THIS_FUTURE_STATE->completed_lineno);

	add_future_state_callbacks_microtask(THIS_FUTURE_STATE);
}

FUTURE_STATE_METHOD(error)
{
	if (throw_if_future_state_completed(THIS_FUTURE_STATE)) {
		RETURN_THROWS();
	}

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OF_CLASS(THIS_FUTURE_STATE->throwable, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	ZVAL_TRUE(&THIS_FUTURE_STATE->notifier.is_terminated);
	zend_apply_current_filename_and_line(&THIS_FUTURE_STATE->completed_filename, &THIS_FUTURE_STATE->completed_lineno);
	add_future_state_callbacks_microtask(THIS_FUTURE_STATE);
}

FUTURE_STATE_METHOD(isComplete)
{
	RETURN_BOOL(Z_TYPE(THIS_FUTURE_STATE->notifier.is_terminated) == IS_TRUE);
}

FUTURE_STATE_METHOD(ignore)
{
	ZVAL_TRUE(&THIS_FUTURE_STATE->notifier.is_terminated);
	THIS_FUTURE_STATE->is_handled = true;
}

FUTURE_STATE_METHOD(__toString)
{
	RETURN_STR(zend_strpprintf(0,
		"FutureObject started at %s:%d",
		THIS_FUTURE_STATE->filename != NULL ? ZSTR_VAL(THIS_FUTURE_STATE->filename) : "<unknown>",
		THIS_FUTURE_STATE->lineno)
    );
}

FUTURE_METHOD(__construct)
{
	zval* future_state;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(future_state, async_ce_future_state)
	ZEND_PARSE_PARAMETERS_END();

	zval_copy(&THIS_FUTURE->future_state, future_state);
}

FUTURE_METHOD(isComplete)
{
	async_future_state_t * future_state = (async_future_state_t *) Z_OBJ(THIS_FUTURE->future_state);
	RETURN_BOOL(Z_TYPE(future_state->notifier.is_terminated) == IS_TRUE);
}

FUTURE_METHOD(ignore)
{
	async_future_state_t * future_state = (async_future_state_t *) Z_OBJ(THIS_FUTURE->future_state);
	ZVAL_TRUE(&future_state->notifier.is_terminated);
	future_state->is_handled = true;
}

FUTURE_METHOD(map)
{
}

FUTURE_METHOD(catch)
{
}

FUTURE_METHOD(finally)
{
}

FUTURE_METHOD(await)
{
}

static void async_future_state_object_destroy(zend_object *object)
{
	async_future_state_t *future_state = (async_future_state_t *) object;

	// Add exception if the future state is not handled but was completed.
	// Ignore if the shutdown is in progress.
	if (Z_TYPE(future_state->notifier.is_terminated) == IS_TRUE
		&& future_state->is_handled == false
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

	if (future_state->filename != NULL) {
		zend_string_release(future_state->filename);
	}

	if (future_state->completed_filename != NULL) {
		zend_string_release(future_state->completed_filename);
	}

	async_ce_notifier->default_object_handlers->dtor_obj(object);
}

void async_register_future_ce(void)
{
	async_ce_future_state = register_class_Async_FutureState(async_ce_notifier);
	async_ce_future_state->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_future->default_object_handlers = &async_future_state_handlers;

	memcpy(&async_future_state_handlers, async_ce_notifier->default_object_handlers, sizeof(zend_object_handlers));
	async_future_state_handlers.dtor_obj = async_future_state_object_destroy;

	async_ce_future = register_class_Async_Future();
	async_ce_future->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	async_ce_future->default_object_handlers = &async_future_handlers;

	async_future_handlers = std_object_handlers;
	async_future_handlers.clone_obj = NULL;
}
