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
#ifndef LIBUV_EVENT_LOOP_H
#define LIBUV_EVENT_LOOP_H

#include "php.h"
#include "zend_types.h"
#include "uv.h"
#include "../async.h"
#include "../php_layer/notifier.h"
#include "../php_layer/ev_handles.h"

typedef struct _libuv_handle_s libuv_handle_t;
typedef struct _libuv_poll_s libuv_poll_t;
typedef struct _libuv_timer_s libuv_timer_t;

struct _libuv_handle_s {
	async_ev_handle_t handle;
	uv_handle_t uv_handle;
};

struct _libuv_poll_s {
	async_ev_handle_t handle;
	uv_poll_t uv_handle;
};

struct _libuv_timer_s {
	async_ev_handle_t handle;
	uv_timer_t uv_handle;
};

#endif //LIBUV_EVENT_LOOP_H
