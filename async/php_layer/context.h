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
#ifndef CONTEXT_H
#define CONTEXT_H

#include "php.h"
#include "zend_common.h"

BEGIN_EXTERN_C()

typedef struct
{
	union
	{
		/* PHP std object Async\Context */
		zend_object std;
		struct
		{
			char _padding[sizeof(zend_object) - sizeof(zval)];
			HashTable map;
			HashTable objects;
		};
	};

} async_context_t;

ZEND_API zend_class_entry * async_ce_context;

void async_register_context_ce(void);

ZEND_API async_context_t * async_context_new(async_context_t * parent);
ZEND_API zval * async_context_find_by_key(async_context_t * context, zend_object * key);
ZEND_API zval * async_context_find_by_str(async_context_t * context, zend_string * key);

ZEND_API async_context_t * async_context_with_key(async_context_t * context, zend_object * key, zval * value);
ZEND_API async_context_t * async_context_with_str(async_context_t * context, zend_string * key, zval * value);

END_EXTERN_C()

#endif //CONTEXT_H
