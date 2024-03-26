#ifndef __random__
#define __random__

#include "stdint.h"

/**
 * Generate 32-bit seed
 */
void gen_seed_32(uint32_t * const seed);

/**
 * Initiade mersenne twister PRNG
 */
void mersenne_twister_init(const unsigned long seed);

/**
 * Generate doubles uniformly on (0,1), expects mersenne to be initiated with valid seed.
 */
double gen_rand(void);
float gen_rand_f(void);

/**
 * Generate doubles uniformly on the interval (lower, upper), expects mersenne to be 
 * initialized with valid seed.
 */
double gen_continuous_uniform(const double lower, const double upper);
float gen_continuous_uniform_f(const float lower, const float upper);

#endif
