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
#ifndef ASYNC_API_H
#define ASYNC_API_H

#include <stdbool.h>

#include "zend_async_API.h"

typedef struct
{
	zend_coroutine_event_callback_t callback;
	unsigned int total;
	unsigned int waiting_count;
	unsigned int resolved_count;
	bool ignore_errors;
} async_await_callback_t;

void async_api_register(void);

#endif //ASYNC_API_H
