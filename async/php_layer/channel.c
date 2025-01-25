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

#include "callback.h"
#include "channel_arginfo.h"
#include "async/php_async.h"
#include "async/php_scheduler.h"
#include "async/internal/circular_buffer.h"
#include "async/internal/zval_circular_buffer.h"

#define METHOD(name) PHP_METHOD(Async_Channel, name)
#define THIS_CHANNEL ((async_channel_t *)((char *)Z_OBJ_P(ZEND_THIS) - XtOffsetOf(async_channel_t, std)))
#define THIS(field) THIS_CHANNEL->field
#define THROW_IF_CLOSED if (UNEXPECTED(THIS(closed))) { \
	zend_throw_exception(async_ce_channel_was_closed_exception, "Channel was closed", 0); return; \
	RETURN_THROWS(); \
}

static bool emit_data_pushed(async_channel_t *channel);
static void emit_data_popped(async_channel_t *channel);
static void emit_channel_closed(async_channel_t *channel);

METHOD(__construct)
{
	zend_long capacity = 8;
	zend_object * owner = NULL;
	zend_long expandable = false;

	ZEND_PARSE_PARAMETERS_START(0, 3)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(capacity)
		Z_PARAM_OBJECT_OF_CLASS(owner, zend_ce_fiber)
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

	if (EG(active_fiber) != NULL) {
		async_channel_set_owner_fiber(Z_OBJ_P(ZEND_THIS), EG(active_fiber));
	} else if (owner != NULL) {
		async_channel_set_owner_fiber(Z_OBJ_P(ZEND_THIS), (zend_fiber *) owner);
	} else {
		ZVAL_NULL(async_channel_get_owner(Z_OBJ_P(ZEND_THIS)));
	}

	THIS(notifier) = async_notifier_new_by_class(sizeof(reactor_notifier_t), async_ce_channel_notifier);
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
		Z_PARAM_OBJ_OF_CLASS(cancellation, async_ce_notifier)
		Z_PARAM_BOOL(waitOnFull)
	ZEND_PARSE_PARAMETERS_END();

	THROW_IF_CLOSED

	zval * owner = async_channel_get_owner(Z_OBJ_P(ZEND_THIS));

	if (UNEXPECTED(owner != NULL && Z_OBJ_P(owner) != &EG(active_fiber)->std)) {
		zend_throw_exception(async_ce_channel_exception, "Only owner fiber can send data to the channel", 0);
		RETURN_THROWS();
	}

	async_channel_t * channel = THIS_CHANNEL;

	if (UNEXPECTED(waitOnFull && false == channel->expandable && circular_buffer_is_full(&channel->buffer))) {
		async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, (reactor_notifier_t *) cancellation);

		async_resume_when(resume, channel->notifier, false, async_resume_when_callback_resolve);
		async_await(resume);
		OBJ_RELEASE(&resume->std);

		if (UNEXPECTED(EG(exception))) {
			RETURN_THROWS();
		}
	} else if (UNEXPECTED(false == channel->expandable && circular_buffer_is_full(&channel->buffer))) {
		zend_throw_exception(async_ce_channel_is_full_exception, "Channel is full", 0);
		RETURN_THROWS();
	}

	zval_c_buffer_push(&channel->buffer, data);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	emit_data_pushed(channel);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	if (IS_ASYNC_HAS_DEFER_FIBER) {
		async_await_timeout(timeout, (reactor_notifier_t *) cancellation);
	}

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}
}

METHOD(sendAsync)
{
	zval *data;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(data)
	ZEND_PARSE_PARAMETERS_END();

	THROW_IF_CLOSED

	zval * owner = async_channel_get_owner(Z_OBJ_P(ZEND_THIS));

	if (UNEXPECTED(owner != NULL && Z_OBJ_P(owner) != &EG(active_fiber)->std)) {
		zend_throw_exception(async_ce_channel_exception, "Only owner fiber can send data to the channel", 0);
		RETURN_THROWS();
	}

	async_channel_t * channel = THIS_CHANNEL;

	if (UNEXPECTED(false == channel->expandable && circular_buffer_is_full(&channel->buffer))) {
		zend_throw_exception(async_ce_channel_is_full_exception, "Channel is full", 0);
		RETURN_THROWS();
	}

	zval_c_buffer_push(&channel->buffer, data);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	emit_data_pushed(channel);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}
}

METHOD(receive)
{
	zend_long timeout = 0;
	zend_object *cancellation = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(timeout)
		Z_PARAM_OBJ_OF_CLASS(cancellation, async_ce_notifier)
	ZEND_PARSE_PARAMETERS_END();

	THROW_IF_CLOSED

	zval * owner = async_channel_get_owner(Z_OBJ_P(ZEND_THIS));

	if (UNEXPECTED(owner != NULL && Z_OBJ_P(owner) == &EG(active_fiber)->std)) {
		zend_throw_exception(async_ce_channel_exception, "Owner fiber cannot receive data from the channel", 0);
		RETURN_THROWS();
	}

	async_channel_t * channel = THIS_CHANNEL;

	if (circular_buffer_is_not_empty(&channel->buffer)) {
		zval_c_buffer_pop(&channel->buffer, return_value);
		return;
	}

	async_await_timeout(timeout, (reactor_notifier_t *) cancellation);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	if (UNEXPECTED(circular_buffer_is_empty(&channel->buffer))) {
		RETURN_NULL();
	}

	zval_c_buffer_pop(&channel->buffer, return_value);

	if (EG(exception)) {
		emit_data_popped(channel);
		RETURN_THROWS();
	}

	emit_data_popped(channel);

	if (EG(exception)) {
		RETURN_THROWS();
	}
}

METHOD(receiveAsync)
{
	THROW_IF_CLOSED

	zval * owner = async_channel_get_owner(Z_OBJ_P(ZEND_THIS));

	if (UNEXPECTED(owner != NULL && Z_OBJ_P(owner) == &EG(active_fiber)->std)) {
		zend_throw_exception(async_ce_channel_exception, "Owner fiber cannot receive data from the channel", 0);
		RETURN_THROWS();
	}

	async_channel_t * channel = THIS_CHANNEL;

	if (circular_buffer_is_empty(&channel->buffer)) {
		RETURN_NULL();
	}

	zval_c_buffer_pop(&channel->buffer, return_value);

	if (EG(exception)) {
		emit_data_popped(channel);
		RETURN_THROWS();
	}

	emit_data_popped(channel);

	if (EG(exception)) {
		RETURN_THROWS();
	}
}

METHOD(close)
{
	if (THIS(closed)) {
        return;
    }

	THIS(closed) = true;
	emit_channel_closed(THIS_CHANNEL);
	zval_c_buffer_cleanup(&THIS(buffer));
	circular_buffer_dtor(&THIS(buffer));
}

METHOD(isClosed)
{

}

METHOD(isFull)
{
	RETURN_BOOL(circular_buffer_is_full(&THIS(buffer)));
}

METHOD(isEmpty)
{
	RETURN_BOOL(circular_buffer_is_empty(&THIS(buffer)));
}

METHOD(isNotEmpty)
{
	RETURN_BOOL(circular_buffer_is_not_empty(&THIS(buffer)));
}

METHOD(getCapacity)
{
	RETURN_LONG(circular_buffer_capacity(&THIS(buffer)));
}

METHOD(getUsed)
{
	RETURN_LONG(circular_buffer_count(&THIS(buffer)));
}

METHOD(getNotifier)
{
	RETURN_OBJ_COPY(THIS(notifier));
}

static void async_channel_object_destroy(zend_object* object)
{
	async_channel_t *channel = CHANNEL_FROM_ZEND_OBJ(object);

	if (channel->notifier != NULL) {
		OBJ_RELEASE(&channel->notifier->std);
		channel->notifier = NULL;
	}

	if (channel->buffer.start != NULL) {
		zval_c_buffer_cleanup(&channel->buffer);
		circular_buffer_dtor(&channel->buffer);
		channel->buffer.start = NULL;
	}
}

static bool emit_data_pushed(async_channel_t *channel)
{
	channel->data_popped = false;

	// 1. Get first notifier from the list
	HashTable *callbacks = Z_ARR(channel->notifier->callbacks);

	zval *current;
	zval resolved_callback;
	zval event;
	ZVAL_LONG(&event, ASYNC_DATA_PUSHED);
	bool was_resumed = false;

	ZEND_HASH_FOREACH_VAL(callbacks, current)

		if (EXPECTED(Z_TYPE_P(current) == IS_OBJECT)) {
			zend_resolve_weak_reference(current, &resolved_callback);
			// Resume object and Callback object use different handlers.
			if (Z_TYPE(resolved_callback) == IS_OBJECT && Z_OBJ_P(&resolved_callback)->ce == async_ce_resume) {

				if (false == was_resumed) {
					async_resume_notify((async_resume_t *) Z_OBJ(resolved_callback), channel->notifier, &event, NULL);
					was_resumed = ((async_resume_t *) Z_OBJ(resolved_callback))->status == ASYNC_RESUME_SUCCESS;
				}

			} else if (Z_TYPE(resolved_callback) == IS_OBJECT) {
				async_callback_notify(Z_OBJ(resolved_callback), &channel->notifier->std, &event, NULL);
			}

			zval_ptr_dtor(&resolved_callback);
		}
	ZEND_HASH_FOREACH_END();

	return was_resumed;
}

static void emit_data_popped(async_channel_t *channel)
{
	if (channel->data_popped) {
        return;
    }

	zval event;
	ZVAL_LONG(&event, ASYNC_DATA_POPPED);
	async_notifier_notify(channel->notifier, &event, NULL);

	if (EG(exception)) {
		channel->data_popped = true;
	}
}

static void emit_channel_closed(async_channel_t *channel)
{
	channel->data_popped = false;
	channel->closed = true;

	zend_object * cancellable = async_new_exception(async_ce_channel_was_closed_exception, "Channel was closed");
	zval event, error;
	ZVAL_LONG(&event, ASYNC_CHANNEL_CLOSED);
	ZVAL_OBJ(&error, cancellable);

	async_notifier_notify(channel->notifier, &event, &error);
	OBJ_RELEASE(cancellable);
}


static zend_object_handlers async_channel_handlers;

void async_register_channel_ce(void)
{
	// interfaces
	async_ce_producer_i = register_class_Async_ProducerInterface();
	async_ce_consumer_i = register_class_Async_ConsumerInterface();
	async_ce_channel_state_i = register_class_Async_ChannelStateInterface();
	async_ce_channel_i = register_class_Async_ChannelInterface(async_ce_producer_i, async_ce_consumer_i, async_ce_channel_state_i);

	async_ce_channel_notifier = register_class_Async_ChannelNotifier(async_ce_notifier);
	async_ce_channel = register_class_Async_Channel(async_ce_channel_i);

	async_ce_channel->default_object_handlers = &async_channel_handlers;
	async_ce_channel->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_channel->create_object = NULL;

	async_channel_handlers = std_object_handlers;
	async_channel_handlers.dtor_obj = async_channel_object_destroy;
	async_channel_handlers.clone_obj = NULL;
}