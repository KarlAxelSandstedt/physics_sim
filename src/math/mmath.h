#ifndef __MORE_MATH_H__
#define __MORE_MATH_H__

#include "mg_common.h"
#include <math.h>
#include <stdbool.h>

#include "SSE_main.h"
#include "matrix.h"
#include "vector.h"
#include "random.h"
#include "transformation.h"
#include "quaternion.h"

#define MM_PI_F       3.14159f
#define MM_PI_2_F     (2.0f * MM_PI_F)

#define MM_PI       3.14159265358979323846
#define MM_PI_2     (2.0 * MM_PI)

#define MG_SSE	/* SSE1, SSE2, SSE3, SSE4.1, SSE4.2 */
#include <immintrin.h>






union f32_bit_representation {
	f32 value;
	i32 bits;
};

#define F32_SIGN_BIT(f32_bits_representation) ((f32_bits_representation.bits & 0x80000000) >> 31)

bool		is_power_of_two(const int n);
uint32_t	power_of_two_ceil(uint32_t n); /* Return smallest 2^k value larget than n, n < 2^(31) */

#endif
