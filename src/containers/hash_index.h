#ifndef __HASH_INDEX__
#define __HASH_INDEX__

#include "mg_common.h"
#include "mg_mempool.h"

/**
 * hash_index - A fast cache coherent hash table. The user defines his or her own dynamic array of values. To find a value
 * 	corresponding to a generated key, the user retrieves indices from the hash table which are to be used to check for
 * 	the wanted value in the dynamic array.
 */
struct hash_index {
	struct arena *mem;
	int *hash;
	int *index_chain;
	int hash_size;
	int index_size;
	int granularity;
	int hash_mask;
	int lookup_mask;
};


#define DEFAULT_HASH_SIZE	1024
#define DEFAULT_GRANULARITY	1024

/**
 * hash_index_new() - Create a new hash table on the heap.
 */
struct hash_index *hash_new(struct arena *mem, const size_t new_hash_size, const size_t initial_index_size);

/**
 * hash_index_new_default() - Create a new hash table on the heap with default size and granularity.
 */
struct hash_index *hash_new_default(void);

/**
 * hash_index_free() - Destroy the hash table.
 */
void hash_free(struct hash_index * const hi);

/**
 * hash_mem_allocated() - Return the size of allocated memory
 */
size_t hash_mem_allocated(const struct hash_index * const hi);

/**
 * hash_mem_size() - Return the size of allocated memory and hash_index type size
 */
size_t hash_mem_size(const struct hash_index * const hi);

/**
 * hash_get_hash_size() - Get the size of hash.
 */
size_t hash_get_hash_size(const struct hash_index * const hi);

/**
 * hash_get_index_size() - Get the size of the indices. 
 */
size_t hash_get_index_size(const struct hash_index * const hi);

/**
 * hash_index_add() - Add an index to the hash table.
 *
 * Add an index to the hash table, assumes the index has not yet been added to the hash table.
 */
void hash_add(struct hash_index * const hi, const int key, const int index);

/**
 * hash_index_remove() - Remove an index from the hash table.
 */
void hash_remove(const struct hash_index * const hi, const int key, const int index);

/**
 * hash_index_first() - Get the first entry from the hash table.
 *
 * Return: -1 if the entry is empty.
 */
int hash_first(const struct hash_index * const hi, const int key);

/**
 * hash_index_next() - Get the next index of the hash table.
 *
 * Return: -1 if the end of the hash chain has been reached.
 */
int hash_next(const struct hash_index * const hi, const int index);

/**
 * hash_insert_index() - Insert an entry into the index and add it to the hash, increasing all indices >= index.
 */
void hash_insert_index(struct hash_index * const hi, const int key, const int index);

/**
 * hash_remove_index() - Remove an entry from the index and remove it from the hash, decreasing all indices >= index.
 */
void hash_remove_index(const struct hash_index * const hi, const int key, const int index);

/**
 * hash_clear() - Clear the hash table.
 */
void hash_clear(const struct hash_index * const hi);

/**
 * hash_set_granularity() - Set granularity.
 */
void hash_set_granularity(struct hash_index * const hi, const int granularity);

/**
 * hash_resize_index() - Resize the index without touching the hash table.
 */
void hash_resize_index(struct hash_index * const hi, const int new_index_size);

int *hash_index_hash_ptr(struct hash_index *const hi);

/**
 * hash_get_spread() - Get the spread over the hash table.
 *
 * Return: A number in the range [0,100] representing the spread over the hash table.
 */
int hash_get_spread(const struct hash_index * const hi);

/**
 * hash_generate_key_int() - Generate key from int source
 */
int hash_generate_key_int(const int n);

/**
 * hash_generate_key_int() - Generate key from string source
 */
int hash_generate_key_str(const char * const str);

#endif
