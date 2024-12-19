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
#include "circular_buffer.h"

/**
 * Initialize a new zval circular buffer.
 *
 */
circular_buffer_t *circular_buffer_new(size_t count, size_t item_size, allocator_t *allocator)
{
  	if(item_size <= 0) {
		zend_error(E_ERROR, "Item size must be greater than zero");
		return NULL;
	}

	if(allocator == NULL) {
		allocator = &zend_std_allocator;
	}

    if(count <= 0) {
		count = 4;
	}

	circular_buffer_t *buffer;

	buffer = (allocator->m_calloc)(1, sizeof(circular_buffer_t));

    buffer->allocator = allocator;
	buffer->item_size = item_size;
	buffer->min_size = count;
	buffer->start = (allocator->m_calloc)(count, item_size);
	buffer->end = buffer->start + (count - 1) * item_size;
	buffer->head = NULL;
	buffer->tail = NULL;

	return buffer;
}

/**
 * Free the memory associated with a zval circular buffer.
 */
void circular_buffer_destroy(circular_buffer_t *buffer)
{
	(buffer->allocator->m_free)(buffer->start);
	(buffer->allocator->m_free)(buffer);
}

/**
 * The method will return TRUE if the buffer actually uses half the memory allocated.
 * This means the memory can be released.
 */
zend_always_inline bool circular_buffer_should_be_rellocate(circular_buffer_t *buffer)
{
	if(buffer->head == NULL || buffer->tail == NULL) {
		return 0;
	}

	return llabs(buffer->head - buffer->tail) < (((buffer->end - buffer->start) + buffer->item_size) / 2);
}

/**
 * Reallocate the memory associated with a zval circular buffer.
 */
void circular_buffer_rellocate(circular_buffer_t *buffer, size_t new_count)
{
  	size_t current_count = ((buffer->end - buffer->start) / buffer->item_size) + 1;
    bool increase = true;

  	if(new_count <= 0) {

		if(circular_buffer_is_full(buffer)) {
			new_count = current_count * 2;
			increase = true;
		} else if(circular_buffer_count(buffer) < (current_count / 2)) {
			new_count = current_count / 2;

            if(new_count < buffer->min_size) {
				new_count = buffer->min_size;
			}

			increase = false;
		} else {
            // No need to reallocate
			return;
		}
	}

    if(increase) {
    	void *new_end = buffer->start + (new_count - 1) * buffer->item_size;

    	if(buffer->head > new_end || buffer->tail > new_end) {
    		zend_error(E_WARNING, "Cannot reallocate circular buffer, head or tail is out of bounds");
    		return;
    	}

        size_t head_offset = buffer->head - buffer->start;
        size_t tail_offset = buffer->tail != NULL ? buffer->tail - buffer->start : 0;

    	buffer->start = (buffer->allocator->m_realloc)(buffer->start, new_count * buffer->item_size);
        buffer->head = buffer->start + head_offset;
        buffer->tail = buffer->start + tail_offset;
    	buffer->end = new_end;

        return;
	}

    // If buffer is empty we can simply free the memory and allocate a new buffer.
	if(buffer->head == NULL || buffer->head == buffer->tail) {
		(buffer->allocator->m_free)(buffer->start);
		buffer->start = (buffer->allocator->m_alloc)(new_count * buffer->item_size);
        buffer->head = NULL;
        buffer->tail = NULL;
		buffer->end = buffer->start + (new_count - 1) * buffer->item_size;
		return;
	}

	//
	// Shrinking the size of a circular buffer is a more complex operation that involves three steps:
	// * A new buffer of the required size is allocated.
	// * Data is copied starting from the head/tail.
	// * The old buffer is destroyed.
    //


    void *tail = buffer->tail != NULL ? buffer->tail : buffer->start;
    void *start = buffer->head > tail ? buffer->tail : buffer->head;
    void *end = buffer->head > tail ? buffer->head : buffer->tail;
    size_t size = end - start;

    // allocate a new buffer
    zval *new_start = (buffer->allocator->m_alloc)(new_count * buffer->item_size);
    zval *new_end = new_start + (new_count - 1) * buffer->item_size;

    // copy data
    memcpy(new_start, start, size);

	size_t head_offset = buffer->head - buffer->start;
	size_t tail_offset = buffer->tail != NULL ? buffer->tail - buffer->start : 0;

    // Free the old buffer
    buffer->allocator->m_free(buffer->start);

    buffer->start = new_start;
	buffer->head = buffer->start + head_offset;

    if(buffer->tail != NULL) {
		buffer->tail = buffer->start + tail_offset;
	}
}

/**
 * Move the header to the next position.
 */
static zend_always_inline zend_result circular_buffer_header_next(circular_buffer_t *buffer)
{
  	if(buffer->head == NULL) {
		buffer->head = buffer->start;
	} else if(buffer->head >= buffer->end) {

        if(buffer->tail != NULL) {
			buffer->head = buffer->start;
		} else {
			return FAILURE;
		}

	} else if((buffer->head + buffer->item_size) >= buffer->tail) {
		buffer->head += buffer->item_size;
	}

	return SUCCESS;
}

/**
 * Move the tail to the next position.
 */
static zend_always_inline zend_result circular_buffer_tail_next(circular_buffer_t *buffer)
{
	if(UNEXPECTED(buffer->tail == buffer->head)) {
		return FAILURE;
	}

	if(buffer->tail == NULL) {
		buffer->tail = buffer->start;
	} else if(buffer->tail >= buffer->end) {
		buffer->tail = buffer->start;
	} else if((buffer->tail + buffer->item_size) <= buffer->head) {
		buffer->tail += buffer->item_size;
	} else {
		return FAILURE;
	}

	return SUCCESS;
}


/**
 * Push a new zval into the circular buffer.
 */
zend_result circular_buffer_push(circular_buffer_t *buffer, void *value)
{
  	if(circular_buffer_is_full(buffer)) {
		circular_buffer_rellocate(buffer, 0);
	}

	if(circular_buffer_header_next(buffer) == FAILURE) {
		zend_error(E_WARNING, "Cannot push into full circular buffer");
		return FAILURE;
	}

	memcpy(buffer->head, value, buffer->item_size);
	return SUCCESS;
}

/**
 * Pop a zval from the circular buffer.
 */
zend_result circular_buffer_pop(circular_buffer_t *buffer, void *value)
{
	if(circular_buffer_tail_next(buffer) == FAILURE) {
		zend_error(E_WARNING, "Cannot pop from empty circular buffer");
		return FAILURE;
	}

	memcpy(value, buffer->tail, buffer->item_size);

	if(circular_buffer_should_be_rellocate(buffer)) {
		circular_buffer_rellocate(buffer, 0);
	}

	return SUCCESS;
}

/**
 * Check if the circular buffer is empty.
 */
zend_always_inline zend_bool circular_buffer_is_empty(circular_buffer_t *buffer)
{
  	return buffer->head == buffer->tail;
}

/**
 * Check if the circular buffer is full.
 */
zend_always_inline zend_bool circular_buffer_is_full(circular_buffer_t *buffer)
{
	return buffer->head != NULL
         	&& ((buffer->tail == NULL && buffer->head == buffer->end)
             || (llabs(buffer->head - buffer->tail) == (buffer->end - buffer->start)));
}

zend_always_inline size_t circular_buffer_count(circular_buffer_t *buffer)
{
  	if(buffer->head == NULL) {
		return 0;
	}

    void *tail = buffer->tail != NULL ? buffer->tail : buffer->start;

    return (llabs(buffer->head - tail) / buffer->item_size) + 1;
}

//
// Functions for ZVAL circular buffer
//
zend_always_inline circular_buffer_t *zval_circular_buffer_new(size_t count, allocator_t *allocator)
{
	return circular_buffer_new(count, sizeof(zval), allocator);
}

/**
 * Push a new zval into the circular buffer.
 * The zval will be copied and its reference count will be increased.
 */
zend_always_inline zend_result zval_circular_buffer_push(circular_buffer_t *buffer, zval *value)
{
	Z_TRY_ADDREF_P(value);
	return circular_buffer_push(buffer, value);
}

/**
 * Pop a zval from the circular buffer.
 * The zval will be copied and its reference count will not be changed because your code will get the ownership.
 */
zend_always_inline zend_result zval_circular_buffer_pop(circular_buffer_t *buffer, zval *value)
{
  	ZVAL_UNDEF(value);
	return circular_buffer_pop(buffer, value);
}