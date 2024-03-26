#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "dynamic_array.h"

#define ALLOC_FAILED "Error: failed memory allocation in dynamic_array module."

static void mem_alloc_error()
{
	fprintf(stderr, "%s\n", ALLOC_FAILED);
	exit(EXIT_FAILURE);
}
	
struct d_array *d_array_new(struct arena *mem, const size_t length, const size_t size, const size_t granularity)
{
	struct d_array *vec;
	assert(size > 0 && granularity > 0);

	if (mem)
	{
		vec = arena_push(mem, NULL, sizeof(struct d_array));
		vec->data = arena_push(mem, NULL, length*size);
	}
	else
	{
		vec = malloc(sizeof(struct d_array));
		if (!vec)
			mem_alloc_error();
		
		if (vec->length > 0) {
			vec->data = calloc((length*size), sizeof(char));
			if (vec->data == NULL)
				mem_alloc_error();
		} else {
			vec->data = NULL;
		}


	}

	vec->length = length;
	vec->size = size;
	vec->granularity = granularity;
	vec->max_used = SIZE_MAX;

	return vec;
}

struct d_array *d_array_init(const size_t length, const size_t size, void * const data, const size_t granularity)
{
	assert(size > 0 && granularity > 0);

	struct d_array *vec = malloc(sizeof(struct d_array));
	if (!vec)
		mem_alloc_error();

	vec->length = length;
	vec->size = size;
	vec->data = data;
	vec->granularity = granularity;
	vec->max_used = length - 1;

	return vec;
}

void d_array_set_granularity(struct d_array * const src, const size_t new_granularity)
{
	assert(src && new_granularity > 0);

	src->granularity = new_granularity;
}

struct d_array *d_array_copy(const struct d_array * const src, const size_t length)
{
	assert(src);

	if (length == 0)
		return NULL;

	struct d_array *cpy = malloc(sizeof(struct d_array));
	if (!cpy)
		mem_alloc_error();

	cpy->data = calloc(length, src->size);
	if (cpy->data == NULL)
		mem_alloc_error();

	cpy->length = length;
	cpy->size = src->size;

	if (length > src->length)
		memcpy(cpy->data, src->data, (src->size * src->length));
	else
		memcpy(cpy->data, src->data, (src->size * length));

	return cpy;
}

void *d_array_get(const struct d_array * const vec, const size_t index)
{
	if (!vec || vec->length == 0 || index >= vec->length)
		return NULL;
	else
		return (char *) vec->data + index * vec->size;
}

static void auto_resize(struct d_array * const src, const size_t index)
{
	assert(index >= src->length);

	int new_size = (2 * src->length) + 1 - index;
	const int mod = new_size % src->granularity;

	if (mod) {
		new_size += src->granularity - mod;
	}

	d_array_resize(src, new_size);
}

void *d_array_set(struct d_array * const vec, const size_t index, const void * const data)
{
	if (!vec || vec->length == 0)
		return NULL;

	if (index >= vec->length)
		auto_resize(vec, index);
		

	void *index_addr = (char *) vec->data + index * vec->size;
	memcpy(index_addr, data, vec->size);

	if (index > vec->max_used || vec->max_used == SIZE_MAX)
		vec->max_used = index;
	
	return index_addr;
}

void *d_array_add(struct d_array * const vec, const void * const data)
{
	if (!vec || vec->length == 0)
		return NULL;

	vec->max_used += 1;
	if (vec->max_used >= vec->length)
		auto_resize(vec, vec->max_used);

	void *index_addr = (char *) vec->data + vec->max_used * vec->size;
	if (data)
		memcpy(index_addr, data, vec->size);
	
	return index_addr;
}

void d_array_resize(struct d_array * const vec, const size_t length)
{
	if (!vec || length == 0 || vec->length == 0)
		return;

	vec->data = realloc(vec->data, length * vec->size);
	
	if (vec->data == NULL)
		mem_alloc_error();

	if (vec->length < length) {
		void *new_block = (char *) vec->data + vec->length * vec->size;
		memset(new_block, 0, (length - vec->length) * vec->size);
	}
	
	vec->length = length;
}

void d_array_free(struct d_array *vec)
{
	if (vec == NULL)
		return;
	
	free(vec->data);
	free(vec);
}

size_t d_array_addr_to_index(const struct d_array *vec, const void *addr)
{
	uintptr_t byte_offset = ((uintptr_t) addr) - ((uintptr_t) vec->data);
	return byte_offset / vec->size;
}
