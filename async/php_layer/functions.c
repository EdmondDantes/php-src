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
#include "functions.h"
#include "zend_common.h"
#include "zend_smart_str.h"
#include "zend_fibers.h"
#include <ext/standard/info.h>
#include "callback.h"
#include "notifier.h"
#include "functions_arginfo.h"


PHP_MINFO_FUNCTION(async_info) {
	php_info_print_table_start();
	php_info_print_table_header(2, "Module", "True Asynchrony");
	php_info_print_table_row(2, "Version", PHP_ASYNC_VERSION);
	php_info_print_table_row(2, "Support", "Enabled");
	php_info_print_table_end();
}

static zend_module_entry async_module = { /* {{{ */
	STANDARD_MODULE_HEADER,
	"PHP True Asynchrony",
	ext_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_ASYNC_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */


zend_result async_register_module(void) /* {{{ */
{
	zend_module_entry *module;

	EG(current_module) = module = zend_register_module_ex(&async_module, MODULE_PERSISTENT);

	if (UNEXPECTED(module == NULL)) {
		return FAILURE;
	}

	ZEND_ASSERT(module->module_number == 0);

	return SUCCESS;
}
/* }}} */