#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hash_index.h"
#include "mmath.h"

/**
 * @hash: An array with set length that is empty when each index is set to -1. hash[i] != -1 is a
 * 	index to either a user-defined value array, or the index_chain
 * @index_chain: A dynamic array containing indices to a user-defined value array. index_chain[i] = -1
 * 	means it empty and can be used
 * @hash_size: The hash's size (Forced to be a power of 2) so we can use AND insteaf of modulo operations
 * @index_size: The index chain's size
 * @granularity: The granularity of the hash table
 * @hash_mask: hash_size - 1, makes us possible to use AND operation instead of more expensive modulo operation
 * @lookup_mask: 0 when the hash table is empty, -1=0xFFFF... otherwise. Hence when looking up a value, if the
 * 	hash table is empty, we AND the key with 0, yielding 0, which is the correct index in INVALID_INDEX.
 * 	Otherwise, if the hash table isn't empty, we AND with 0xFFFF... which is a identity operation, or no op.
 */

/* adress pointed to by hash and index_chain in order for lookup to work on a empty hash table */
static int INVALID_INDEX[1] = {-1};

struct hash_index *hash_new_default(void)
{
	struct hash_index *hi = malloc(sizeof(struct hash_index));

	hi->mem = NULL;
	hi->hash = INVALID_INDEX;
	hi->index_chain = INVALID_INDEX;
	hi->hash_size = DEFAULT_HASH_SIZE;
	hi->index_size = DEFAULT_HASH_SIZE;
	hi->granularity = DEFAULT_GRANULARITY;
	hi->hash_mask = DEFAULT_HASH_SIZE - 1;
	hi->lookup_mask = 0;

	return hi;
}

struct hash_index *hash_new(struct arena *mem, const size_t new_hash_size, const size_t initial_index_size)
{
	assert(is_power_of_two(new_hash_size));

	struct hash_index *hi;
	if (mem)
	{
		hi = (struct hash_index *) arena_push_packed(mem, NULL, sizeof(struct hash_index));

		hi->mem = mem;
		hi->hash_size = new_hash_size;
		hi->index_size = initial_index_size;
		hi->granularity = DEFAULT_GRANULARITY;
		hi->hash_mask = new_hash_size - 1;
		hi->index_size = initial_index_size;
		hi->lookup_mask = -1;
		hi->hash = (i32 *) arena_push_packed(mem, NULL, hi->hash_size * sizeof(int));
		hi->index_chain = (i32 *) arena_push_packed(mem, NULL, hi->index_size * sizeof(int));
		memset(hi->hash, -1, hi->hash_size * sizeof(int));
		memset(hi->index_chain, -1, initial_index_size * sizeof(int));
	}
	else
	{
		hi = malloc(sizeof(struct hash_index));
		
		hi->hash = INVALID_INDEX;
		hi->index_chain = INVALID_INDEX;
		hi->hash_size = new_hash_size;
		hi->index_size = initial_index_size;
		hi->granularity = DEFAULT_GRANULARITY;
		hi->hash_mask = new_hash_size - 1;
		hi->lookup_mask = 0;
	}

	return hi;
}

void hash_free(struct hash_index * const hi)
{
	if (hi->mem)
	{
		arena_pop_packed(hi->mem, hash_mem_size(hi)); 
	}
	else
	{
		if (hi->hash != INVALID_INDEX)
			free(hi->hash);
		if (hi->index_chain != INVALID_INDEX)
			free(hi->index_chain);
		free(hi);
	}
}

size_t hash_mem_allocated(const struct hash_index *const hi)
{
	return (hi->hash_size + hi->index_size) * sizeof(int);
}

size_t hash_mem_size(const struct hash_index *const hi)
{
	return hash_mem_allocated(hi) + sizeof(struct hash_index);
}

size_t hash_get_hash_size(const struct hash_index * const hi)
{
	return hi->hash_size;
}

size_t hash_get_index_size(const struct hash_index * const hi)
{
	return hi->index_size;
}

static void allocate(struct hash_index * const hi, const int new_index_size)
{
	hi->hash = malloc(hi->hash_size * sizeof(int));
	memset(hi->hash, -1, hi->hash_size * sizeof(int));
	hi->index_chain = malloc(new_index_size * sizeof(int));
	memset(hi->index_chain, -1, new_index_size * sizeof(int));
	hi->index_size = new_index_size;
	hi->lookup_mask = -1;
}

void hash_add(struct hash_index * const hi, const int key, const int index)
{
	assert(index >= 0);

	if (hi->hash == INVALID_INDEX)
		allocate(hi, index >= hi->index_size ? index + 1 : hi->index_size);
	else if (index >= hi->index_size)
		hash_resize_index(hi, index + 1);

	int h = key & hi->hash_mask;
	hi->index_chain[index] = hi->hash[h];
	hi->hash[h] = index;
}

void hash_remove(const struct hash_index * const hi, const int key, const int index)
{
	if (hi->hash == INVALID_INDEX)
		return;

	const int h = key & hi->hash_mask;

	if (hi->hash[h] == index) {
		hi->hash[h] = hi->index_chain[index];
	} else {
		for (int i = hi->hash[h]; i != -1; i = hi->index_chain[i]) {
			if (hi->index_chain[i] == index) {
				hi->index_chain[i] = hi->index_chain[index];
				break;
			}
		}
	}
	hi->index_chain[index] = -1;
}	

int hash_first(const struct hash_index * const hi, const int key)
{
	return hi->hash[key & hi->hash_mask & hi->lookup_mask];
}

int hash_next(const struct hash_index * const hi, const int index)
{
	assert(index >= 0 && index < hi->index_size);

	return hi->index_chain[index & hi->lookup_mask];
}

void hash_insert_index(struct hash_index * const hi, const int key, const int index)
{
	int i, max;

	if (hi->hash != INVALID_INDEX) {
		max = index;
		for (i = 0; i < hi->hash_size; ++i) {
			if (hi->hash[i] >= index) {
				hi->hash[i]++;
				if (hi->hash[i] > max)
					max = hi->hash[i];
			}
		}
		for (i = 0; i < hi->index_size; ++i) {
			if (hi->index_chain[i] >= index) {
				hi->index_chain[i]++;
				if (hi->index_chain[i] > max)
					max = hi->index_chain[i];
			}
		}
		if (max >= hi->index_size) 
			hash_resize_index(hi, max + 1);
		for (i = max; i > index; --i) {
			hi->index_chain[i] = hi->index_chain[i-1];
		}
	}

	hash_add(hi, key, index);
}

void hash_remove_index(const struct hash_index * const hi, const int key, const int index)
{
	int i, max;
	
	hash_remove(hi, key, index);
	if (hi->hash != INVALID_INDEX) {
		max = index;
		for (i = 0; i < hi->hash_size; ++i) {
			if (hi->hash[i] >= index) {
				if (hi->hash[i] > max)
					max = hi->hash[i];
				hi->hash[i] -= 1;
			}
		}
		for (i = 0; i < hi->index_size; ++i) {
			if (hi->index_chain[i] >= index) {
				if (hi->index_chain[i] > max)
					max = hi->index_chain[i];
				hi->index_chain[i] -= 1;
			}
		}
		for (i = index; i < max; ++i)
			hi->index_chain[i] = hi->index_chain[i+1];
		hi->index_chain[max] = -1;
	}
}

void hash_clear(const struct hash_index * const hi)
{
	if (hi->hash != INVALID_INDEX) {
		memset(hi->hash, -1, hi->hash_size * sizeof(int));
		memset(hi->index_chain, -1, hi->index_size * sizeof(int));
	}
}

void hash_set_granularity(struct hash_index * const hi, const int new_granularity)
{
	assert(new_granularity > 0);
	
	hi->granularity = new_granularity;
}

void hash_resize_index(struct hash_index * const hi, const int new_index_size)
{
	if (new_index_size <= hi->index_size)
		return;

	int new_size;
	const int mod = new_index_size % hi->granularity;

	if (!mod)
		new_size = new_index_size;
	else
		new_size = new_index_size + hi->granularity - mod;

	if (hi->index_chain == INVALID_INDEX) {
		hi->index_size = new_size;
		return;
	}

	if (hi->mem)
	{
		hi->index_chain = (i32 *) arena_push_packed(hi->mem, NULL, (new_size - hi->index_size) * sizeof(int));
		memset(hi->index_chain + hi->index_size, -1, (new_size - hi->index_size) * sizeof(int));
		hi->index_size = new_size;
	}
	else
	{
		hi->index_chain = realloc(hi->index_chain, new_size * sizeof(int));
		memset(hi->index_chain + hi->index_size, -1, (new_size - hi->index_size) * sizeof(int));
		hi->index_size = new_size;
	}
}

int *hash_index_hash_ptr(struct hash_index *const hi)
{
	return hi->hash;
}

int hash_generate_key_int(const int n)
{
	return n;
}

int hash_generate_key_str(const char *str)
{
	int hash = 0;
	for (int i = 0; *str != '\0'; ++str, ++i)
		hash += *str * (i+119);
	return hash;
}
