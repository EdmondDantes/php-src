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

#include "closure.h"
#include "channel_arginfo.h"
#include "async/php_async.h"
#include "async/php_scheduler.h"
#include "async/internal/circular_buffer.h"
#include "async/internal/zval_circular_buffer.h"

#define METHOD(name) PHP_METHOD(Async_Channel, name)
#define THIS_CHANNEL ((async_channel_t *)((char *)Z_OBJ_P(ZEND_THIS) - XtOffsetOf(async_channel_t, std)))
#define THIS(field) THIS_CHANNEL->field

#define THROW_IF_CLOSED if (UNEXPECTED(THIS(closed))) { \
		zend_throw_exception(async_ce_channel_was_closed_exception, "Channel was closed", 0); \
		RETURN_THROWS(); \
	}

#define THROW_IF_CLOSED_AND_EMPTY if (UNEXPECTED(THIS(closed) && circular_buffer_is_empty(&THIS(buffer)))) { \
		zend_throw_exception(async_ce_channel_was_closed_exception, "Channel was closed", 0); \
		RETURN_THROWS(); \
	}

static bool emit_data_pushed(async_channel_t *channel);
static void emit_data_popped(async_channel_t *channel);
static void emit_channel_closed(async_channel_t *channel);

static void resume_when_data_pushed(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error, async_resume_notifier_t *resume_notifier);
static void resume_when_data_popped(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error, async_resume_notifier_t *resume_notifier);

zend_always_inline void close_channel(async_channel_t *channel)
{
	channel->closed = true;
	emit_channel_closed(channel);
}

static void on_fiber_finished(
	zend_fiber * fiber, zend_fiber_defer_callback * callback, zend_object * exception, bool * capture_exception
)
{
	async_channel_t * channel = CHANNEL_FROM_ZEND_OBJ(callback->object);
	channel->fiber_callback_index = -1;
	close_channel(channel);

	ZEND_ASSERT(async_channel_get_owner_fiber(callback->object) == fiber && "Fiber is not the owner of the channel");
}

METHOD(__construct)
{
	zend_long capacity = 8;
	zend_object * owner = NULL;
	bool expandable = false;

	ZEND_PARSE_PARAMETERS_START(0, 3)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(capacity)
		Z_PARAM_OBJ_OF_CLASS(owner, zend_ce_fiber)
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
	zend_fiber * owner_fiber = NULL;

	THIS(finish_producing) = false;
	THIS(closed) = false;

	if (EG(active_fiber) != NULL) {
		async_channel_set_owner_fiber(Z_OBJ_P(ZEND_THIS), EG(active_fiber));
		owner_fiber = EG(active_fiber);
	} else if (owner != NULL) {
		async_channel_set_owner_fiber(Z_OBJ_P(ZEND_THIS), (zend_fiber *) owner);
		owner_fiber = (zend_fiber *) owner;
	} else {
		ZVAL_NULL(async_channel_get_owner(Z_OBJ_P(ZEND_THIS)));
	}

	THIS(notifier) = async_notifier_new_by_class(sizeof(reactor_notifier_t), async_ce_channel_notifier);

	if (owner_fiber != NULL) {
		THIS(fiber_callback_index) = zend_fiber_defer(
				owner_fiber,
				zend_fiber_create_defer_callback(on_fiber_finished, Z_OBJ_P(ZEND_THIS), true),
				true
			);
	} else {
		THIS(fiber_callback_index) = -1;
	}
}

METHOD(send)
{
	zval *data;
	zend_long timeout = 0;
	zend_object *cancellation = NULL;
	bool waitOnFull = true;

	ZEND_PARSE_PARAMETERS_START(1, 4)
		Z_PARAM_ZVAL(data)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(timeout)
		Z_PARAM_OBJ_OF_CLASS(cancellation, async_ce_notifier)
		Z_PARAM_BOOL(waitOnFull)
	ZEND_PARSE_PARAMETERS_END();

	THROW_IF_CLOSED

	const zend_fiber * owner = async_channel_get_owner_fiber(Z_OBJ_P(ZEND_THIS));

	if (UNEXPECTED(owner != NULL && owner != EG(active_fiber))) {
		zend_throw_exception(async_ce_channel_exception, "Only owner fiber can send data to the channel", 0);
		RETURN_THROWS();
	}

	async_channel_t * channel = THIS_CHANNEL;

	if (UNEXPECTED(waitOnFull && false == channel->expandable && circular_buffer_is_full(&channel->buffer))) {
		async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, (reactor_notifier_t *) cancellation);

		async_resume_when(resume, channel->notifier, false, resume_when_data_popped);
		async_wait(resume);
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
		async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, (reactor_notifier_t *) cancellation);
		async_resume_when(resume, channel->notifier, false, resume_when_data_popped);
		async_wait(resume);
		OBJ_RELEASE(&resume->std);
	}

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}
}

METHOD(trySend)
{
	zval *data;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(data)
	ZEND_PARSE_PARAMETERS_END();

	THROW_IF_CLOSED

	const zend_fiber * owner = async_channel_get_owner_fiber(&THIS_CHANNEL->std);

	if (UNEXPECTED(owner != NULL && owner != EG(active_fiber))) {
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

	const zend_fiber * owner = async_channel_get_owner_fiber(&THIS_CHANNEL->std);

	if (UNEXPECTED(owner != NULL && owner == EG(active_fiber))) {
		zend_throw_exception(async_ce_channel_exception, "Owner fiber cannot receive data from the channel", 0);
		RETURN_THROWS();
	}

	async_channel_t * channel = THIS_CHANNEL;

	if (EXPECTED(circular_buffer_is_not_empty(&channel->buffer))) {
		zval_c_buffer_pop(&channel->buffer, return_value);

		if(UNEXPECTED(EG(exception))) {
			RETURN_THROWS();
		}

		emit_data_popped(channel);

		if(UNEXPECTED(EG(exception))) {
            zval_ptr_dtor(return_value);
			RETURN_THROWS();
		}

		return;
	}

	if (UNEXPECTED(THIS(closed))) {
		RETURN_NULL();
	}

	async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, (reactor_notifier_t *) cancellation);
	async_resume_when(resume, channel->notifier, false, resume_when_data_pushed);

	async_wait(resume);
	OBJ_RELEASE(&resume->std);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	if (UNEXPECTED(THIS(closed))) {
		RETURN_NULL();
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
		zval_ptr_dtor(return_value);
		RETURN_THROWS();
	}
}

METHOD(tryReceive)
{
	const zend_fiber * owner = async_channel_get_owner_fiber(&THIS_CHANNEL->std);

	if (UNEXPECTED(owner != NULL && owner == EG(active_fiber))) {
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
		zval_ptr_dtor(return_value);
		RETURN_THROWS();
	}
}

METHOD(waitUntilWritable)
{
	zend_long timeout = 0;
	zend_object *cancellation = NULL;

	if (false == circular_buffer_is_full(&THIS(buffer))) {
		RETURN_TRUE;
	}

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(timeout)
		Z_PARAM_OBJ_OF_CLASS(cancellation, async_ce_notifier)
	ZEND_PARSE_PARAMETERS_END();

	async_channel_t * channel = THIS_CHANNEL;

	async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, (reactor_notifier_t *) cancellation);

	async_resume_when(resume, channel->notifier, false, resume_when_data_popped);
	async_wait(resume);
	OBJ_RELEASE(&resume->std);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	if (UNEXPECTED(channel->closed && circular_buffer_is_empty(&channel->buffer))) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

METHOD(waitUntilReadable)
{
	zend_long timeout = 0;
	zend_object *cancellation = NULL;

	if (circular_buffer_is_not_empty(&THIS(buffer))) {
		RETURN_TRUE;
	}

	if (THIS(closed)) {
		RETURN_FALSE;
	}

	ZEND_PARSE_PARAMETERS_START(0, 2)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(timeout)
		Z_PARAM_OBJ_OF_CLASS(cancellation, async_ce_notifier)
	ZEND_PARSE_PARAMETERS_END();

	async_channel_t * channel = THIS_CHANNEL;

	async_resume_t *resume = async_new_resume_with_timeout(NULL, timeout, (reactor_notifier_t *) cancellation);
	async_resume_when(resume, channel->notifier, false, resume_when_data_pushed);

	async_wait(resume);
	OBJ_RELEASE(&resume->std);

	if (UNEXPECTED(EG(exception))) {
		RETURN_THROWS();
	}

	if (UNEXPECTED(channel->closed && circular_buffer_is_empty(&channel->buffer))) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

METHOD(finishProducing)
{
	if (THIS(closed)) {
        return;
    }

	const zend_fiber * owner = async_channel_get_owner_fiber(&THIS_CHANNEL->std);

	if (UNEXPECTED(owner != NULL && owner != EG(active_fiber))) {
		zend_throw_exception(async_ce_channel_exception, "Only owner fiber can finish producing", 0);
		RETURN_THROWS();
	}

	close_channel(THIS_CHANNEL);

	if (THIS(fiber_callback_index) != -1) {
		zend_fiber_remove_defer(owner, THIS(fiber_callback_index));
		THIS(fiber_callback_index) = -1;
	}
}

METHOD(finishConsuming)
{
	if (THIS(closed)) {
        return;
    }

	const zend_fiber * owner = async_channel_get_owner_fiber(&THIS_CHANNEL->std);

	if (UNEXPECTED(owner != NULL && owner == EG(active_fiber))) {
		zend_throw_exception(async_ce_channel_exception, "Owner fiber cannot finish consuming", 0);
		RETURN_THROWS();
	}

	if (UNEXPECTED(circular_buffer_is_not_empty(&THIS_CHANNEL->buffer))) {
		async_warning("Try to close the channel with data. The data will be lost.");
	}

	close_channel(THIS_CHANNEL);
}

METHOD(discardData)
{
	zval_c_buffer_cleanup(&THIS(buffer));
}

METHOD(close)
{
	if (THIS(closed)) {
        return;
    }

	close_channel(THIS_CHANNEL);
}

METHOD(isClosed)
{
	RETURN_BOOL(THIS(closed));
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

METHOD(isProducingFinished)
{
	RETURN_BOOL(THIS(closed) && circular_buffer_is_empty(&THIS(buffer)));
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
	RETURN_OBJ_COPY((zend_object *) THIS(notifier));
}

///
/// Iterator methods
///

METHOD(current)
{
	ZEND_MN(Async_Channel_receive)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

METHOD(key)
{
	RETURN_NULL();
}

METHOD(next)
{
}

METHOD(rewind)
{
}

METHOD(valid)
{
	RETURN_BOOL(false == (THIS(closed) && circular_buffer_is_empty(&THIS(buffer))));
}

METHOD(count)
{
	RETURN_LONG(circular_buffer_count(&THIS(buffer)));
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
				async_closure_notify(Z_OBJ(resolved_callback), &channel->notifier->std, &event, NULL);
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

	zval event;
	ZVAL_LONG(&event, ASYNC_CHANNEL_CLOSED);
	async_notifier_notify(channel->notifier, &event, NULL);
}

static zend_object *async_channel_object_create(zend_class_entry *class_entry)
{
	const size_t size = sizeof(async_channel_t) + zend_object_properties_size(class_entry);
	async_channel_t * object = emalloc(size);

	zend_object_std_init(&object->std, class_entry);
	object_properties_init(&object->std, class_entry);

	return &object->std;
}

static void async_channel_object_destroy(zend_object* object)
{
	async_channel_t *channel = CHANNEL_FROM_ZEND_OBJ(object);

	if (false == channel->closed) {
		close_channel(channel);
	}

	if (channel->fiber_callback_index != -1) {
		const zend_fiber *fiber = async_channel_get_owner_fiber(object);

		if (fiber != NULL) {
			zend_fiber_remove_defer(fiber, channel->fiber_callback_index);
			channel->fiber_callback_index = -1;
		}
	}

	if (channel->notifier != NULL) {
		OBJ_RELEASE(&channel->notifier->std);
		channel->notifier = NULL;
	}

	zval_c_buffer_cleanup(&channel->buffer);
	circular_buffer_dtor(&channel->buffer);
}

static void resume_when_data_pushed(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error, async_resume_notifier_t *resume_notifier)
{
	if (error != NULL && Z_TYPE_P(error) == IS_OBJECT) {
		async_resume_fiber(resume, NULL, Z_OBJ_P(error));
		return;
	}

	if (Z_LVAL_P(event) == ASYNC_DATA_PUSHED || Z_LVAL_P(event) == ASYNC_CHANNEL_CLOSED) {
        async_resume_fiber(resume, NULL, NULL);
    }
}

static void resume_when_data_popped(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error, async_resume_notifier_t *resume_notifier)
{
	if (error != NULL && Z_TYPE_P(error) == IS_OBJECT) {
		async_resume_fiber(resume, NULL, Z_OBJ_P(error));
		return;
	}

	if (Z_LVAL_P(event) == ASYNC_DATA_POPPED || Z_LVAL_P(event) == ASYNC_CHANNEL_CLOSED) {
        async_resume_fiber(resume, NULL, NULL);
    }
}

static void async_channel_object_free(zend_object *object)
{
	zend_object_std_dtor(object);
}

static zend_object_handlers async_channel_handlers;

void async_register_channel_ce(void)
{
	// interfaces
	async_ce_channel_state_i = register_class_Async_ChannelStateInterface();
	async_ce_producer_i = register_class_Async_ProducerInterface(async_ce_channel_state_i);
	async_ce_consumer_i = register_class_Async_ConsumerInterface(async_ce_channel_state_i, zend_ce_iterator, zend_ce_countable);
	async_ce_channel_i = register_class_Async_ChannelInterface(async_ce_producer_i, async_ce_consumer_i);

	async_ce_channel_exception = register_class_Async_ChannelException(zend_ce_exception);
	async_ce_channel_was_closed_exception = register_class_Async_ChannelWasClosed(async_ce_channel_exception);
	async_ce_channel_is_full_exception = register_class_Async_ChannelIsFull(async_ce_channel_exception);

	async_ce_channel_notifier = register_class_Async_ChannelNotifier(async_ce_notifier);
	async_ce_channel = register_class_Async_Channel(async_ce_channel_i);

	async_ce_channel->default_object_handlers = &async_channel_handlers;
	async_ce_channel->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	async_ce_channel->create_object = async_channel_object_create;

	async_channel_handlers = std_object_handlers;
	async_channel_handlers.offset = XtOffsetOf(async_channel_t, std);
	async_channel_handlers.dtor_obj = async_channel_object_destroy;
	async_channel_handlers.free_obj = async_channel_object_free;
	async_channel_handlers.clone_obj = NULL;
}
