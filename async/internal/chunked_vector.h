#ifndef CHUNKED_VECTOR_H
#define CHUNKED_VECTOR_H


typedef _chunked_vector_s chunked_vector_t;
typedef _chunked_vector_chunk_s chunked_vector_chunk_t;

struct _chunked_vector_s {
	size_t item_size;
	size_t initial_memory_size;
	size_t trigger_chunk_size;
	int item_count;
	int chunk_count;
	chunked_vector_chunk_t *first;
};

struct _chunked_vector_chunk_s {
	chunked_vector_chunk_t* next;
	size_t total;
	size_t used;
}

#define CHUNKED_VECTOR_FOREACH(vector, type, item_var)                       \
	chunked_vector_chunk_t* chunk = vector->first;						     \
	size_t offset = 0;													     \
	void* base_memory = (char*)chunk + sizeof(chunked_vector_chunk_t);	     \
	while (chunk != NULL) { \
		if (offset >= chunk->used) { \
			chunk = chunk->next; \
			offset = 0; \
			continue; \
		} \
		type* item_var = base_memory + offset;


#define CHUNKED_VECTOR_FOREACH_END() \
		offset += vector->item_size; \
	}

chunked_vector_t* chunked_vector_allocate(size_t item_size, int reserve_items, size_t trigger_chunk_size);
chunked_vector_chunk_t* chunked_vector_expand(chunked_vector_t* vector, chunked_vector_chunk_t* chunk, chunked_vector_chunk_t* prev_chunk, size_t required_memory);
chunked_vector_chunk_t* chunked_vector_pickup(chunked_vector_t* vector, size_t required_memory);
void* chunked_vector_allocate_items(chunked_vector_t* vector, int count);
zend_result chunked_vector_add(chunked_vector_t* vector, const void* item);
zend_result chunked_vector_remove_from(chunked_vector_t* vector, chunked_vector_chunk_t* chunk, int item_offset);
zend_result chunked_vector_free(chunked_vector_t* vector);
zend_result chunked_vector_optimize(chunked_vector_t* vector);

#endif // CHUNKED_VECTOR_H
