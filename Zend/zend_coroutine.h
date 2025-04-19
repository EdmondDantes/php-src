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
#ifndef ZEND_COROUTINE_H
#define ZEND_COROUTINE_H

#include "zend_fibers.h"

typedef struct _zend_coroutine_waker zend_coroutine_waker;
typedef struct _zend_coroutine zend_coroutine;

typedef zend_array* (*zend_coroutine_get_awaiting_info_t)(zend_coroutine * coroutine);

struct _zend_coroutine {
	/* Flags are defined in enum zend_fiber_flag. */
	uint8_t flags;

	/* Native C fiber context. */
	zend_fiber_context context;

	/* Callback and info / cache to be used when fiber is started. */
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;

	/* Current Zend VM execute data being run by the fiber. */
	zend_execute_data *execute_data;

	/* Active fiber vm stack. */
	zend_vm_stack vm_stack;

	/* Coroutine waker */
	zend_coroutine_waker *waker;

	/* Storage for fiber return value. */
	zval result;

	/* PHP object handle. */
	zend_object std;
};

struct _zend_coroutine_waker {
	zend_coroutine_get_awaiting_info_t get_awaiting_info;
};

BEGIN_EXTERN_C()

ZEND_API zend_coroutine *zend_coroutine_create(zend_fcall_info *fci, zend_fcall_info_cache *fci_cache);

END_EXTERN_C()

#endif //ZEND_COROUTINE_H
