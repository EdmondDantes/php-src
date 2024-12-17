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

#ifndef ZEND_COMMON_H
#define ZEND_COMMON_H

#include "php.h"
#include "zend_exceptions.h"
#include "zend_smart_str.h"
#include "zend_interfaces.h"

#define IF_THROW_RETURN_VOID if(EG(exception) != NULL) { return; }
#define IF_THROW_RETURN(value) if(EG(exception) != NULL) { return value; }

zval* async_new_weak_reference_from(const zval* referent);
void async_resolve_weak_reference(zval* weak_reference, zval* retval);

#endif //ZEND_COMMON_H
