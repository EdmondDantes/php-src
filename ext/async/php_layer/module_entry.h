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

#include <ext/async/php_scheduler.h>
#include "zend_common.h"

#define PHP_ASYNC_VERSION "1.0.0-dev"

zend_module_entry async_module_entry;

zend_result async_register_module(void);

#endif //ASYNC_MODULE_ENTRY_H
