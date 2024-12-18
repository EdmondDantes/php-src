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
#include "php.h"
#include "async.h"
#include "php_layer/functions.h"

#ifdef ZTS
ZEND_API int async_globals_id;
ZEND_API size_t async_globals_offset;
TSRMLS_MAIN_CACHE_DEFINE()
#else
ZEND_API async_globals_t* async_globals;
#endif

/**
 * Async globals constructor.
 */
static void async_globals_ctor(async_globals_t *async_globals)
{

}

/**
 * Async globals destructor.
 */
static void async_globals_dtor(async_globals_t *async_globals)
{

}

/**
 * Async startup function.
 */
void async_startup(void)
{
	if (async_register_module() == FAILURE) {
		zend_error(E_CORE_WARNING, "Failed to register the 'True Asynchrony' module.");
		return;
	}

#ifdef ZTS

	ts_allocate_fast_id(
		&async_globals_id,
		&async_globals_offset,
		sizeof(async_globals_t),
		(ts_allocate_ctor) async_globals_ctor,
		(ts_allocate_dtor) async_globals_dtor
	);

	async_globals_t *async_globals = ts_resource(async_globals_id);
	async_globals_ctor(async_globals);
#else
	async_globals_ctor(async_globals);
#endif
}

/**
 * Async shutdown function.
 */
void async_shutdown(void)
{
#ifdef ZTS
	ts_free_id(async_globals_id);
#else
	async_globals_dtor(async_globals);
#endif
}