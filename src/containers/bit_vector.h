#ifndef __BIT_VECTOR_H__
#define __BIT_VECTOR_H__

#include <stdint.h>

#define BIT_VECTOR_BLOCK_SIZE (8 * sizeof(uint64_t))

/* bits : [0, num_bits-1] */
struct bit_vec {
	uint64_t num_blocks;
	uint64_t num_bits;
	uint64_t* bits;
};

size_t bit_vec_alloc_size(const uint64_t num_bits);	/* Return size needed for bit vector */
struct bit_vec bit_vec_init(const uint64_t num_bits, const uint64_t num_blocks, const uint64_t clear_bit, uint64_t* bits);	/* Init the bit vector with preallocated bits and clear bit */
struct bit_vec bit_vec_new(const uint64_t num_bits, const uint64_t clear_bit);	/* initialize the bit_vec with clear_bit [Dont forget to perhaps free bits if not allocated in a pool] */
struct bit_vec bit_vec_copy(const struct bit_vec* bvec);
uint8_t bit_vec_get_bit(const struct bit_vec* bvec, const uint64_t bit);
void bit_vec_set_bit(const struct bit_vec* bvec, const uint64_t bit, const uint64_t bit_value);
uint64_t bit_vec_count(const struct bit_vec* bvec);										/* Get the count of 1's in the bit vector */
uint64_t bit_vec_compare(const struct bit_vec* bvec1, const struct bit_vec* bvec2);     /* 0 if non equivalent, 1 if equivalent */
void bit_vec_clear(struct bit_vec* bvec, const uint64_t clear_bit);
uint64_t bit_vec_get_first_occurance(const struct bit_vec* bvec, const uint64_t bit_value);				/* Find first index where bit_value is found, returns UINT64_MAX if no occurance */
void bit_vec_AND(struct bit_vec *bvec_mutable, const struct bit_vec *bvec_src);			/* Apply method between two bitvectors and store result in bvec_mutable */
void bit_vec_OR(struct bit_vec* bvec_mutable, const struct bit_vec* bvec_src);
void bit_vec_XOR(struct bit_vec* bvec_mutable, const struct bit_vec* bvec_src);

#endif
