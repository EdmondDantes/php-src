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
	ASYNC_RESUME_WAITING = 1,
	ASYNC_RESUME_SUCCESS = 2,
	ASYNC_RESUME_ERROR = 3,
	ASYNC_RESUME_FINISHED = 4,
	ASYNC_RESUME_IGNORED = 5
} ASYNC_RESUME_STATUS;

typedef struct _async_resume_s async_resume_t;
typedef void (*async_resume_when_callback_t)(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error);

typedef struct _async_resume_notifier_s async_resume_notifier_t;

/**
 * async_resume_t structure.
 *
 * @note Note: The structure of this object is BASED on the knowledge that the result property
 * is located in the property[1] slot!
 * If you change the number of properties in the Resume object, you must update this structure!
 */
struct _async_resume_s {
	union
	{
		/* PHP object handle. */
		zend_object std;
		struct {
			char _padding[sizeof(zend_object) - sizeof(zval)];
			zval result;
		};
	};
	zend_fiber *fiber;
	ASYNC_RESUME_STATUS status;
	zend_object *error;
	/* A list of notifier objects (reactor_notifier_t) that occurred during the last iteration of the event loop. */
	HashTable * triggered_notifiers;
	/* The array of async_resume_notifier_t. */
	HashTable notifiers;
	/* Filename of the resume object creation point. */
	zend_string *filename;
	/* Line number of the resume object creation point. */
	uint32_t lineno;
};

/**
 * The structure is used to store the callback and the notifier object.
 * When the Notifier sends an event notification,
 * The Resume finds the corresponding entry in the list and calls the specified callback to handle the event.
 */
struct _async_resume_notifier_s {
	reactor_notifier_t *notifier;
	/**
	 * The callback can be of two types:
	 * * An internal Zend Engine callback, a pointer to a C function of type `async_resume_when_callback_t` (IS_PTR)
	 * * A `PHP` user-mode callback. (CALLABLE)
	 */
	zval callback;
};

static zend_always_inline zval* async_resume_get_callback(zend_object* resume)
{
	return &resume->properties_table[0];
}

BEGIN_EXTERN_C()

ZEND_API zend_class_entry * async_ce_resume;

void async_register_resume_ce(void);
ZEND_API async_resume_t * async_resume_new(zend_fiber * fiber);
ZEND_API async_resume_t * async_resume_new_ex(zend_fiber * fiber, const size_t size);
ZEND_API void async_resume_when(
		async_resume_t				*resume,
		reactor_notifier_t			*notifier,
		const bool					trans_notifier,
		const async_resume_when_callback_t callback
	);
ZEND_API void async_resume_remove_notifier(async_resume_t *resume, reactor_notifier_t *notifier);
ZEND_API void async_resume_when_callback_resolve(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error);
ZEND_API void async_resume_when_callback_cancel(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error);
ZEND_API void async_resume_when_callback_timeout(async_resume_t *resume, reactor_notifier_t *notifier, zval* event, zval* error);
ZEND_API void async_resume_notify(async_resume_t* resume, reactor_notifier_t* notifier, zval* event, zval* error);
ZEND_API void async_resume_waiting(async_resume_t *resume);
ZEND_API int async_resume_get_ready_poll_handles(const async_resume_t *resume);

END_EXTERN_C()

#endif //ASYNC_RESUME_H
