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
#ifndef RESUME_H
#define RESUME_H

typedef enum {
	ASYNC_RESUME_PENDING = 0,
	ASYNC_RESUME_WAITING = 1,
	ASYNC_RESUME_SUCCESS = 2,
	ASYNC_RESUME_ERROR = 3,
} ASYNC_RESUME_STATUS;

typedef struct _async_resume_s async_resume_t;

/**
 * Object structure.
 */
struct _async_resume_s {
	/* PHP object handle. */
	zend_object std;
	zend_fiber *fiber;
	ASYNC_RESUME_STATUS status;
	zval *value;
	zval *error;
	HashTable notifiers;
};

ZEND_API zend_class_entry * async_ce_resume;

void async_register_resume_ce(void);

#endif //RESUME_H
