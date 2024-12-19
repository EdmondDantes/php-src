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
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "php.h"
#include "allocator.h"

typedef struct _circular_buffer_s circular_buffer_t;

struct _circular_buffer_s {
    size_t item_size;
    size_t min_size;
	void *start;
	void *end;
	void *head;
	void *tail;
	allocator_t *allocator;
};

zend_always_inline zend_bool circular_buffer_is_full(circular_buffer_t *buffer);
zend_always_inline zend_bool circular_buffer_is_empty(circular_buffer_t *buffer);
zend_result circular_buffer_push(circular_buffer_t *buffer, void *value);
zend_result circular_buffer_pop(circular_buffer_t *buffer, void *value);
zend_always_inline size_t circular_buffer_count(circular_buffer_t *buffer);
void circular_buffer_rellocate(circular_buffer_t *buffer, size_t new_count);

#endif //CIRCULAR_BUFFER_H
