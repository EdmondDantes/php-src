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
#ifndef ASYNC_EXCEPTIONS_H
#define ASYNC_EXCEPTIONS_H

#include <php.h>

BEGIN_EXTERN_C()

ZEND_API zend_class_entry * async_ce_async_exception;
ZEND_API zend_class_entry * async_ce_cancellation_exception;
ZEND_API zend_class_entry * async_ce_input_output_exception;
ZEND_API zend_class_entry * async_ce_timeout_exception;
ZEND_API zend_class_entry * async_ce_poll_exception;

void async_register_exceptions_ce(void);
zend_object * async_new_exception(zend_class_entry *exception_ce, const char *format, ...);
ZEND_API ZEND_COLD zend_object * async_throw_error(const char *format, ...);
ZEND_API ZEND_COLD zend_object * async_throw_cancellation(const char *format, ...);
ZEND_API ZEND_COLD zend_object * async_throw_input_output(const char *format, ...);
ZEND_API ZEND_COLD zend_object * async_throw_timeout(const char *format, const zend_long timeout);
ZEND_API ZEND_COLD zend_object * async_throw_poll(const char *format, ...);

END_EXTERN_C()

#endif //ASYNC_EXCEPTIONS_H
