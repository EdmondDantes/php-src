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
#ifndef FUTURE_H
#define FUTURE_H

#include "php.h"
#include "notifier.h"

BEGIN_EXTERN_C()

ZEND_API zend_class_entry *async_ce_future_state;
ZEND_API zend_class_entry *async_ce_future;

typedef struct _async_future_state_s
{
	reactor_notifier_t notifier;
	zval result;
	zend_object * throwable;
	bool is_handled;
	/* Filename of the future object creation point. */
	zend_string *filename;
	/* Line number of the future object creation point. */
	uint32_t lineno;
	/* Filename of the future object completion point. */
	zend_string *completed_filename;
	/* Line number of the future object completion point. */
	uint32_t completed_lineno;

} async_future_state_t;

typedef struct _async_future_s
{
	zend_object std;
	struct
	{
		char _padding[sizeof(zend_object) - sizeof(zval)];
		/**
		 * PHP object Async\FutureState
		 */
		zval future_state;
	};

} async_future_t;

void async_register_future_ce(void);

END_EXTERN_C()

#endif //FUTURE_H
