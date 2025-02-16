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

#include "exceptions.h"
#include "future_arginfo.h"
#include "zend_common.h"

#define FUTURE_STATE_METHOD(name) PHP_METHOD(Async_FutureState, name)
#define FUTURE_METHOD(name) PHP_METHOD(Async_Future, name)
#define THIS_FUTURE_STATE ((async_future_state_t *) Z_OBJ_P(ZEND_THIS))

static zend_object_handlers async_future_state_handlers;
static zend_object_handlers async_future_handlers;

static void invoke_future_state_callbacks(async_microtask_t *microtask);

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

FUTURE_STATE_METHOD(__construct)
{
	async_future_state_t* future_state = THIS_FUTURE_STATE;
	zend_apply_current_filename_and_line(&future_state->filename, &future_state->lineno);
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

	// Call all callbacks in the microtask.
	async_scheduler_add_microtask_handler(invoke_future_state_callbacks, Z_OBJ_P(ZEND_THIS));
}

FUTURE_STATE_METHOD(error)
{
	if (throw_if_future_state_completed(THIS_FUTURE_STATE)) {
		RETURN_THROWS();
	}

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJ_OF_CLASS(THIS_FUTURE_STATE->throwable, zend_ce_throwable)
	ZEND_PARSE_PARAMETERS_END();

	zend_apply_current_filename_and_line(&THIS_FUTURE_STATE->completed_filename, &THIS_FUTURE_STATE->completed_lineno);
	async_scheduler_add_microtask_handler(invoke_future_state_callbacks, Z_OBJ_P(ZEND_THIS));
}

FUTURE_STATE_METHOD(isComplete)
{
	RETURN_BOOL(Z_TYPE(THIS_FUTURE_STATE->notifier.is_terminated) == IS_TRUE);
}

FUTURE_STATE_METHOD(ignore)
{
	THIS_FUTURE_STATE->is_handled = true;
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
