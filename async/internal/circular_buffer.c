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

#define MINIMUM_COUNT 4

#ifdef ASYNC_UNIT_TESTS
#include <stdio.h>
#endif

#ifdef ASYNC_UNIT_TESTS
#define ASYNC_ERROR(type, format) fprintf(stderr, format)
#else
#define ASYNC_ERROR(type, format) zend_error(type, format)
#endif

/**
 * Initialize a new zval circular buffer.
 *
 */
circular_buffer_t *circular_buffer_new(size_t count, const size_t item_size, const allocator_t *allocator)
{
  	if(item_size <= 0) {
		ASYNC_ERROR(E_ERROR, "Item size must be greater than zero");
		return NULL;
	}

	if(allocator == NULL) {
		allocator = &zend_std_allocator;
	}

    if(count <= 0) {
		count = MINIMUM_COUNT;
	}

    circular_buffer_t* buffer = (allocator->m_calloc)(1, sizeof(circular_buffer_t));

	if (UNEXPECTED(buffer == NULL)) {
		ASYNC_ERROR(E_ERROR, "Failed to allocate memory for circular buffer");
		return NULL;
	}

	if (UNEXPECTED(circular_buffer_ctor(buffer, count, item_size, allocator) == FAILURE)) {
		(allocator->m_free)(buffer);
		return NULL;
	}

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

zend_result circular_buffer_ctor(circular_buffer_t * buffer, size_t count, const size_t item_size, const allocator_t *allocator)
{
	if(allocator == NULL) {
		allocator = &zend_std_allocator;
	}

	if(count <= 0) {
		count = MINIMUM_COUNT;
	}

	void *start = (allocator->m_calloc)(count, item_size);

	if (UNEXPECTED(start == NULL)) {
		ASYNC_ERROR(E_ERROR, "Failed to allocate memory for circular buffer");
		return FAILURE;
	}

	buffer->allocator	= allocator;
	buffer->item_size	= item_size;
	buffer->min_size	= count;
	buffer->start		= start;
	buffer->end			= (char *) buffer->start + (count - 1) * item_size;
	buffer->decrease_t	= 0;
	buffer->head		= NULL;
	buffer->tail		= NULL;

	return SUCCESS;
}

zend_always_inline void circular_buffer_dtor(const circular_buffer_t *buffer)
{
	(buffer->allocator->m_free)(buffer->start);
}

/**
 * The method will return TRUE if the buffer actually uses half the memory allocated.
 * This means the memory can be released.
 */
static zend_always_inline bool circular_buffer_should_be_decrease(const circular_buffer_t *buffer)
{
	if(buffer->head == NULL || buffer->tail == NULL) {
		return 0;
	}

	ptrdiff_t capacity = (char *)buffer->head - (char *)buffer->tail;

	if (capacity < 0) {
		capacity = -capacity;
	}

	return (size_t)(capacity) < buffer->decrease_t;
}

/**
 * Reallocate the memory associated with a zval circular buffer.
 */
zend_result circular_buffer_realloc(circular_buffer_t *buffer, size_t new_count)
{
	const size_t current_count = (((char *)buffer->end - (char *)buffer->start) / buffer->item_size) + 1;
    bool increase = true;

  	if(new_count <= 0) {

		if(circular_buffer_is_full(buffer)) {
			new_count = current_count * 2;
			increase = true;
		} else if(circular_buffer_should_be_decrease(buffer)) {
			new_count = current_count / 2;

            if(new_count < buffer->min_size) {
				new_count = buffer->min_size;
			}

			increase = false;
		} else {
            // No need to reallocate
			return SUCCESS;
		}
	}

    if(increase) {
    	const void *new_end = (char *)buffer->start + (new_count - 1) * buffer->item_size;

    	if(buffer->head > new_end || buffer->tail > new_end) {
    		ASYNC_ERROR(E_WARNING, "Cannot reallocate circular buffer, head or tail is out of bounds");
    		return FAILURE;
    	}

    	const ptrdiff_t head_offset = (char *)buffer->head - (char *)buffer->start;
    	const ptrdiff_t tail_offset = buffer->tail != NULL ? (char *)buffer->tail - (char *)buffer->start : 0;

    	void *new_start = (buffer->allocator->m_realloc)(buffer->start, new_count * buffer->item_size);

		if(new_start == NULL) {
			ASYNC_ERROR(E_WARNING, "Failed to reallocate circular buffer");
			return FAILURE;
		}

    	buffer->start	= new_start;
        buffer->head	= (char *) buffer->start + head_offset;
        buffer->tail	= buffer->tail != NULL ? (char *) buffer->start + tail_offset : NULL;
    	buffer->end		= (char *) new_start + (new_count - 1) * buffer->item_size;
    	buffer->decrease_t = new_count / 2 - new_count / 4;

        return SUCCESS;
	}

    // If buffer is empty we can simply free the memory and allocate a new buffer.
	if(buffer->head == NULL || buffer->head == buffer->tail) {
		(buffer->allocator->m_free)(buffer->start);
		buffer->start = (buffer->allocator->m_alloc)(new_count * buffer->item_size);

		if (buffer->start == NULL) {
			ASYNC_ERROR(E_WARNING, "Failed to reallocate circular buffer");
			return FAILURE;
		}

        buffer->head = NULL;
        buffer->tail = NULL;
		buffer->end = (char*) buffer->start + (new_count - 1) * buffer->item_size;
		buffer->decrease_t = new_count / 2 - new_count / 4;
		return SUCCESS;
	}

	//
	// Shrinking the size of a circular buffer is a more complex operation that involves three steps:
	// * A new buffer of the required size is allocated.
	// * Data is copied starting from the head/tail.
	// * The old buffer is destroyed.
    //

    const void *tail	= buffer->tail != NULL ? buffer->tail : buffer->start;
    const void *start	= buffer->head > tail ? buffer->tail : buffer->head;
    const void *end		= buffer->head > tail ? buffer->head : buffer->tail;
    const size_t size	= (char *)end - (char *)start;

    // allocate a new buffer
    void *new_start		= (buffer->allocator->m_alloc)(new_count * buffer->item_size);
    void *new_end		= (char *) new_start + (new_count - 1) * buffer->item_size;

    // copy data
    memcpy(new_start, start, size);

	const ptrdiff_t head_offset = (char *)buffer->head - (char *)buffer->start;
	const ptrdiff_t tail_offset = buffer->tail != NULL ? (char *)buffer->tail - (char *)buffer->start : 0;

    // Free the old buffer
    buffer->allocator->m_free(buffer->start);

    buffer->start		= new_start;
	buffer->head		= (char *) buffer->start + head_offset;
	buffer->end			= new_end;
	buffer->decrease_t	= new_count / 2 - new_count / 4;

    if(buffer->tail != NULL) {
		buffer->tail	= (char *)buffer->start + tail_offset;
	}

	return SUCCESS;
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

	} else if(((char *) buffer->head + buffer->item_size) >= (char *) buffer->tail) {
		buffer->head = (char *)buffer->head + buffer->item_size;
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
	} else if(((char *)buffer->tail + buffer->item_size) <= (char *) buffer->head) {
		buffer->tail = (char *) buffer->tail + buffer->item_size;
	} else {
		return FAILURE;
	}

	return SUCCESS;
}


/**
 * Push a new zval into the circular buffer.
 */
zend_result circular_buffer_push(circular_buffer_t *buffer, const void *value)
{
	const bool should_reallocate = circular_buffer_is_full(buffer);

  	if(should_reallocate && circular_buffer_realloc(buffer, 0) == FAILURE) {
		return FAILURE;
	}

	if(circular_buffer_header_next(buffer) == FAILURE) {
		ASYNC_ERROR(E_WARNING, "Cannot push into full circular buffer");
		return FAILURE;
	}

	memcpy(buffer->head, value, buffer->item_size);

	if(!should_reallocate && circular_buffer_should_be_decrease(buffer) && circular_buffer_realloc(buffer, 0) == FAILURE) {
		return SUCCESS;
	}

	return SUCCESS;
}

/**
 * Pop a zval from the circular buffer.
 */
zend_result circular_buffer_pop(circular_buffer_t *buffer, void *value)
{
	if(circular_buffer_tail_next(buffer) == FAILURE) {
		ASYNC_ERROR(E_WARNING, "Cannot pop from empty circular buffer");
		return FAILURE;
	}

	memcpy(value, buffer->tail, buffer->item_size);

	return SUCCESS;
}

/**
 * Check if the circular buffer is empty.
 */
zend_bool circular_buffer_is_empty(const circular_buffer_t *buffer)
{
  	return buffer->head == buffer->tail;
}

/**
 * Check if the circular buffer is full.
 */
zend_bool circular_buffer_is_full(const circular_buffer_t *buffer)
{
	if (buffer->head == NULL) {
		return 0;
	}

	const void *tail = (buffer->tail != NULL) ? buffer->tail : buffer->start;

	ptrdiff_t used = (char *)buffer->head - (char *)tail;

	if (used < 0) {
		used = -used;
	}

	const ptrdiff_t capacity = (char *)buffer->end - (char *)buffer->start;

	return ((buffer->tail == NULL && buffer->head == buffer->end) || (used == capacity));
}

/**
 * The method will return the number of existing elements currently in the ring buffer.
 * Do not confuse this with the buffer’s capacity!
 */
size_t circular_buffer_count(const circular_buffer_t *buffer)
{
	if (buffer->head == NULL) {
		return 0;
	}

	const void *tail = (buffer->tail != NULL) ? buffer->tail : buffer->start;

	ptrdiff_t dist = (char *)buffer->head - (char *)tail;

	if (dist < 0) {
		dist = -dist;
	}

	return (size_t)(dist / (ptrdiff_t)buffer->item_size) + 1;
}


//
// Functions for ZVAL circular buffer
//
circular_buffer_t *zval_circular_buffer_new(const size_t count, const allocator_t *allocator)
{
	return circular_buffer_new(count, sizeof(zval), allocator);
}

/**
 * Push a new zval into the circular buffer.
 * The zval will be copied and its reference count will be increased.
 */
zend_result zval_circular_buffer_push(circular_buffer_t *buffer, zval *value)
{
	Z_TRY_ADDREF_P(value);
	return circular_buffer_push(buffer, value);
}

/**
 * Pop a zval from the circular buffer.
 * The zval will be copied and its reference count will not be changed because your code will get the ownership.
 */
zend_result zval_circular_buffer_pop(circular_buffer_t *buffer, zval *value)
{
  	ZVAL_UNDEF(value);
	return circular_buffer_pop(buffer, value);
}