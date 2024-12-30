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
#ifndef ASYNC_RESUME_H
#define ASYNC_RESUME_H

#include "notifier.h"

typedef enum {
	ASYNC_RESUME_NO_STATUS = 0,
	ASYNC_RESUME_PENDING = 1,
	ASYNC_RESUME_SUCCESS = 2,
	ASYNC_RESUME_ERROR = 3,
} ASYNC_RESUME_STATUS;

typedef struct _async_resume_s async_resume_t;
typedef void (*async_resume_when_callback)(async_resume_t *resume, reactor_notifier_t *notifier);

/**
 * Object structure.
 */
struct _async_resume_s {
	/* PHP object handle. */
	zend_object std;
	zend_fiber *fiber;
	ASYNC_RESUME_STATUS status;
	zval *result;
	zend_object *error;
	HashTable * triggered_notifiers;
	HashTable notifiers;
};

struct _async_callback_notifier_s {
	reactor_notifier_t *notifier;
	zval callback;
};

static zend_always_inline zval* async_resume_get_callback(zend_object* resume)
{
	return &resume->properties_table[0];
}

ZEND_API zend_class_entry * async_ce_resume;

void async_register_resume_ce(void);
ZEND_API async_resume_t * async_resume_new();
void async_resume_destroy(async_resume_t *resume);
void async_resume_when(async_resume_t *resume, reactor_notifier_t *notifier, async_resume_when_callback *callback);
void async_resume_when_callback_resolve(async_resume_t *resume, reactor_notifier_t *notifier);
void async_resume_when_callback_cancel(async_resume_t *resume, reactor_notifier_t *notifier);
void async_resume_notify(async_resume_t* resume, reactor_notifier_t* notifier, const zval* event, const zval* error);

#endif //ASYNC_RESUME_H
