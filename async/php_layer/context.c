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
#include "context.h"
#include "context_arginfo.h"

#define METHOD(name) PHP_METHOD(Async_Context, name)
#define THIS_CONTEXT ((async_context_t *)(Z_OBJ_P(ZEND_THIS)))

METHOD(__construct)
{
}

METHOD(find)
{

}

METHOD(get)
{

}

METHOD(has)
{

}

METHOD(findLocal)
{

}

METHOD(getLocal)
{

}

METHOD(hasLocal)
{

}

METHOD(withKey)
{

}

METHOD(withoutKey)
{

}

METHOD(getParent)
{

}

METHOD(isEmpty)
{

}

void async_register_context_ce(void)
{
	async_ce_context = register_class_Async_Context();
}