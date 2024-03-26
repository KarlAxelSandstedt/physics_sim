#include "stdint.h"
#include "assert.h"

#ifdef __linux__
	#include "sys/random.h"
#elif _WIN64
	
#endif

/**
 * Implementation of the Mersenne Twister.
 *
 * word_size - number of bits of the integer generated in [0, 2^w-1}
 */

#define N 624
#define M 397
#define A 0x9908b0df /* constant matrix */
#define UPPER_MASK 0x80000000 /* w-r bits */
#define LOWER_MASK 0x7fffffff /* r */

#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define SHIFT_U 11
#define SHIFT_S 7
#define SHIFT_T 15
#define SHIFT_L 18

static uint32_t state[N];
static uint32_t mat[2] = {0, A};
static int index = -1;

void gen_seed_32(uint32_t * const seed)
{
#ifdef __linux__
	size_t bytes = getrandom(seed, sizeof(uint32_t), 0);
	assert(bytes == sizeof(uint32_t));
#elif _WIN64
#define _CRT_RAND_S
#include <stdlib.h>
	rand_s(seed);
#endif
}

void mersenne_twister_init(const uint32_t seed)
{
	assert(seed != 0);

	state[0] = seed & 0xffffffff;
	for (index = 1; index < N; ++index)
		state[index] = (69069 * state[index-1]) & 0xffffffff;
	
	index = 0;
}

static uint32_t mesenne_twister_generate(void)
{
	assert(index != -1);

	uint32_t y = (state[index] & UPPER_MASK) | (state[(index+1) % N] & LOWER_MASK);
	state[index] = state[(index + M) % N] ^ (y >> 1) ^ mat[y & 0x1];

	y = state[index];
	y ^= y >> SHIFT_U;
	y ^= (y << SHIFT_S) & TEMPERING_MASK_B;
	y ^= (y << SHIFT_T) & TEMPERING_MASK_C;
	y ^= y >> SHIFT_L;

	index = (index + 1) % N;

	return y;
}

double gen_rand(void)
{
	const uint32_t y = mesenne_twister_generate();
	return (double) y / (uint32_t) 0xffffffff;
}

double gen_continuous_uniform(const double lower, const double upper)
{
	double real = gen_rand();
	return lower + (upper-lower) * real;
}

float gen_rand_f(void)
{
	const uint32_t y = mesenne_twister_generate();
	return (float) y / (uint32_t)0xffffffff;
}

float gen_continuous_uniform_f(const float lower, const float upper)
{
	float real = gen_rand_f();
	return lower + (upper - lower) * real;
}