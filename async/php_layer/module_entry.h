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
#ifndef ASYNC_MODULE_ENTRY_H
#define ASYNC_MODULE_ENTRY_H

#include <async/php_scheduler.h>
#include "zend_common.h"

#define PHP_ASYNC_VERSION "1.0.0-dev"

ZEND_API zend_class_entry *async_ce_walker;

typedef struct _async_walker_s {
	union
	{
		/* PHP std object */
		zend_object std;
		struct
		{
			char _padding[sizeof(zend_object) - sizeof(zval)];
			zval is_finished;
			zval iterator;
			zval custom_data;
			zval defer;
		};
	};

	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	zend_object_iterator * zend_iterator;
	zend_object * run_closure;
	zend_object * next_closure;
	async_microtask_t * next_microtask;
	HashTable * target_hash;
	HashPosition position;
	uint32_t hash_iterator;
} async_walker_t;

zend_result async_register_module(void);

#endif //ASYNC_MODULE_ENTRY_H
