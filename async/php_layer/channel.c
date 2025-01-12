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
#include "channel.h"
#include "channel_arginfo.h"
#include "async/php_async.h"
#include "async/php_scheduler.h"
#include "async/internal/circular_buffer.h"
#include "async/internal/zval_circular_buffer.h"

#define METHOD(name) PHP_METHOD(Async_Channel, name)
#define THIS_CHANNEL ((async_channel_t *)((char *)Z_OBJ_P(ZEND_THIS) - XtOffsetOf(async_channel_t, std)))
#define THIS(field) THIS_CHANNEL->field

METHOD(__construct)
{
	zend_long capacity = 8;
	zend_long direction = ASYNC_CHANNEL_DIRECTION_RECEIVE;
	zend_bool expandable = false;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(capacity)
		Z_PARAM_LONG(direction)
		Z_PARAM_BOOL(expandable)
	ZEND_PARSE_PARAMETERS_END();

	if (capacity < 0) {
		zend_argument_value_error(1, "must be greater than zero");
		return;
	}

	if (capacity == 0) {
		capacity = 8;
	}

	circular_buffer_t * buffer = &THIS(buffer);

	if (UNEXPECTED(circular_buffer_ctor(buffer, capacity, sizeof(zval), &zend_std_persistent_allocator) == FAILURE)) {
		zend_throw_error(NULL, "Failed to allocate memory for channel buffer");
		RETURN_THROWS();
	}

	THIS(expandable) = expandable;
	THIS(direction) = (int)direction;

	if (direction != ASYNC_CHANNEL_DIRECTION_BOTH && EG(active_fiber)) {
		async_channel_set_owner_fiber(Z_OBJ_P(ZEND_THIS), EG(active_fiber));
	} else {
		ZVAL_NULL(async_channel_get_owner(Z_OBJ_P(ZEND_THIS)));
	}
}

METHOD(send)
{
	zval *data;
	zend_long timeout = 0;
	zend_object *cancellation = NULL;
	zend_bool waitOnFull = true;

	ZEND_PARSE_PARAMETERS_START(1, 4)
		Z_PARAM_ZVAL(data)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(timeout)
		Z_PARAM_OBJ(cancellation, async_ce_notifier)
		Z_PARAM_BOOL(waitOnFull)
	ZEND_PARSE_PARAMETERS_END();
}

METHOD(sendAsync)
{

}

METHOD(receive)
{

}

METHOD(receiveAsync)
{

}

METHOD(close)
{

}

METHOD(isClosed)
{

}

METHOD(isFull)
{

}

METHOD(isEmpty)
{

}

METHOD(isNotEmpty)
{

}

METHOD(getCapacity)
{

}

METHOD(getUsed)
{

}


static zend_object_handlers async_channel_handlers;

void async_register_channel_ce(void)
{
	async_ce_channel = register_class_Async_Channel();

	async_ce_channel->default_object_handlers = &async_channel_handlers;
	async_ce_channel->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_channel->create_object = async_channel_object_create;

	async_channel_handlers = std_object_handlers;
	async_channel_handlers.dtor_obj = async_channel_object_destroy;
	async_channel_handlers.free_obj = async_channel_object_free;
	async_channel_handlers.clone_obj = NULL;
}