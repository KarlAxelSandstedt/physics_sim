#ifndef __MATH_DEBUG_COMMON_H__
#define __MATH_DEBUG_COMMON_H__

enum rounding_type
{
	ROUNDING_NEAREST_TIES_EVEN,
	ROUNDING_NEAREST_TIES_AWAY_FROM_ZERO,
	ROUNDING_TOWARDS_PLUS_INF,
	ROUNDING_TOWARDS_MINUS_INF,
	ROUNDING_TOWARDS_ZERO,
	ROUNDING_COUNT,
};

enum ieee_type
{
	IEEE_NAN,
	IEEE_INF,
	IEEE_ZERO,
	IEEE_NORMAL,
	IEEE_SUBNORMAL,
};

union ieee32
{
	f32 f;
	u32 bits;		/* SIGN_BIT(1) | EXPONENT(8) | SIGNIFICAND(23) */
};


enum fINF_type
{
	FINF_NAN,
	FINF_INF,
	FINF_ZERO,
	FINF_NORMAL,
	FINF_SUBNORMAL,	/* Only for internal use when we do arithmetic operations */
};

/* NOTE: significand does not include hidden normalization bit! */
typedef struct fINF fINF;
struct fINF
{
	 i64 exponent;
	 u64 significand_bit_count;
	 u8 *significand;			/*  PAD(n) | SIGNIFICAND(bit_count) */
	 u32 sign;			
	 enum fINF_type type;
};

/*
 * NOTE: The cutoff bit is the most significant bit determining the rounding, ie if we want 23 bit precision in 
 *  	 significant, the cutoff bit is bit 24.
 */
enum cutoff_type
{
	CUTOFF_0,
	CUTOFF_1,
	CUTOFF_TIE,
};

#endif
