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
#ifndef ASYNC_LIBUV_REACTOR_H
#define ASYNC_LIBUV_REACTOR_H

#include "php.h"
#include "zend_types.h"
#include <libuv/uv.h>
#include "../php_async.h"
#include "../php_layer/notifier.h"
#include "../php_layer/ev_handles.h"

BEGIN_EXTERN_C()

typedef struct _libuv_handle_s libuv_handle_t;
typedef struct _libuv_poll_s libuv_poll_t;
typedef struct _libuv_timer_s libuv_timer_t;
typedef struct _libuv_signal_s libuv_signal_t;
typedef struct _libuv_process_s libuv_process_t;
typedef struct _libuv_thread_s libuv_thread_t;
typedef struct _libuv_fs_event_s libuv_fs_event_t;

struct _libuv_handle_s {
	reactor_handle_t handle;
	uv_handle_t *uv_handle;
};

struct _libuv_poll_s {
	reactor_handle_t handle;
	uv_poll_t *uv_handle;
};

struct _libuv_timer_s {
	reactor_handle_t handle;
	uv_timer_t *uv_handle;
};

struct _libuv_signal_s {
	reactor_handle_t handle;
	uv_signal_t *uv_handle;
};

struct _libuv_process_s {
	reactor_handle_t handle;
	uv_process_t *uv_handle;
};

struct _libuv_thread_s {
	reactor_handle_t handle;
	uv_thread_t *uv_handle;
};

struct _libuv_fs_event_s {
	reactor_handle_t handle;
	uv_fs_event_t *uv_handle;
};

END_EXTERN_C()

#endif //ASYNC_LIBUV_REACTOR_H
