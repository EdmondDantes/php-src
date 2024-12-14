
#include "chunked_vector.h"
#include "php.h"
#include "zend_types.h"

const size_t CACHE_LINE_SIZE = 64;
const size_t MEMORY_MIN_SIZE = 128;
const size_t CACHE_OPT_SIZE = 256;		// 4 cache lines
const size_t CACHE_MAX_SIZE = 4096;		// Typical for x86_64

/**
 * Allocate a new chuncked vector.
 * 
 * @param item_size				Size of each item in the vector.
 * @param reserve_items			Number of items to reserve memory for.
 * @param trigger_chunk_size	The amount of memory after which a new CHUNK is created.
 * 
 */
chuncked_vector_t* allocate_chuncked_vector(size_t item_size, int reserve_items, size_t trigger_chunk_size)
{
	if (item_size == 0) {
		return NULL;
	}

	if (trigger_chunk_size <= MEMORY_MIN_SIZE) {
		trigger_chunk_size = CACHE_OPT_SIZE - sizeof(chuncked_vector_t);
	} else if (trigger_chunk_size > CACHE_MAX_SIZE) {
		trigger_chunk_size = CACHE_MAX_SIZE - sizeof(chuncked_vector_t);
	}

	// Calculate the initial size of the vector
	size_t initial_memory_size = reserve_items * item_size;

	if (initial_memory_size <= 0) {
		initial_memory_size = item_size;
	}

	if (initial_memory_size > trigger_chunk_size) {
		initial_memory_size = trigger_chunk_size;
	}

	chuncked_vector_t* vector	= (chuncked_vector_t*)pecalloc(1, sizeof(chuncked_vector_t), 1);

	if (!vector) {
		return NULL;
	}

	vector->item_size			= item_size;
	vector->trigger_chunk_size  = trigger_chunk_size;
	vector->initial_memory_size = initial_memory_size;

	size_t required_memory		= sizeof(chuncked_vector_chunck_t) + initial_memory_size;
	chuncked_vector_chunck_t* first = (chuncked_vector_chunck_t*) pecalloc(1, required_memory, 1);

	first->total				= initial_memory_size;
	first->first.used			= 0;
	first->first.next			= NULL;

	vector->first				= first;

	return vector;
}

/**
 * Expand the chuncked vector.
 *
 * @param vector			The vector.
 * @param chunk				The current chunk to expand.
 * @param prev_chunk		The previous chunk.
 * @param required_memory	The amount of memory to expand.
 * @return					The new chunk.
 */
chuncked_vector_chunck_t* chuncked_vector_expand(chuncked_vector_t* vector, chuncked_vector_chunck_t* chunk, chuncked_vector_chunck_t* prev_chunk, size_t required_memory)
{
	if (UNEXPECTED(required_memory <= 0)) {
		required_memory		= vector->item_size;
	}

	if ((chunck->used + required_memory) > vector->trigger_chunk_size) {
		// Allocate a new chunk
		size_t new_chunk_size	= vector->initial_memory_size;

		if (new_chunk_size < required_memory) {
			new_chunk_size = required_memory;
		}

		size_t summary_size = sizeof(chuncked_vector_t) + new_chunk_size;

		chuncked_vector_chunck_t* new_chunk = (chuncked_vector_chunck_t*)pecalloc(1, summary_size, 1);

		if (!new_chunk) {
			return NULL;
		}

		new_chunk->total	= new_chunk_size;
		new_chunk->used		= 0;
		new_chunk->next		= NULL;

		chunck->next		= new_chunk;

		return new_chunk;
	}

	// Relloacte the current chunk
	size_t new_memory_size	= chunk->total + required_memory;	

	chunk->total			= new_memory_size;

	// Allocate memory for the new chunk
	void* new_chunk			= (void*) pecalloc(1, sizeof(chuncked_vector_chunck_t) + new_memory_size, 1);

	memcpy(new_chunk, chunk, sizeof(chuncked_vector_chunck_t) + new_memory_size);
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
chuncked_vector_chunck_t* chuncked_vector_pickup(chuncked_vector_t* vector, size_t required_memory)
{
	if (!vector) {
		return NULL;
	}

	if (required_memory <= 0) {
		required_memory = vector->item_size;
	}

	chuncked_vector_chunck_t* chunck = &vector->first;

	do {
		if (EXPECTED((chunck->total - chunck->used) >= required_memory)) {
			return chunck;
		}

		chunck = chunck->next;

	} while (chunck);

	return chunck;
}

/**
 * Add an item to the chuncked vector.
 *
 * @param vector	The vector to add the item to.
 * @param item		The item to add.
 * @return			A zend_result indicating success or failure of the operation.
 */
zend_result chuncked_vector_add(chuncked_vector_t* vector, const void* item)
{

	if (!vector || !item) {
		return FAILURE;
	}

	chuncked_vector_chunck_t* chunck = chuncked_vector_pickup(vector, vector->item_size);

	if (UNEXPECTED((chunck->total - chunck->used) < vector->item_size)) {

	}
	

	return SUCCESS;
}
