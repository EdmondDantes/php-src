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
#include "../php_layer/reactor_handles.h"

BEGIN_EXTERN_C()

void async_libuv_startup(void);
void async_libuv_shutdown(void);

typedef struct _libuv_poll_s libuv_poll_t;
typedef struct _libuv_timer_s libuv_timer_t;
typedef struct _libuv_signal_s libuv_signal_t;
typedef struct _libuv_process_s libuv_process_t;
typedef struct _libuv_thread_s libuv_thread_t;
typedef struct _libuv_dns_info_s libuv_dns_info_t;
typedef struct _libuv_fs_event_s libuv_fs_event_t;

struct _libuv_poll_s {
	union
	{
		zend_object std;
		reactor_handle_t handle;
		reactor_poll_t poll;
	};
	uv_poll_t *uv_handle;
	bool is_listening;
};

struct _libuv_timer_s {
	union
	{
		zend_object std;
		reactor_handle_t handle;
		reactor_timer_t timer;
	};
	uv_timer_t *uv_handle;
	bool is_listening;
};

struct _libuv_signal_s {
	union
	{
		zend_object std;
		reactor_handle_t handle;
		reactor_signal_t signal;
	};
	uv_signal_t *uv_handle;
	bool is_listening;
};

struct _libuv_process_s {
	union
	{
		zend_object std;
		reactor_handle_t handle;
		reactor_process_t process;
	};
	uv_process_t *uv_handle;
};

struct _libuv_thread_s {
	union
	{
		zend_object std;
		reactor_handle_t handle;
		reactor_thread_t thread;
	};
	uv_thread_t *uv_handle;
};

struct _libuv_dns_info_s {
	union
	{
		zend_object std;
		reactor_handle_t handle;
		reactor_dns_info_t dns_info;
	};
	union
	{
		uv_getaddrinfo_t * addr_info;
		uv_getnameinfo_t * name_info;
	};
	bool is_addr_info;
};

struct _libuv_fs_event_s {
	union
	{
		zend_object std;
		reactor_handle_t handle;
		reactor_file_system_t fs_event;
	};
	uv_fs_event_t *uv_handle;
	bool is_listening;
};

END_EXTERN_C()

#endif //ASYNC_LIBUV_REACTOR_H
