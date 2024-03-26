#ifndef __VECTOR__
#define __VECTOR__

#include "mg_mempool.h"

#define NUM_TEXTURE_GRANULARITY 32

/**
 * struct d_array - A dynamic array
 *
 * @length: The length of the dynamic_array.
 * @size: The size of each individual element.
 * @data: The array of length @length with elements of size @size
 * @granularity: The size to increase the dynamic array with when reallocation is performed
 * @max_used: The highest used index (useful for d_array_add).
 *
 * A dynamic_array with length N has valid indices [0,N-1] just like an array. Anything
 * else is out of bounds.
 */
struct d_array {
	void *data;
	size_t length;
	size_t size;
	size_t granularity;
	size_t max_used;
};

/**
 * dynamic_array_new() - Create a new dynamic_array.
 * @length: Length of the new dynamic_array.
 * @size: Size of each element.
 *
 * Creates a new dynamic_array of given length where each element is of the given size.
 *
 * Return: The new dynamic_array.
 *
 * Error: It is a checked runtime error for the size to be 0. It is an unchecked
 * 	  runtime error for the given size not take into account any padding that
 * 	  might be needed, as the array will be allocated using (length * size).
 * 	  If memory allocation failed, exit.
 */
struct d_array *d_array_new(struct arena *mem, const size_t length, const size_t size, const size_t granularity);

/**
 * dynamic_array_init() - Create a new dynamic_array with given array.
 * @length: Length of the new dynamic_array.
 * @size: Size of each element.
 * @data: The array containing data.
 *
 * The dynamic_array that is created will deallocate the given array itself if called
 * with dynamic_array_free().
 *
 * Return: The new dynamic_array.
 *
 * Error: It is an unchecked runtime error for the array to be an address to stack.
 * 	  It is another unchecked runtime error to not set valid length and size
 * 	  values such that (length * size) is the number of bytes stored in the
 * 	  array.
 */
struct d_array *d_array_init(const size_t length, const size_t size, void * const data, const size_t granularity);

/**
 * d_array_set_granularity() - Set a new granularity.
 * @src: The d_array
 * @new_granularity: The new granularity
 */
void d_array_set_granularity(struct d_array * const src, const size_t new_granularity);


/**
 * dynamic_array_copy() - Copy a dynamic_array's @length first elements.
 * @src: The dynamic_array to be copied from.
 * @length: The length of the new dynamic_array.
 *
 * Creates a new dynamic_array and copies the length first elements from the given dynamic_array.
 * If the given length exceeds the given dynamic_array's length, extra storage will be
 * added and set to 0 in the new dynamic_array. If the given length is 0, null is
 * return.
 *
 * Return: The new dynamic_array.
 *
 * Error: It is a checked runtime error that the given dynamic_array is null. If memory
 * 	  allocation fails, exit.
 */
struct d_array *d_array_copy(const struct d_array * const src, const size_t length);

/**
 * dynamic_array_get() - Get the address of a given index in a dynamic_array.
 * @vec: The dynamic_array.
 * @index: The index.
 *
 * Return: The address of the dynamic_array's index.
 *
 * The returned value is either the address  of the dynamic_array's index, or null if the
 * user tries to access an out of bounds index, or if the dynamic_array is of length 0,
 * or if the dynamic_array is simply null.
 *
 * Error: Using the returned value after the dynamic_array has been resized
 * 	  is an unchecked runtime error.
 */
void *d_array_get(const struct d_array * const vec, const size_t index);

/**
 * dynamic_array_set() - Set the given index of a dynamic_array to some data.
 * @vec: The dynamic_array.
 * @index: The index.
 * @data: The address to the data.
 *
 * Return: The address of the dynamic_array's index.
 *
 * The returned value is either the address of the dynamic_array's index, or null if the
 * user tries to access an out of bounds index, or if the dynamic_array is of length 0,
 * or if the dynamic_array is simply null.
 *
 * Error: It is an unchecked runtime error for the data pointed to by data_addr to
 *        overlap with the dynamic_array's index storage.
 */
void *d_array_set(struct d_array * const vec, const size_t index, const void * const data);

/**
 * d_array_add() - Add data to index max_used + 1, increasing the size if needed.
 * @vec: The dynamic_array.
 * @data: The address to the data.
 *
 * Return: The address of the dynamic_array's index.
 *
 * The returned value is either the address of the dynamic_array's index, or null if the
 * user tries to access an out of bounds index, or if the dynamic_array is of length 0,
 * or if the dynamic_array is simply null.
 *
 * Error: It is an unchecked runtime error for the data pointed to by data_addr to
 *        overlap with the dynamic_array's index storage.
 
 */
void *d_array_add(struct d_array * const vec, const void * const data);

/**
 * dynamic_array_resize() - Resize a dynamic_array.
 * @vec: The dynamic_array.
 * @length: The new length.
 *
 * Resizes a given dynamic_array to a new length, hence adding or removing storage at
 * the last index of the dynamic_array. If storage is added, it will be set to 0. If
 * the new length is 0, or if the dynamic_array is empty, or simply null, do nothing.
 *
 * Error: If memory reallocation fails, exit.
 */
void d_array_resize(struct d_array * const vec, const size_t length);

/**
 * dynamic_array_free() - Free a dynamic_array.
 * @vec: The dynamic_array.
 *
 * If the given dynamic_array is not null, deallocate the dynamic_array and its array, otherwise do nothing.
 */
void d_array_free(struct d_array *vec);

size_t d_array_addr_to_index(const struct d_array *vec, const void *addr);

#endif
