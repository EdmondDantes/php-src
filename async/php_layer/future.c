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
#include "future_arginfo.h"

#define FUTURE_STATE_METHOD(name) PHP_METHOD(Async_FutureState, name)
#define FUTURE_METHOD(name) PHP_METHOD(Async_Future, name)

static zend_object_handlers async_future_state_handlers;
static zend_object_handlers async_future_handlers;

static void async_future_state_object_destroy(zend_object *object)
{
	async_future_state_t *future_state = (async_future_state_t *) object;

	// Add exception if the future state is not handled but was completed.
	if (Z_TYPE(future_state->notifier.is_terminated) == IS_TRUE && future_state->is_handled == false) {
		// TODO: add special function to put exception to scheduler
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
