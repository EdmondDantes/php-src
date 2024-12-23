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
#include "exceptions.h"
#include <zend_exceptions.h>

#include "exceptions_arginfo.h"

void async_register_exceptions_ce(void)
{
	zend_class_entry* exception_ce = zend_exception_get_default();

	async_ce_cancellation_exception = register_class_Async_CancellationException(exception_ce);
	async_ce_input_output_exception = register_class_Async_InputOutputException(exception_ce);
	async_ce_timeout_exception = register_class_Async_TimeoutException(exception_ce);
	async_ce_poll_exception = register_class_Async_PollException(exception_ce);
}

ZEND_API ZEND_COLD zend_object * async_throw_cancellation(char *format, ...)
{
	const zend_object *previous_exception = EG(exception);

	if (format == NULL
		&& previous_exception != NULL
		&& instanceof_function(previous_exception->ce, async_ce_cancellation_exception)) {
		format = "The operation was canceled by timeout";
	} else {
		format = format ? format : "The operation was canceled";
	}

	va_list args;
	va_start(args, format);

	zend_object *obj = zend_throw_exception_ex(async_ce_cancellation_exception, 0, format, args);

	va_end(args);
	return obj;
}

ZEND_API ZEND_COLD zend_object * async_throw_input_output(char *format, ...)
{
	format = format ? format : "An input/output error occurred.";

	va_list args;
	va_start(args, format);

	zend_object *obj = zend_throw_exception_ex(async_ce_input_output_exception, 0, format, args);

	va_end(args);
	return obj;
}

ZEND_API ZEND_COLD zend_object * async_throw_timeout(const char *format, const zend_long timeout)
{
	format = format ? format : "A timeout of %u microseconds occurred";

	return zend_throw_exception_ex(async_ce_timeout_exception, 0, format, timeout);
}

ZEND_API ZEND_COLD zend_object * async_throw_poll(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	zend_object *obj = zend_throw_exception_ex(async_ce_poll_exception, 0, format, args);

	va_end(args);
	return obj;
}