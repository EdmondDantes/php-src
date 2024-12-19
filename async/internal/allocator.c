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
#include "allocator.h"
#include "php.h"

void *zend_std_alloc(size_t size)
{
	return emalloc(size);
}

void *zend_std_calloc(size_t num, size_t size)
{
	return ecalloc(num, size);
}

void *zend_std_realloc(void *ptr, size_t size)
{
	return erealloc(ptr, size);
}

void zend_std_free(void *ptr)
{
	efree(ptr);
}

allocator_t zend_std_allocator = {
	zend_std_alloc,
	zend_std_calloc,
	zend_std_realloc,
	zend_std_free
};

void *zend_std_palloc(size_t size)
{
	return pemalloc(size, 1);
}

void *zend_std_pcalloc(size_t num, size_t size)
{
	return pecalloc(num, size, 1);
}

void *zend_std_prealloc(void *ptr, size_t size)
{
	return perealloc(ptr, size, 1);
}

void zend_std_pfree(void *ptr)
{
	pefree(ptr, 1);
}

allocator_t zend_std_persistent_allocator = {
	zend_std_palloc,
	zend_std_pcalloc,
	zend_std_prealloc,
	zend_std_pfree
};