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

#endif // CHUNKED_VECTOR_H
