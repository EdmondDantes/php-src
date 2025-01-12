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
#ifndef ASYNC_CHANNEL_H
#define ASYNC_CHANNEL_H

#include "notifier.h"
#include "php.h"
#include "zend_common.h"
#include "async/internal/circular_buffer.h"

typedef enum {
	ASYNC_CHANNEL_DIRECTION_SEND = 1,
	ASYNC_CHANNEL_DIRECTION_RECEIVE = 2,
	ASYNC_CHANNEL_DIRECTION_BOTH = 3
} ASYNC_CHANNEL_DIRECTION;

typedef struct _async_channel_s async_channel_t;

struct _async_channel_s {
	circular_buffer_t buffer;
	bool expandable;
	int direction;
	reactor_notifier_t *notifier;
	zend_object std;
};

zend_always_inline zval* async_channel_get_owner(zend_object* channel)
{
	return &channel->properties_table[0];
}

zend_always_inline zend_fiber* async_channel_get_owner_fiber(zend_object* channel)
{
	if (Z_TYPE(channel->properties_table[0]) != IS_OBJECT) {
		return NULL;
	}

	return (zend_fiber*) &Z_OBJ(channel->properties_table[0]);
}

zend_always_inline void async_channel_set_owner_fiber(zend_object* channel, zend_fiber * fiber)
{
	zval z_fiber;
	ZVAL_OBJ(&z_fiber, fiber);
	zval_property_copy(&channel->properties_table[0], &z_fiber);
}

ZEND_API zend_class_entry * async_ce_channel;
ZEND_API zend_class_entry * async_ce_channel_exception;
ZEND_API zend_class_entry * async_ce_channel_was_closed_exception;
ZEND_API zend_class_entry * async_ce_channel_is_full_exception;

void async_register_channel_ce(void);

#endif //ASYNC_CHANNEL_H
