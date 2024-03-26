#include <stdlib.h>
#include <assert.h>

#include "bit_vector.h"

size_t bit_vec_alloc_size(const uint64_t num_bits)
{
	assert(num_bits >= 1);

	struct bit_vec bvec = { .num_bits = num_bits };
	bvec.num_blocks = num_bits / BIT_VECTOR_BLOCK_SIZE;
	if (num_bits % BIT_VECTOR_BLOCK_SIZE) {
		bvec.num_blocks += 1;
	}

	return bvec.num_blocks * (BIT_VECTOR_BLOCK_SIZE / 8);
}

struct bit_vec bit_vec_init(const uint64_t num_bits, const uint64_t num_blocks, const uint64_t clear_bit, uint64_t* bits)
{
	assert(num_bits >= 1 && num_blocks >= 1 && clear_bit <= 1);

	struct bit_vec bvec = { .num_bits = num_bits, .num_blocks = num_blocks, .bits = bits };
	
	for (uint64_t block = 0; block < bvec.num_blocks; ++block) {
		bvec.bits[block] = UINT64_MAX * clear_bit;
	}

	return bvec;
}

struct bit_vec bit_vec_new(const uint64_t num_bits, const uint64_t clear_bit)
{
	assert(num_bits >= 1 && clear_bit <= 1);

	struct bit_vec bvec = { .num_bits = num_bits };
	bvec.num_blocks = num_bits / BIT_VECTOR_BLOCK_SIZE;
	if (num_bits % BIT_VECTOR_BLOCK_SIZE) {
		bvec.num_blocks += 1;
	}
	bvec.bits = malloc(bvec.num_blocks * sizeof(uint64_t));
	
	for (uint64_t block = 0; block < bvec.num_blocks; ++block) {
		bvec.bits[block] = UINT64_MAX * clear_bit;
	}

	return bvec;
}

struct bit_vec bit_vec_copy(const struct bit_vec* bvec)
{
	struct bit_vec copy = { .bits = bvec->bits, .num_bits = bvec->num_bits, .num_blocks = bvec->num_blocks };

	copy.bits = malloc(bvec->num_blocks * BIT_VECTOR_BLOCK_SIZE);
	for (uint64_t i = 0; i < bvec->num_blocks; ++i) {
		copy.bits[i] = bvec->bits[i];
	}

	return copy;
}

uint8_t bit_vec_get_bit(const struct bit_vec* bvec, const uint64_t bit)
{
	assert(bit < bvec->num_bits);

	const uint64_t block = bit / BIT_VECTOR_BLOCK_SIZE;
	const uint8_t block_bit = bit % BIT_VECTOR_BLOCK_SIZE;

	return (bvec->bits[block] >> block_bit) & 0x1;
}

void bit_vec_set_bit(const struct bit_vec* bvec, const uint64_t bit, const uint64_t bit_value)
{
	assert(bit < bvec->num_bits && bit_value <= 1);

	const uint64_t block = bit / BIT_VECTOR_BLOCK_SIZE;
	const uint8_t block_bit = bit % BIT_VECTOR_BLOCK_SIZE;

	const uint64_t mask = (UINT64_MAX - (((uint64_t)0x1) << block_bit)); /* Get all bits in block but set wanted bit to zero */

	bvec->bits[block] = (bvec->bits[block] & mask) + (bit_value << bit);
}

uint64_t bit_vec_count(const struct bit_vec* bvec)
{
	uint64_t count = 0;
	uint64_t block_index;
	uint64_t block;
	uint8_t shift;

	for (block_index = 0; block_index < bvec->num_blocks - 1; ++block_index) {
		block = bvec->bits[block_index];
		for (shift = 0; shift < BIT_VECTOR_BLOCK_SIZE; ++shift) {
			count += (block >> shift) & ((uint64_t) 0x1);
		}
	}

	const uint64_t bits_last_block = bvec->num_bits - block_index * BIT_VECTOR_BLOCK_SIZE;
	assert(0 < bits_last_block && bits_last_block <= BIT_VECTOR_BLOCK_SIZE);

	block = bvec->bits[block_index];
	for (shift = 0; shift < bits_last_block; ++shift) {
		count += (block >> shift) & ((uint64_t) 0x1);
	}

	return count;
}

uint64_t bit_vec_compare(const struct bit_vec* bvec1, const struct bit_vec* bvec2)
{
	assert((bvec1->num_bits == bvec2->num_bits) && (bvec1->num_blocks == bvec2->num_blocks));

	uint64_t neq = 0;
	for (uint64_t block = 0; block < bvec1->num_blocks; ++block) {
		neq |= bvec1->bits[block] ^ bvec2->bits[block];
	}

	return !neq ? 1 : 0;
}

void bit_vec_clear(struct bit_vec* bvec, const uint64_t clear_bit)
{
	assert(bvec->num_blocks > 0 && bvec->num_bits > 0 && clear_bit <= 1);

	for (uint64_t block = 0; block < bvec->num_blocks; ++block) {
		bvec->bits[block] = UINT64_MAX * clear_bit;
	}
}

uint64_t bit_vec_get_first_occurance(const struct bit_vec* bvec, const uint64_t bit_value)
{
	assert(bvec->num_blocks > 0 && bvec->num_bits > 0 && bit_value <= 1);

	const uint64_t mask = bit_value * UINT64_MAX;
	uint64_t block_index;

	if (bit_value) {
		for (block_index = 0; block_index < bvec->num_blocks; ++block_index) {
			if ((mask | bvec->bits[block_index]) > 0) {
				const uint64_t block = bvec->bits[block_index];
				const uint64_t bits_in_block = (block_index < bvec->num_blocks - 1) ? BIT_VECTOR_BLOCK_SIZE : (bvec->num_bits - block_index * BIT_VECTOR_BLOCK_SIZE);
				for (uint64_t shift = 0; shift < bits_in_block; ++shift) {
					if ((block >> shift) & ((uint64_t)0x1)) {
						return BIT_VECTOR_BLOCK_SIZE * block_index + shift;
					}
				}
			}
		}
	} else {
		for (block_index = 0; block_index < bvec->num_blocks - 1; ++block_index) {
			if ((mask | bvec->bits[block_index]) < UINT64_MAX) {
				const uint64_t block = bvec->bits[block_index];
				const uint64_t bits_in_block = (block_index < bvec->num_blocks - 1) ? BIT_VECTOR_BLOCK_SIZE : (bvec->num_bits - block_index * BIT_VECTOR_BLOCK_SIZE);
				for (uint64_t shift = 0; shift < bits_in_block; ++shift) {
					if (!((block >> shift) & ((uint64_t)0x1))) {
						return BIT_VECTOR_BLOCK_SIZE * block_index + shift;
					}
				}
			}
		}
	}

	return UINT64_MAX;
}

void bit_vec_AND(struct bit_vec* bvec_mutable, const struct bit_vec* bvec_src)
{
	assert( ( bvec_mutable->num_bits == bvec_src->num_bits ) && (bvec_mutable->num_blocks == bvec_src->num_blocks) );
	
	for (uint64_t block = 0; block < bvec_mutable->num_blocks; ++block) {
		bvec_mutable->bits[block] &= bvec_src->bits[block];
	}
}

void bit_vec_OR(struct bit_vec* bvec_mutable, const struct bit_vec* bvec_src)
{
	assert( ( bvec_mutable->num_bits == bvec_src->num_bits ) && (bvec_mutable->num_blocks == bvec_src->num_blocks) );

	for (uint64_t block = 0; block < bvec_mutable->num_blocks; ++block) {
		bvec_mutable->bits[block] |= bvec_src->bits[block];
	}
}

void bit_vec_XOR(struct bit_vec* bvec_mutable, const struct bit_vec* bvec_src)
{
	assert( ( bvec_mutable->num_bits == bvec_src->num_bits ) && (bvec_mutable->num_blocks == bvec_src->num_blocks) );
	
	for (uint64_t block = 0; block < bvec_mutable->num_blocks; ++block) {
		bvec_mutable->bits[block] ^= bvec_src->bits[block];
	}
}
