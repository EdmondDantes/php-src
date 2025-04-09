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
#ifndef ZEND_ASYNC_API_H
#define ZEND_ASYNC_API_H

#include "zend_coroutine.h"

typedef struct _zend_async_api zend_async_api;

typedef void (*zend_async_spawn_t)();
typedef void (*zend_async_suspend_t)(zend_coroutine *coroutine);

struct _zend_async_api {
	zend_async_spawn_t spawn;
	zend_async_suspend_t suspend;
};

typedef struct _zend_async_handle zend_async_handle;

struct _zend_async_handle {
};

#endif //ZEND_ASYNC_API_H
