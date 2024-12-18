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
#ifndef PHP_ASYNC_H
#define PHP_ASYNC_H

#include "php.h"

typedef struct _async_globals_s async_globals_t;

struct _async_globals_s {
	HashTable awaiting;
};

/* Async global */
#ifdef ZTS
# define ASYNC_G(v) ZEND_TSRMG_FAST(async_globals_id, async_globals_t *, v)
#else
# define EG(v) (async_globals.v)
ZEND_API async_globals_t* async_globals;
#endif


#endif //ASYNC_H
