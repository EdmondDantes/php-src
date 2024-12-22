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
#ifndef HANDLES_H
#define HANDLES_H

#include "php.h"
#include "zend_types.h"
#include "notifier.h"

async_handle_t* async_resource_to_handle(zend_resource *resource, zend_ulong actions);
async_handle_t* async_timeout_new(zend_ulong timeout);
async_handle_t* async_signal_new(zend_long sig_number);

#endif //HANDLES_H
