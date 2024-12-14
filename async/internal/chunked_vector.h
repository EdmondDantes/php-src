#ifndef CHUNKED_VECTOR_H
#define CHUNKED_VECTOR_H


typedef _chuncked_vector_s chuncked_vector_t;
typedef _chunked_vector_chunk_s chuncked_vector_chunck_t;

struct _chuncked_vector_s {
	size_t item_size;
	size_t initial_memory_size;
	size_t trigger_chunk_size;
	chuncked_vector_chunck_t *first;
};

struct _chunked_vector_chunk_s {
	chuncked_vector_chunck_t* next;
	size_t total;
	size_t used;
}

#endif // CHUNKED_VECTOR_H
