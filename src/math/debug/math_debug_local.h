#ifndef __MATH_DEBUG_LOCAL_H__
#define __MATH_DEBUG_LOCAL_H__

#include "math_debug_public.h"

/*************** IEEE-754 emulation ***************/
 
#define		IEEE32_SIGN_LENGTH	1
#define		IEEE32_EXPONENT_LENGTH	8
#define		IEEE32_SIGNIFICAND_LENGTH	23

#define		IEEE32_SIGN_MASK	0x80000000
#define		IEEE32_EXPONENT_MASK	0x7f800000
#define		IEEE32_SIGNIFICAND_MASK	0x007fffff

#define		IEEE32_BIAS		127
#define		IEEE32_MAX_EXPONENT	127
#define		IEEE32_MIN_EXPONENT	-126

#define		IEEE32_MAX_POSITIVE_SUBNORMAL	f32_max_positive_subnormal()
#define		IEEE32_MIN_POSITIVE_SUBNORMAL	f32_min_positive_subnormal()
#define		IEEE32_MAX_NEGATIVE_SUBNORMAL	f32_max_negative_subnormal()
#define		IEEE32_MIN_NEGATIVE_SUBNORMAL	f32_min_negative_subnormal()

#define		IEEE32_MAX_POSITIVE_NORMAL	f32_max_positive_normal()
#define		IEEE32_MIN_POSITIVE_NORMAL	f32_min_positive_normal()
#define		IEEE32_MAX_NEGATIVE_NORMAL	f32_max_negative_normal()
#define		IEEE32_MIN_NEGATIVE_NORMAL	f32_min_negative_normal()

void ieee32_print(FILE *file, const union ieee32 val);
void ieee32_bits_print(FILE *file, const u32 bits);

/* return bit value of sign in FP f or ieee32 f */
u32 f32_sign(const f32 f);
u32 ieee32_sign(const union ieee32 f);

u32 ieee32_exponent(const union ieee32 f); /* shift down and return exponent */
u32 ieee32_significand(const union ieee32 f); /* shift down and return significand */

u32 ieee32_significand_length_without_trailing_zeroes(union ieee32 src);
u64 ieee32_leading_zero_count(const union ieee32 f); /* return leading number of zeroes b1 b2 ... in significand */
u32 ieee32_bit(const union ieee32 f, const u32 bit); /* get bit value of given bit */

/* Return 1 if f is prefix, otherwise 0 */
enum ieee_type ieee32_classify(const f32 f);
u32 ieee32_test_nan(const f32 f);
u32 ieee32_test_positive_inf(const f32 f);
u32 ieee32_test_negative_inf(const f32 f);
u32 ieee32_test_positive_zero(const f32 f);
u32 ieee32_test_negative_zero(const f32 f);
u32 ieee32_test_normal(const f32 f);
u32 ieee32_test_subnormal(const f32 f);

union ieee32 ieee32_set(const u32 sign, const i32 exponent, const u32 significand); /* integer values, not mask */
union ieee32 ieee32_nan(void);
union ieee32 ieee32_inf(const u32 sign);
union ieee32 ieee32_zero(const u32 sign);

union ieee32 ieee32_max_positive_subnormal(void);
union ieee32 ieee32_min_positive_subnormal(void);
union ieee32 ieee32_max_negative_subnormal(void);
union ieee32 ieee32_min_negative_subnormal(void);

union ieee32 ieee32_max_positive_normal(void);
union ieee32 ieee32_min_positive_normal(void);
union ieee32 ieee32_max_negative_normal(void);
union ieee32 ieee32_min_negative_normal(void);

f32 f32_nan(void);
f32 f32_inf(const u32 sign);
f32 f32_zero(const u32 sign);

f32 f32_max_positive_subnormal(void);
f32 f32_min_positive_subnormal(void);
f32 f32_max_negative_subnormal(void);
f32 f32_min_negative_subnormal(void);

f32 f32_max_positive_normal(void);
f32 f32_min_positive_normal(void);
f32 f32_max_negative_normal(void);
f32 f32_min_negative_normal(void);

f32 f32_subnormal(const u32 sign, const u32 significand);
f32 f32_nan_(const u32 sign, const u32 significand);

/**
 * (1) Run operation as if infinite precision, unbounded range
 * (2) round, if necessary
 */
f32 ieee32_add(const f32 a, const f32 b);
f32 ieee32_sub(const f32 a, const f32 b);
f32 ieee32_mul(const f32 a, const f32 b);
f32 ieee32_div(const f32 a, const f32 b);

/*************** arbitrary floating point precision lib ***************/

/**
 * fINF is a normalized arbitrary precision floating point representation.
 * Where approximations must be made, an arbitrary precision in bits should 
 * be given.
 *
 * INVARIANT: fINF Significand bit_0 starts at the end of the significand byte array,
 * 	with bit_0 being the least significant bit in the byte.
 */
fINF 		fINF_from_f32(struct arena *mem, const f32 src);
fINF 		fINF_from_ieee32(struct arena *mem, const union ieee32 src);
f32 		f32_from_fINF(struct arena *mem, const fINF src, const enum rounding_type round);
union ieee32 	ieee32_from_fINF(struct arena *mem, const fINF src, const enum rounding_type round);

void 		fINF_copy_significand_from_ieee32(fINF *dst, union ieee32 src, const u64 upper_bit, const u64 lower_bit);
enum cutoff_type fINF_cutoff_type(const fINF f, const u64 p); /* return how cutoff type for rounding, where p is the p:th bit of the signifcand and the 1:st bit of the cutoff */
u64 		fINF_significand_bytes(const fINF f);
fINF		fINF_set(struct arena *mem, const u32 sign, const i64 exponent, u64 bit_count, u8 *significand);

/* siginifcand = b1 b2 b3 .... */
/* NOTE: first bit in significand is b1! */
void fINF_print(FILE *file, const fINF src);
u64 fINF_significand_bits(const fINF f);
u8 fINF_get_significand_bit(const fINF f, const u64 i);
void fINF_set_significand_bit(const fINF f, const u64 i, const u8 bit_value);

/* return 1 if a CHECK b, else 0 */
u64 fINF_eq(const fINF a, const fINF b);
u64 fINF_le(const fINF a, const fINF b);
u64 fINF_leq(const fINF a, const fINF b);
u64 fINF_ge(const fINF a, const fINF b);
u64 fINF_geq(const fINF a, const fINF b);

/* return 1 if a.magnitude CHECK b.magnitude, else 0 */
u64 fINF_eq_magnitude(const fINF a, const fINF b);
u64 fINF_le_magnitude(const fINF a, const fINF b);
u64 fINF_leq_magnitude(const fINF a, const fINF b);
u64 fINF_ge_magnitude(const fINF a, const fINF b);
u64 fINF_geq_magnitude(const fINF a, const fINF b);

fINF fINF_add(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round);
fINF fINF_sub(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round);
fINF fINF_mul(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round);
fINF fINF_div(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round);

/* given src.significand_bit_count <= wanted_length <= 32, recast the significand as a u32 */
u32 fINF_truncate_to_u32(const fINF src, const u64 wanted_length);
u64 iNF_least_significant_zero(const fINF src);	/* return least significant bit b_i in significand that is 0, or 0 if whole significand is 1. */
void fINF_copy_significand_partition(struct arena *mem, fINF *res, const fINF src, const u64 upper_bit, const u64 lower_bit);
fINF fINF_increment_and_propagate(struct arena *mem, const fINF src);
fINF fINF_truncate_and_increment_lower_bit(struct arena *mem, const fINF src, const u64 trunc_bit); /* truncate and keep trailing zeroes then increment lowest bit */
fINF fINF_copy(struct arena *mem, const fINF src);
fINF fINF_truncate(struct arena *mem, const fINF src, const u64 trunc_bit); /* truncate b_{trunc} b_{trunc+1} ... */

#endif
