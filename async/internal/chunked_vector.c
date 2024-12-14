
#include "chunked_vector.h"
#include "php.h"
#include "zend_types.h"

const size_t CACHE_LINE_SIZE = 64;
const size_t MEMORY_MIN_SIZE = 128;
const size_t CACHE_OPT_SIZE = 256;		// 4 cache lines
const size_t CACHE_MAX_SIZE = 4096;		// Typical for x86_64

/**
 * Allocate a new chunked vector.
 * 
 * @param item_size				Size of each item in the vector.
 * @param reserve_items			Number of items to reserve memory for.
 * @param trigger_chunk_size	The amount of memory after which a new CHUNK is created.
 * 
 */
chunked_vector_t* chunked_vector_allocate(size_t item_size, int reserve_items, size_t trigger_chunk_size)
{
	if (item_size == 0) {
		return NULL;
	}

	if (trigger_chunk_size <= MEMORY_MIN_SIZE) {
		trigger_chunk_size = CACHE_OPT_SIZE - sizeof(chunked_vector_chunk_t);
	} else if (trigger_chunk_size > (CACHE_MAX_SIZE - sizeof(chunked_vector_chunk_t))) {
		trigger_chunk_size = CACHE_MAX_SIZE - sizeof(chunked_vector_chunk_t);
	}

	// Calculate the initial size of the vector
	size_t initial_memory_size	= reserve_items * item_size;

	if (initial_memory_size <= 0) {
		initial_memory_size		= item_size;
	}

	// Make sure the initial memory size is at least as large as the trigger chunk size
	// It means that each chunk will contain at least one item.
	trigger_chunk_size			= initial_memory_size > trigger_chunk_size ? initial_memory_size : trigger_chunk_size;


	chunked_vector_t* vector	= (chunked_vector_t*) pecalloc(1, sizeof(chunked_vector_t), 1);

	if (!vector) {
		return NULL;
	}

	vector->item_size			= item_size;
	vector->trigger_chunk_size  = trigger_chunk_size;
	vector->initial_memory_size = initial_memory_size;
	vector->item_count			= 0;
	vector->chunk_count			= 1;

	size_t required_memory		= sizeof(chunked_vector_chunk_t) + initial_memory_size;
	chunked_vector_chunk_t* first = (chunked_vector_chunk_t*) pecalloc(1, required_memory, 1);

	first->total				= initial_memory_size;
	first->first.used			= 0;
	first->first.next			= NULL;

	vector->first				= first;

	return vector;
}

/**
 * Expand the chunked vector.
 *
 * @param vector			The vector.
 * @param chunk				The current chunk to expand.
 * @param prev_chunk		The previous chunk.
 * @param required_memory	The amount of memory to expand.
 * @return					The new chunk.
 */
chunked_vector_chunk_t* chunked_vector_expand(chunked_vector_t* vector, chunked_vector_chunk_t* chunk, chunked_vector_chunk_t* prev_chunk, size_t required_memory)
{
	if (UNEXPECTED(required_memory <= 0)) {
		required_memory	= vector->item_size;
	}

	size_t new_chunk_size = vector->initial_memory_size;

	if (new_chunk_size < required_memory) {
		new_chunk_size = required_memory;
	}

	if ((chunk->used + required_memory) > vector->trigger_chunk_size) {
		// Allocate a new chunk

		chunked_vector_chunk_t* new_chunk = (chunked_vector_chunk_t*) pecalloc(1, sizeof(chunked_vector_t) + new_chunk_size, 1);

		if (!new_chunk) {
			return NULL;
		}

		new_chunk->total	= new_chunk_size;
		new_chunk->used		= 0;
		new_chunk->next		= NULL;

		chunk->next		= new_chunk;
		vector->chunk_count++;

		return new_chunk;
	}

	// Relloacte the current chunk	
	chunked_vector_chunk_t* new_chunk = (chunked_vector_chunk_t*) pecalloc(1, sizeof(chunked_vector_chunk_t) + new_chunk_size, 1);

	memcpy(new_chunk, chunk, sizeof(chunked_vector_chunk_t) + chunk->used);
	new_chunk->total		= new_chunk_size;

	pefree(chunk, 1);

	if (prev_chunk) {
		prev_chunk->next = new_chunk;
	} else {
		vector->first = new_chunk;
	}

	return new_chunk;
}

/**
 * The method selects a suitable chunk to place the required_memory.
 * If there is no suitable chunk, the method returns last chunk.
 *
 * @param vector	The vector to pick up a chunk from.
 * @return			The selected chunk.
 */
chunked_vector_chunk_t* chunked_vector_pickup(chunked_vector_t* vector, size_t required_memory)
{
	if (!vector) {
		return NULL;
	}

	if (required_memory <= 0) {
		required_memory = vector->item_size;
	}

	chunked_vector_chunk_t* chunk = vector->first;

	do {
		if (EXPECTED((chunk->total - chunk->used) >= required_memory)) {
			return chunk;
		}

		chunk = chunk->next;

	} while (chunk);

	return chunk;
}

void* chunked_vector_allocate_items(chunked_vector_t* vector, int count)
{
	if (!vector || count <= 0) {
		return NULL;
	}

	size_t required_memory = vector->item_size * count;

	chunked_vector_chunk_t* chunk = chunked_vector_pickup(vector, required_memory);

	if (UNEXPECTED(!chunk)) {
		return NULL;
	}

	vector->item_count += count;

	return (char*)chunk + sizeof(chunked_vector_chunk_t) + chunk->used;
}

/**
 * Add an item to the chunked vector.
 *
 * @param vector	The vector to add the item to.
 * @param item		The item to add.
 * @return			A zend_result indicating success or failure of the operation.
 */
zend_result chunked_vector_add(chunked_vector_t* vector, const void* item)
{

	if (!vector || !item) {
		return FAILURE;
	}

	void* memory = chunked_vector_allocate_items(vector, 1);

	if (UNEXPECTED(!memory)) {
		return FAILURE;
	}

	memcpy(memory, item, vector->item_size);

	return SUCCESS;
}

/**
 * Remove an item from the chunked vector.
 *
 * @param vector		The vector to remove the item from.
 * @param chunk			The chunk to remove the item from.
 * @param item_offset	The offset of the item to remove.
 * @return				A zend_result indicating success or failure of the operation.
 */
zend_result chunked_vector_remove_from(chunked_vector_t* vector, chunked_vector_chunk_t* chunk, int item_offset)
{
	if (!vector || !chunk || item_offset < 0) {
		return FAILURE;
	}

	size_t memory_offset = vector->item_size * item_offset;
	size_t memory_bound = memory_offset + vector->item_size;

	// If last item in the chunk
	if (memory_bound == chunk->used) {
		chunk->used -= vector->item_size;
		vector->item_count--;
		return SUCCESS;
	} else if(memory_bound > chunk->used) {
		return FAILURE;
	}

	void* memory = (char*)chunk + sizeof(chunked_vector_chunk_t) + memory_offset;

	// Move the memory
	memmove(memory, memory + vector->item_size, vector->item_size);

	chunk->used -= vector->item_size;
	vector->item_count--;

	return SUCCESS;
}

/**
 * Free the memory used by the chunked vector.
 * 
 * @param vector	The vector to free.
 * @return			A zend_result indicating success or failure of the operation.
 */
zend_result chunked_vector_free(chunked_vector_t* vector)
{
	if (!vector) {
		return FAILURE;
	}

	chunked_vector_chunk_t* chunk = vector->first;
	chunked_vector_chunk_t* next = NULL;

	while (chunk) {
		next = chunk->next;
		pefree(chunk, 1);
		chunk = next;
	}

	pefree(vector, 1);

	return SUCCESS;
}

/**
 * Optimize the chunked vector.
 * The method optimizes the vector's chunks and merges them into a single contiguous block of memory.
 * This method can be time and resource intensive.
 * It also removes chunks with empty elements.
 * 
 *
 * @param vector	The vector to optimize.
 * @return			A zend_result indicating success or failure of the operation.
 */
zend_result chunked_vector_optimize(chunked_vector_t* vector)
{
	if (!vector) {
		return FAILURE;
	}

	// No need to optimize if there is only one chunk or no items
	if (vector->chunk_count == 1 || vector->item_count == 0) {
		return SUCCESS;
	}

	size_t required_memory = vector->item_count * vector->item_size;

	chunked_vector_chunk_t* new_chunk = (chunked_vector_chunk_t*) pecalloc(1, sizeof(chunked_vector_t) + required_memory, 1);

	if (!new_chunk) {
		return FAILURE;
	}

	void* memory = (char*)new_chunk + sizeof(chunked_vector_chunk_t);

	chunked_vector_chunk_t* chunk = vector->first;

	chunked_vector_chunk_t* next = NULL;

	// Copy all items to the new big chunk
	while (chunk) {
		next = chunk->next;

		if (chunk->used <= 0) {
			pefree(chunk, 1);
			chunk = next;
			continue;
		}

		memcpy(memory, (char*)chunk + sizeof(chunked_vector_chunk_t), chunk->used);
		memory += chunk->used;
		pefree(chunk, 1);
		chunk = next;
	}

	vector->first = new_chunk;
	vector->chunk_count = 1;

	new_chunk->total = required_memory;
	new_chunk->used = required_memory;
	new_chunk->next = NULL;

	return SUCCESS;
}
