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

#include "zend_common.h"

BEGIN_EXTERN_C()

typedef struct _async_context_s async_context_t;

struct _async_context_s
{
	union
	{
		/* PHP std object Async\Context */
		zend_object std;
		struct
		{
			char _padding[sizeof(zend_object) - sizeof(zval)];
			HashTable *map;
			HashTable *objects;
			union
			{
				async_context_t * parent;
				zend_object * parent_weak_ref;
			};
			bool is_weak_ref;
		};
	};
};

typedef struct
{
	union
	{
		/* PHP std object Async\Key */
		zend_object std;
		struct
		{
			char _padding[sizeof(zend_object) - sizeof(zval)];
			zval description;
		};
	};
} async_key_t;

ZEND_API zend_class_entry * async_ce_context;
ZEND_API zend_class_entry * async_ce_key;
ZEND_API zend_class_entry * async_ce_context_exception;

void async_register_context_ce(void);

ZEND_API async_context_t * async_context_new(async_context_t * parent, const bool is_weak_ref);
ZEND_API zval * async_context_find_by_key(async_context_t * context, zend_object * key, bool local, int recursion);
ZEND_API zval * async_context_find_by_str(async_context_t * context, zend_string * key, bool local, int recursion);

ZEND_API void async_context_set_key(async_context_t * context, zend_object * key, zval * value, bool replace);
ZEND_API void async_context_set_str(async_context_t * context, zend_string * key, zval * value, bool replace);
ZEND_API void async_context_del_key(const async_context_t * context, zval * key);

ZEND_API zend_object* async_context_clone(zend_object * object);

ZEND_API async_context_t * async_context_current(bool auto_create, bool add_ref);
ZEND_API async_context_t * async_context_current_new(bool is_override, const bool is_weak_ref);

ZEND_API async_key_t * async_key_new(zend_string * description);
ZEND_API async_key_t * async_key_new_from_string(char * description, size_t len);

END_EXTERN_C()

#endif //CONTEXT_H
