#include <string.h>
#include "math_debug_local.h"

void fINF_print(FILE *file, const fINF src)
{
	fprintf(file, "fINF: (%u, %li)\t", src.sign, src.exponent);

	fprintf(file, "(");
	for (u64 i = 0; i < fINF_significand_bits(src) - src.significand_bit_count; ++i)
	{
		fprintf(file, "0");
	}
	fprintf(file, ")");
	for (u64 i = 1; i <= src.significand_bit_count; ++i)
	{
		fprintf(file, "%u", fINF_get_significand_bit(src, i));
	}
	fprintf(file, "\n");
}

fINF fINF_copy(struct arena *mem, const fINF src)
{
	fINF dst = src;

	if (src.significand_bit_count)
	{
		fINF_copy_significand_partition(mem, &dst, src, 1, src.significand_bit_count);
	}

	return dst;
}

static fINF fINF_round_ties_to_even_cutoff(struct arena *mem, const fINF src, const u64 cutoff_bit)
{
	assert(cutoff_bit >= 1);
	fINF res;

	switch(fINF_cutoff_type(src, cutoff_bit))
	{
		/* cutoff rounds to 0 */
		case CUTOFF_0:
		{
			res = fINF_truncate(mem, src, cutoff_bit);
		} break;
	
		/* cutoff rounds to 1 */
		case CUTOFF_1:
		{
			res = fINF_truncate_and_increment_lower_bit(mem, src, cutoff_bit);
		} break;
	
		/* cutoff rounds according to tiebreaker rule */
		case CUTOFF_TIE:
		{
			if (cutoff_bit == 1 || fINF_get_significand_bit(src, cutoff_bit-1))
			{
				res = fINF_truncate_and_increment_lower_bit(mem, src, cutoff_bit);
			}
			else
			{
				res = fINF_truncate(mem, src, cutoff_bit);
			}
		}
	}

	return res;
}

static fINF fINF_round_ties_to_away_from_zero_cutoff(struct arena *mem, const fINF src, const u64 cutoff_bit)
{
	assert(cutoff_bit >= 1);
	fINF res;

	switch(fINF_cutoff_type(src, cutoff_bit))
	{
		/* cutoff rounds to 0 */
		case CUTOFF_0:
		{
			res = fINF_truncate(mem, src, cutoff_bit);
		} break;
	
		/* cutoff rounds to 1 */
		/* cutoff rounds according to tiebreaker rule */
		case CUTOFF_1:
		case CUTOFF_TIE:
		{
			res = fINF_truncate_and_increment_lower_bit(mem, src, cutoff_bit);
		} break;
	}

	return res;
}

static u8 *fINF_alloc_significand(struct arena *mem, const u64 bit_count)
{
	if (bit_count == 0) { return NULL; }

	u64 bytes = bit_count / 8;	
	if (bit_count % 8)
	{
		bytes += 1;
	}

	u8 *p = arena_push(mem, NULL, bytes);
	memset(p, 0, bytes);
	return p;
}

u64 fINF_least_significant_zero(const fINF src)
{
	u64 bit = src.significand_bit_count;
	if (bit == 0) { return 0; }

	for (; 0 < bit; --bit)
	{
		if (0 == fINF_get_significand_bit(src, bit))
		{
			break;
		}
	}

	return bit;
}

void fINF_copy_significand_partition(struct arena *mem, fINF *res, const fINF src, const u64 upper_bit, const u64 lower_bit)
{
	assert(lower_bit <= src.significand_bit_count && upper_bit <= lower_bit);
	assert(src.type == FINF_NORMAL || src.type == FINF_SUBNORMAL);

	res->significand_bit_count = lower_bit - upper_bit + 1;
	res->significand = fINF_alloc_significand(mem, res->significand_bit_count);

	for (u64 i = 1; i <= res->significand_bit_count; ++i)
	{
		fINF_set_significand_bit(*res, i, fINF_get_significand_bit(src, upper_bit + i - 1));
	}
}

fINF fINF_truncate(struct arena *mem, const fINF src, const u64 trunc_bit)
{
	assert(trunc_bit > 0);
	if (src.significand_bit_count < trunc_bit) { return fINF_copy(mem, src); }

	const u64 upper_bit = 1;
	u64 lower_bit = trunc_bit-1;
	/* truncate trailing zeroes in order to keep fINF invariant */
	for (; 0 < lower_bit; --lower_bit)
	{
		if (fINF_get_significand_bit(src, lower_bit) == 1)
		{
			break;
		}	
	}

	fINF res = 
	{
		.sign = src.sign,
		.exponent = src.exponent,
		.type = src.type,
	}; 

	if (upper_bit <= lower_bit)
	{
		fINF_copy_significand_partition(mem, &res, src, upper_bit, lower_bit);
	}
	else
	{
		if (res.type == FINF_SUBNORMAL)
		{
			res.type = FINF_ZERO;
		}
		res.significand_bit_count = 0;	
		res.significand = NULL;	
	}

	return res;
}

fINF fINF_truncate_and_increment_lower_bit(struct arena *mem, const fINF src, const u64 trunc_bit)
{
	assert(trunc_bit > 0);
	if (src.significand_bit_count < trunc_bit) { return fINF_copy(mem, src); }

	const u64 upper_bit = 1;
	u64 lower_bit = trunc_bit-1;
	
	fINF res = 
	{
		.sign = src.sign,
		.exponent = src.exponent,
		.type = src.type,
	}; 

	if (upper_bit <= lower_bit)
	{
		fINF_copy_significand_partition(mem, &res, src, upper_bit, lower_bit);
		if (fINF_get_significand_bit(res, res.significand_bit_count))
		{
			res = fINF_increment_and_propagate(mem, res);
		}
		else
		{
			fINF_set_significand_bit(res, res.significand_bit_count, 1);
		}
	}
	else
	{
		res.significand_bit_count = 0;	
		res.significand = NULL;	
	}

	return res;
}

fINF fINF_increment_and_propagate(struct arena *mem, const fINF src)
{
	assert(src.type == FINF_NORMAL || src.type == FINF_SUBNORMAL);

	fINF res = 
	{ 
		.sign = src.sign, 
		.significand_bit_count = 0,
		.significand = NULL,
		.type = src.type,
	};

	if (src.significand_bit_count == 0)
	{
		res.exponent = src.exponent + 1;	
	}
	else
	{
		u64 cutoff = fINF_least_significant_zero(src);	

		/* no overflow */
		if (cutoff)
		{
			res.exponent = src.exponent;
			fINF_copy_significand_partition(mem, &res, src, 1, cutoff);
			fINF_set_significand_bit(res, cutoff, 1);
		}
		/* overflow */
		else
		{
			res.exponent = src.exponent + 1;
		}
	}

	return res;
}

u32 fINF_truncate_significand_to_u32(const fINF src, const u64 wanted_length)
{
	assert(wanted_length <= IEEE32_SIGNIFICAND_LENGTH);

	u32 significand = 0;
	if (src.significand_bit_count == 0) { return significand; }

	u64 bit_count = (wanted_length <= src.significand_bit_count) ? wanted_length : src.significand_bit_count;

	for (u32 i = 1; i <= bit_count; ++i)
	{
		significand <<= 1;
		significand |= (u32) fINF_get_significand_bit(src, i);
	}

	return significand << (IEEE32_SIGNIFICAND_LENGTH - src.significand_bit_count);
}

/* un-normalize fINF by shifting down the significand + exponent one by given amount. */
static fINF fINF_internal_shift_down(struct arena *mem, const fINF src, const u64 shift)
{
	if (shift == 0) { return fINF_copy(mem, src); }

	fINF dst =
	{
		.sign = src.sign,
		.type = FINF_SUBNORMAL,
		.exponent = src.exponent+shift,
		.significand_bit_count = src.significand_bit_count + shift,
	};

	dst.significand = fINF_alloc_significand(mem, dst.significand_bit_count);

	const u64 src_bytes = fINF_significand_bytes(src);
	const u64 dst_bytes = fINF_significand_bytes(dst);
	for (u64 i = 1; i <= src_bytes; ++i)
	{
		dst.significand[dst_bytes-i] = src.significand[src_bytes-i];
	}

	/* set old normalized bit in significand. */
	fINF_set_significand_bit(dst, shift, 1);
	return dst;
}

/**
 * 24 == mantissa size == significand size + 1 
 */
static union ieee32 fINF_to_ieee32_round_to_nearest_tie_even(struct arena *mem, const fINF src)
{
	assert(src.type != FINF_NAN);

	union ieee32 val;

	if ((src.exponent > IEEE32_MAX_EXPONENT) ||
			   (src.exponent == IEEE32_MAX_EXPONENT
				&& src.significand_bit_count >= 24
				&& src.significand[0] == 0xff
				&& src.significand[1] == 0xff
				&& src.significand[2] == 0xff)
			)
	{
		val = ieee32_inf(src.sign);
	}
	else if (src.exponent >= IEEE32_MIN_EXPONENT)
	{
		/* no rounding needed */
		if (src.significand_bit_count <= IEEE32_SIGNIFICAND_LENGTH)
		{
			const u32 significand = fINF_truncate_significand_to_u32(src, src.significand_bit_count);
			val = ieee32_set(src.sign, src.exponent, significand);
		}
		else
		{
			struct arena record = *mem;

			fINF intermediate = fINF_round_ties_to_even_cutoff(mem, src, IEEE32_SIGNIFICAND_LENGTH+1);
			const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
			val = ieee32_set(intermediate.sign, intermediate.exponent, significand);
			*mem = record;
		}
	}
	/* subnormal */
	else if (src.exponent >= -150)
	{
		struct arena record = *mem;

		fINF intermediate = fINF_internal_shift_down(mem, src, IEEE32_MIN_EXPONENT - src.exponent);
		assert(intermediate.exponent == IEEE32_MIN_EXPONENT);
		intermediate = fINF_round_ties_to_even_cutoff(mem, intermediate, IEEE32_SIGNIFICAND_LENGTH + 1);
		const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
		val = ieee32_set(intermediate.sign, intermediate.exponent-1, significand);

		*mem = record;
	}
	else
	{
		if (src.exponent == -151 && src.significand_bit_count >= 1)
		{
			val = ieee32_set(src.sign, IEEE32_MIN_EXPONENT-1, 0x1);
		}
		else
		{
			val = ieee32_zero(src.sign);
		}
	}

	return val;
}

/**
 * 24 == mantissa size == significand size + 1 
 */
static union ieee32 fINF_to_ieee32_round_to_nearest_tie_away_from_zero(struct arena *mem, const fINF src)
{
	assert(src.type != FINF_NAN);

	union ieee32 val;

	if ((src.exponent > IEEE32_MAX_EXPONENT) ||
			   (src.exponent == IEEE32_MAX_EXPONENT
				&& src.significand_bit_count >= 24
				&& src.significand[0] == 0xff
				&& src.significand[1] == 0xff
				&& src.significand[2] == 0xff)
			)
	{
		val = ieee32_inf(src.sign);
	}
	else if (src.exponent >= IEEE32_MIN_EXPONENT)
	{
		
		/* no rounding needed */
		if (src.significand_bit_count <= IEEE32_SIGNIFICAND_LENGTH)
		{
			const u32 significand = fINF_truncate_significand_to_u32(src, src.significand_bit_count);
			val = ieee32_set(src.sign, src.exponent, significand);
		}
		else
		{
			struct arena record = *mem;
			fINF intermediate = fINF_round_ties_to_away_from_zero_cutoff(mem, src, IEEE32_SIGNIFICAND_LENGTH+1);
			const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
			val = ieee32_set(intermediate.sign, intermediate.exponent, significand);
			*mem = record;
		}
	}
	/* subnormal */
	else if (src.exponent >= -150)
	{
		struct arena record = *mem;
		fINF intermediate = fINF_internal_shift_down(mem, src, IEEE32_MIN_EXPONENT - src.exponent);
		assert(intermediate.exponent == IEEE32_MIN_EXPONENT);
		intermediate = fINF_round_ties_to_away_from_zero_cutoff(mem, intermediate, IEEE32_SIGNIFICAND_LENGTH + 1);
		const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);

		val = ieee32_set(intermediate.sign, intermediate.exponent-1, significand);

		*mem = record;

	}
	else
	{
		if (src.exponent == -151)
		{
			val = ieee32_set(src.sign, IEEE32_MIN_EXPONENT-1, 0x1);
		}
		else
		{
			val = ieee32_zero(src.sign);
		}
	}

	return val;
}

static union ieee32 fINF_to_ieee32_round_towards_plus_inf(struct arena *mem, const fINF src)
{
	assert(src.type != FINF_NAN);

	union ieee32 val;

	/* normal/inf */
	if (src.exponent >= 128)
	{
		val = (src.sign) ? ieee32_min_negative_normal() : ieee32_inf(src.sign);
	}
	else if (src.exponent >= IEEE32_MIN_EXPONENT)
	{
		
		/* no rounding needed */
		if (src.significand_bit_count <= IEEE32_SIGNIFICAND_LENGTH)
		{
			const u32 significand = fINF_truncate_significand_to_u32(src, src.significand_bit_count);
			val = ieee32_set(src.sign, src.exponent, significand);
		}
		else
		{
			struct arena record = *mem;
			fINF intermediate = (src.sign) 
				? fINF_truncate(mem, src, IEEE32_SIGNIFICAND_LENGTH+1)
				: fINF_truncate_and_increment_lower_bit(mem, src, IEEE32_SIGNIFICAND_LENGTH+1);
			if (intermediate.exponent >= 128)
			{
				val = (src.sign) ? ieee32_min_negative_normal() : ieee32_inf(src.sign);
			}
			else
			{
				const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
				val = ieee32_set(intermediate.sign, intermediate.exponent, significand);
			}
			*mem = record;
		}
	}
	/* subnormal */
	else if (src.exponent >= -150)
	{
		struct arena record = *mem;
		fINF intermediate = fINF_internal_shift_down(mem, src, IEEE32_MIN_EXPONENT - src.exponent);
		if (intermediate.significand_bit_count > IEEE32_SIGNIFICAND_LENGTH)
		{
			intermediate = (intermediate.sign)
				? fINF_truncate(mem, intermediate, IEEE32_SIGNIFICAND_LENGTH+1)
				: fINF_truncate_and_increment_lower_bit(mem, intermediate, IEEE32_SIGNIFICAND_LENGTH+1); 
		}
		const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
		val = ieee32_set(intermediate.sign, intermediate.exponent-1, significand);
		*mem = record;
	}
	else
	{
		val = (src.sign) ? ieee32_zero(src.sign) : ieee32_min_positive_subnormal();
	}

	return val;
}

static union ieee32 fINF_to_ieee32_round_towards_minus_inf(struct arena *mem, const fINF src)
{
	assert(src.type != FINF_NAN);

	union ieee32 val;

	/* normal/inf */
	if (src.exponent >= 128)
	{
		val = (src.sign) ? ieee32_inf(src.sign) : ieee32_max_positive_normal();
	}
	else if (src.exponent >= IEEE32_MIN_EXPONENT)
	{
		/* no rounding needed */
		if (src.significand_bit_count <= IEEE32_SIGNIFICAND_LENGTH)
		{
			const u32 significand = fINF_truncate_significand_to_u32(src, src.significand_bit_count);
			val = ieee32_set(src.sign, src.exponent, significand);
		}
		else
		{
			struct arena record = *mem;
			fINF intermediate = (!src.sign) 
				? fINF_truncate(mem, src, IEEE32_SIGNIFICAND_LENGTH+1)
				: fINF_truncate_and_increment_lower_bit(mem, src, IEEE32_SIGNIFICAND_LENGTH+1);

			if (intermediate.exponent >= 128)
			{
				val = (intermediate.sign) ? ieee32_inf(intermediate.sign) : ieee32_max_positive_normal();
			}
			else
			{
				const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
				val = ieee32_set(intermediate.sign, intermediate.exponent, significand);
			}
			*mem = record;
		}
	}
	/* subnormal */
	else if (src.exponent >= -150)
	{
		struct arena record = *mem;
		fINF intermediate = fINF_internal_shift_down(mem, src, IEEE32_MIN_EXPONENT - src.exponent);
		if (intermediate.significand_bit_count > IEEE32_SIGNIFICAND_LENGTH)
		{
			intermediate = (intermediate.sign == 0)
				? fINF_truncate(mem, intermediate, IEEE32_SIGNIFICAND_LENGTH+1)
				: fINF_truncate_and_increment_lower_bit(mem, intermediate, IEEE32_SIGNIFICAND_LENGTH+1); 
		}
		const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
		val = ieee32_set(intermediate.sign, intermediate.exponent-1, significand);
		*mem = record;


	}
	else
	{
		val = (src.sign) ? ieee32_max_negative_subnormal() : ieee32_zero(src.sign);
	}

	return val;
}

static union ieee32 fINF_to_ieee32_round_towards_zero(struct arena *mem, const fINF src)
{
	assert(src.type != FINF_NAN);

	union ieee32 val;

	/* normal/inf */
	if (src.exponent >= 128)
	{
		val = (src.sign) ? ieee32_min_negative_normal() : ieee32_max_positive_normal();
	}
	else if (src.exponent >= IEEE32_MIN_EXPONENT)
	{
		
		/* no rounding needed */
		if (src.significand_bit_count <= IEEE32_SIGNIFICAND_LENGTH)
		{
			const u32 significand = fINF_truncate_significand_to_u32(src, src.significand_bit_count);
			val = ieee32_set(src.sign, src.exponent, significand);
		}
		else
		{
			struct arena record = *mem;
			fINF intermediate = fINF_truncate(mem, src, IEEE32_SIGNIFICAND_LENGTH+1);
			if (intermediate.exponent >= 128)
			{
				val = (intermediate.sign) ? ieee32_min_negative_normal() : ieee32_max_positive_normal();
			}
			else
			{
				const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
				val = ieee32_set(intermediate.sign, intermediate.exponent, significand);
			}
			*mem = record;
		}
	}
	/* subnormal */
	else if (src.exponent >= -150)
	{
		struct arena record = *mem;
		fINF intermediate = fINF_internal_shift_down(mem, src, IEEE32_MIN_EXPONENT - src.exponent);
		if (intermediate.significand_bit_count > IEEE32_SIGNIFICAND_LENGTH)
		{
			intermediate = fINF_truncate(mem, intermediate, IEEE32_SIGNIFICAND_LENGTH+1);
		}
		const u32 significand = fINF_truncate_significand_to_u32(intermediate, IEEE32_SIGNIFICAND_LENGTH);
		val = ieee32_set(intermediate.sign, intermediate.exponent-1, significand);
		*mem = record;

	}
	else
	{
		val = ieee32_zero(src.sign);
	}

	return val;
}

union ieee32 (*fINF_to_ieee32_rounding_method[5])(struct arena *mem, const fINF) =
{
	fINF_to_ieee32_round_to_nearest_tie_even,
	fINF_to_ieee32_round_to_nearest_tie_away_from_zero,	
	fINF_to_ieee32_round_towards_plus_inf,
	fINF_to_ieee32_round_towards_minus_inf,
	fINF_to_ieee32_round_towards_zero,
};

u64 fINF_significand_bytes(const fINF f)
{
	u64 bytes = f.significand_bit_count / 8;
	if (f.significand_bit_count % 8)
	{
		bytes += 1;
	}

	return bytes;
}

u64 fINF_significand_bits(const fINF f)
{
	return 8UL*fINF_significand_bytes(f);
}

u8 fINF_get_significand_bit(const fINF f, const u64 i)
{
	if (f.significand_bit_count < i) { return 0; }

	const u64 bit_in_array = fINF_significand_bits(f) - 1 - (f.significand_bit_count - i);
	const u8 b = bit_in_array / 8;
	const u8 bit_shift =  7 - (bit_in_array % 8);
	return (f.significand[b] >> bit_shift) & 0x1;
}

void fINF_set_significand_bit(const fINF f, const u64 i, const u8 bit_value)
{
	assert(i <= f.significand_bit_count);
	assert(bit_value <= 1);

	const u64 bit_in_array = fINF_significand_bits(f) - 1 - (f.significand_bit_count - i);
	const u8 b = bit_in_array / 8;
	const u8 bit_shift =  7 - (bit_in_array % 8);
	const u8 clear_mask = 0xff ^ (0x1 << bit_shift);

	f.significand[b] = (f.significand[b] & clear_mask) | (bit_value << bit_shift);
}

fINF fINF_from_f32(struct arena *mem, const f32 src)
{
	union ieee32 val = { .f = src };
	return fINF_from_ieee32(mem, val);
}

f32 f32_from_fINF(struct arena *mem, const fINF src, const enum rounding_type round)
{
	union ieee32 val = ieee32_from_fINF(mem, src, round);
	return val.f;
}

fINF fINF_from_ieee32(struct arena *mem, union ieee32 src)
{
	fINF val = { .sign = ieee32_sign(src), };

	switch (ieee32_classify(src.f))
	{
		case IEEE_NAN:
		{
			val.type = FINF_NAN;
		} break;

                case IEEE_INF:
		{
			val.type = FINF_INF;
		} break;

		case IEEE_ZERO:
		{
			val.type = FINF_ZERO;
		} break;

		case IEEE_NORMAL:
		{
			val.type = FINF_NORMAL;
			val.exponent = (i32) ieee32_exponent(src) - IEEE32_BIAS;
			val.significand_bit_count = ieee32_significand_length_without_trailing_zeroes(src);
			val.significand = fINF_alloc_significand(mem, val.significand_bit_count);
			if (val.significand)
			{
				fINF_copy_significand_from_ieee32(&val, src, 1, val.significand_bit_count);
			}
		} break;

		case IEEE_SUBNORMAL:
		{
			/* exponent = -127 - (1 + sum of leading zeroes)*/
			val.type = FINF_NORMAL;
			const u64 leading_zero_count = ieee32_leading_zero_count(src);
			const u64 trailing_zero_count = IEEE32_SIGNIFICAND_LENGTH - ieee32_significand_length_without_trailing_zeroes(src);
			val.exponent = -IEEE32_BIAS - (leading_zero_count);
			val.significand_bit_count = IEEE32_SIGNIFICAND_LENGTH - 1 - leading_zero_count - trailing_zero_count;
			val.significand = fINF_alloc_significand(mem, val.significand_bit_count);
			if (val.significand)
			{
				fINF_copy_significand_from_ieee32(&val, src, leading_zero_count + 2, IEEE32_SIGNIFICAND_LENGTH - trailing_zero_count);
			}
		} break;
	}

	return val;
}

void fINF_copy_significand_from_ieee32(fINF *dst, union ieee32 src, const u64 upper_bit, const u64 lower_bit)
{
	assert((lower_bit + 1 - upper_bit) == dst->significand_bit_count);
	//TODO: Rewrite, we expect something else than what we are doing?. 
	
	if (ieee32_test_normal(src.f))
	{
		const u32 significand = src.bits & IEEE32_SIGNIFICAND_MASK;
		for (u64 i = 1; i <= dst->significand_bit_count; ++i)
		{
			const u8 bit = (significand >> (IEEE32_SIGNIFICAND_LENGTH - upper_bit - (i-1))) & 0x1;
			fINF_set_significand_bit(*dst, i, bit);
		}
	}
	else
	{
		const u64 total_bytes = fINF_significand_bytes(*dst);
		const u32 mask = IEEE32_SIGNIFICAND_MASK >> (upper_bit - 1);
		const u32 significand = ((src.bits & mask) >> (IEEE32_SIGNIFICAND_LENGTH - lower_bit));
		for (u64 i = 0; i < total_bytes; ++i)
		{
			dst->significand[total_bytes-i-1] = (u8) (significand >> (8*i));
		}
	}
	
	assert(fINF_get_significand_bit(*dst, dst->significand_bit_count) == 1);
}

union ieee32 ieee32_from_fINF(struct arena *mem, const fINF src, const enum rounding_type round)
{
	union ieee32 val;

	switch (src.type)
	{
		case FINF_NAN: 
		{
			val = ieee32_nan();
		} break;
		
       		case FINF_INF:
		{
			val = ieee32_inf(src.sign);					
		} break;

		case FINF_ZERO:
		{
			val = ieee32_zero(src.sign);					
		} break;

		case FINF_NORMAL:
		{
			val = fINF_to_ieee32_rounding_method[round](mem, src);
		} break;
	}

	return val;
}

enum cutoff_type fINF_cutoff_type(const fINF f, const u64 p)
{
	if (f.significand_bit_count < p) { return CUTOFF_0; }

	enum cutoff_type type;

	/* possible tie */
	if (fINF_get_significand_bit(f, p) == 1)
	{
		type = CUTOFF_TIE;
		for (u64 i = p+1; i <= f.significand_bit_count; ++i)
		{
			if (fINF_get_significand_bit(f, i) == 1)
			{
				type = CUTOFF_1;
				break;
			}
		}
	}
	else
	{
		type = CUTOFF_0;
	}

	return type;
}

fINF fINF_set(struct arena *mem, const u32 sign, const i64 exponent, u64 bit_count, u8 *significand)
{
	fINF val = 
	{ 
		.sign = sign, 
		.exponent = exponent,
		.significand_bit_count = bit_count,
		.type = FINF_NORMAL,
	};

	val.significand = fINF_alloc_significand(mem, val.significand_bit_count);
	if (val.significand)
	{
		memcpy(val.significand, significand, fINF_significand_bytes(val));
	}

	return val;
}

u64 fINF_eq(const fINF a, const fINF b)
{
	//TODO: Must fix infinities, nan...;
	return 0;
}

u64 fINF_le(const fINF a, const fINF b)
{
	//TODO: Must fix infinities, nan...;
	return 0;
}

u64 fINF_leq(const fINF a, const fINF b)
{
	//TODO: Must fix infinities, nan...;
	return 0;
}

u64 fINF_ge(const fINF a, const fINF b)
{
	//TODO: Must fix infinities, nan...;
	return 0;
}

u64 fINF_geq(const fINF a, const fINF b)
{
	//TODO: Must fix infinities, nan...;
	return 0;
}

u64 fINF_eq_magnitude(const fINF a, const fINF b)
{
	if (a.exponent != b.exponent) { return 0; }
	if (a.significand_bit_count != b.significand_bit_count) { return 0; }

	for (u64 i = 1; i <= a.significand_bit_count; ++i)
	{
		const u8 b1 = fINF_get_significand_bit(a, i);
		const u8 b2 = fINF_get_significand_bit(b, i);
		if (b1 != b2)
		{
			return 0;
		}
	}

	return 1;
}

u64 fINF_le_magnitude(const fINF a, const fINF b)
{
	if (a.exponent > b.exponent) { return 0; }
	if (a.exponent < b.exponent) { return 1; }

	const u64 len = (a.significand_bit_count <= b.significand_bit_count) ? a.significand_bit_count : b.significand_bit_count;
	for (u64 i = 1; i <= len; ++i)
	{
		const u8 b1 = fINF_get_significand_bit(a, i);
		const u8 b2 = fINF_get_significand_bit(b, i);
		if (b1 != b2)
		{
			return (b1) ? 0 : 1;
		}
	}

	return (a.significand_bit_count < b.significand_bit_count) ? 1 : 0;
}

u64 fINF_leq_magnitude(const fINF a, const fINF b)
{
	u64 val = 0;

	if (fINF_eq_magnitude(a, b) || fINF_le_magnitude(a,b))
	{
		val = 1;
	}	

	return val;
}

u64 fINF_ge_magnitude(const fINF a, const fINF b)
{
	return fINF_le_magnitude(b, a);
}

u64 fINF_geq_magnitude(const fINF a, const fINF b)
{
	return fINF_le_magnitude(b, a) | fINF_eq_magnitude(a,b);
}

static fINF fINF_internal_add_nan(const fINF a, const fINF b)
{
	assert(a.type == FINF_NAN || b.type == FINF_NAN);

	fINF res = { .type = FINF_NAN };

	return res;
}

static fINF fINF_internal_add_inf(const fINF a, const fINF b)
{
	assert(a.type != FINF_NAN && b.type != FINF_NAN);
	assert(a.type == FINF_INF || b.type == FINF_INF);

	fINF res;

	if (a.type == FINF_INF)
	{
		if (b.type == FINF_INF && a.sign != b.sign)
		{
			res.type = FINF_NAN;
		}
		else
		{
			res = a;
		}
	}
	else
	{
		res = b;
	}

	return res;
}

static fINF fINF_internal_add_zero(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round)
{
	assert(a.type != FINF_NAN && b.type != FINF_NAN);
	assert(a.type != FINF_INF && b.type != FINF_INF);
	assert(a.type == FINF_ZERO || b.type == FINF_ZERO);

	fINF res;

	if (a.type == FINF_ZERO)
	{
		if (b.type == FINF_ZERO)
		{
			if (a.sign == b.sign)
			{
				res = a;
			}
			else
			{
				res.type = FINF_ZERO;
				res.sign = (round == ROUNDING_TOWARDS_MINUS_INF) ? 1 : 0;
			}
		}
		else
		{
			res = fINF_copy(mem, b);
		}
	}
	else
	{
		res = fINF_copy(mem, a);
	}

	return res;
}

static u64 fINF_leading_zeroes(const fINF src)
{
	u64 count = 0;

	for (u64 i = 1; i <= src.significand_bit_count; ++i, ++count)
	{
		if (fINF_get_significand_bit(src, i) == 1)
		{
			break;
		}
	}

	return count;
}

static fINF fINF_internal_renormalize(struct arena *mem, const fINF src)
{
	fINF res;

	if (src.type == FINF_SUBNORMAL)
	{
		const u64 leading_zero_count = fINF_leading_zeroes(src);
		
		res.sign = src.sign;
		res.exponent = src.exponent - leading_zero_count - 1;
		res.type = FINF_NORMAL;
		if (src.significand_bit_count > leading_zero_count + 1)
		{
			fINF_copy_significand_partition(mem, &res, src, leading_zero_count + 2, src.significand_bit_count);
		}
		else
		{
			res.significand_bit_count = 0;
			res.significand = NULL;
		}
	}
	else
	{
		assert(src.type == FINF_NORMAL);
		res = fINF_copy(mem, src);
	}

	return res;
}

static fINF fINF_internal_add(struct arena *mem, const fINF l_mag, const fINF s_mag)
{
	assert(l_mag.type != FINF_NAN &&  s_mag.type != FINF_NAN);
	assert(l_mag.type != FINF_INF &&  s_mag.type != FINF_INF);
	assert(l_mag.type != FINF_ZERO && s_mag.type != FINF_ZERO);

	assert(l_mag.sign == s_mag.sign && fINF_leq_magnitude(s_mag, l_mag));

	const fINF l = fINF_internal_shift_down(mem, l_mag, 1);
	const fINF s = fINF_internal_shift_down(mem, s_mag, l_mag.exponent - s_mag.exponent + 1);

	assert(l.exponent == s.exponent);

	const u64 bit_count = (l.significand_bit_count <= s.significand_bit_count) 
		? s.significand_bit_count
		: l.significand_bit_count;
	fINF res =
	{
		.sign = l.sign,
		.type = FINF_SUBNORMAL,
		.exponent = l.exponent,
		.significand_bit_count = bit_count,
	};

	res.significand = fINF_alloc_significand(mem, bit_count);

	u64 carry = 0;
	for (u64 i = bit_count; 0 < i; --i)
	{
		const u8 bit_sum = fINF_get_significand_bit(l, i) +  fINF_get_significand_bit(s, i) + carry;
		fINF_set_significand_bit(res, i, bit_sum % 2);
		carry = bit_sum >> 1;
	
	}
	
	/* truncate trailing zeroes */
	if (fINF_get_significand_bit(res, bit_count) == 0)
	{
		res = fINF_truncate(mem, res, bit_count);
	}

	if (!carry)
	{
		res = fINF_internal_renormalize(mem, res);
	}
	res.type = FINF_NORMAL;

	assert(res.significand_bit_count == 0 || fINF_get_significand_bit(res, res.significand_bit_count) == 1);
	return res;
}

static fINF fINF_internal_sub(struct arena *mem, const fINF l_mag, const fINF s_mag)
{
	assert(l_mag.sign == s_mag.sign && fINF_leq_magnitude(s_mag, l_mag));

	const fINF l = fINF_internal_shift_down(mem, l_mag, 1);
	const fINF s = fINF_internal_shift_down(mem, s_mag, l_mag.exponent - s_mag.exponent + 1);

	assert(l.exponent == s.exponent);

	const u64 bit_count = (l.significand_bit_count <= s.significand_bit_count) 
		? s.significand_bit_count
		: l.significand_bit_count;
	
	fINF res =
	{
		.sign = l.sign,
		.type = FINF_SUBNORMAL,
		.exponent = l.exponent,
		.significand_bit_count = bit_count,
	};

	res.significand = fINF_alloc_significand(mem, bit_count);

	u64 borrow = 0;
	for (u64 i = 1; i <= bit_count; ++i)
	{
		const u8 bit_1 = fINF_get_significand_bit(l, i);
		const u8 bit_2 = fINF_get_significand_bit(s, i);

		if (bit_1 == 1 && bit_2 == 0)
		{
			fINF_set_significand_bit(res, i, 1);
			borrow = i;
		}
		else if (bit_1 == 0 && bit_2 == 1)
		{
			fINF_set_significand_bit(res, borrow, 0);
			for (u64 j = borrow+1; j <= i; ++j)
			{
				fINF_set_significand_bit(res, j, 1);
			}
			borrow = i;
		}
		else
		{
			fINF_set_significand_bit(res, i, 0);
		}
	}
	
	if (fINF_get_significand_bit(res, bit_count) == 0)
	{
		res = fINF_truncate(mem, res, bit_count);
	}

	if (res.type == FINF_SUBNORMAL)
	{
		res = fINF_internal_renormalize(mem, res);
	}


	assert(res.significand_bit_count == 0 || fINF_get_significand_bit(res, res.significand_bit_count) == 1);
	return res;
}

fINF fINF_add(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round)
{
	fINF res; 

	if (a.type == FINF_NAN || b.type == FINF_NAN)
	{
		res = fINF_internal_add_nan(a, b);
	}
	else if (a.type == FINF_INF || b.type == FINF_INF)
	{
		res = fINF_internal_add_inf(a, b);
	}
	else if (a.type == FINF_ZERO || b.type == FINF_ZERO)
	{
		/* important to keep left-right ordering, as zero-zero addition is not commutative */
		res = fINF_internal_add_zero(mem, a, b, round);
	}
	else
	{
		u64 not_equal = 0;
		fINF large_magnitude, small_magnitude;
		if ((not_equal = fINF_ge_magnitude(a, b)))
		{
			large_magnitude = a;
			small_magnitude = b;
			small_magnitude.sign = large_magnitude.sign;
		}
		else
		{
			not_equal = fINF_le_magnitude(a, b);
			large_magnitude = b;
			small_magnitude = a;
			small_magnitude.sign = large_magnitude.sign;
		}

		if (a.sign == b.sign)
		{
			res = fINF_internal_add(mem, large_magnitude, small_magnitude);
		}		
		else
		{
			if (not_equal)
			{
				res = fINF_internal_sub(mem, large_magnitude, small_magnitude);
			}
			else
			{
				res.type = FINF_ZERO;
				res.sign = (round == ROUNDING_TOWARDS_MINUS_INF) ? 1 : 0;
			}
		}
	}

	return res;
}

fINF fINF_sub(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round)
{
	fINF res = fINF_copy(mem, b); 
	res.sign = 1 - res.sign;

	res = fINF_add(mem, a, res, round);

	return res;
}

static fINF fINF_internal_mul_nan(const fINF a, const fINF b)
{
	assert(a.type == FINF_NAN || b.type == FINF_NAN);

	fINF res = { .type = FINF_NAN };

	return res;
}

static fINF fINF_internal_mul_inf(const fINF a, const fINF b)
{
	assert(a.type != FINF_NAN && b.type != FINF_NAN);
	assert(a.type == FINF_INF || b.type == FINF_INF);

	fINF res;

	if (a.type == FINF_ZERO || b.type == FINF_ZERO)
	{
		res.type = FINF_NAN;
	}
	else
	{
		res.type = FINF_INF;
		res.sign = a.sign ^ b.sign;
	}

	return res;
}

static fINF fINF_internal_mul_zero(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round)
{
	assert(a.type != FINF_NAN && b.type != FINF_NAN);
	assert(a.type != FINF_INF && b.type != FINF_INF);
	assert(a.type == FINF_ZERO || b.type == FINF_ZERO);

	fINF res =
	{
		.type = FINF_ZERO,
		.sign = a.sign ^ b.sign,
	};

	return res;
}

static fINF fINF_internal_mul(struct arena *mem, const fINF a, const fINF b)
{
	assert(a.type != FINF_NAN &&  b.type != FINF_NAN);
	assert(a.type != FINF_INF &&  b.type != FINF_INF);
	assert(a.type != FINF_ZERO && b.type != FINF_ZERO);
	assert(a.exponent >= b.exponent);

	const fINF l = fINF_internal_shift_down(mem, a, 1);
	const fINF s = fINF_internal_shift_down(mem, b, a.exponent - b.exponent + 1);

	assert(l.exponent == s.exponent);

	const u64 bit_count = l.significand_bit_count + s.significand_bit_count;
	fINF res =
	{
		.sign = s.sign ^ l.sign,
		.type = FINF_SUBNORMAL,
		.exponent = l.exponent + s.exponent,
		.significand_bit_count = bit_count,
	};

	res.significand = fINF_alloc_significand(mem, res.significand_bit_count);

	for (u64 i = l.significand_bit_count; 0 < i; --i)
	{
		/* multiply b with bit */
		if (fINF_get_significand_bit(l, i) == 1)
		{
			u8 carry = 0;
			for (u64 j = s.significand_bit_count; 0 < j; --j)
			{
				const u8 bit_sum = fINF_get_significand_bit(res, i+j) +  fINF_get_significand_bit(s, j) + carry;
				fINF_set_significand_bit(res, i+j, bit_sum % 2);
				carry = bit_sum >> 1;
			}

			if (carry)
			{
				assert(fINF_get_significand_bit(res, i) == 0);
				fINF_set_significand_bit(res, i, 1);
			}
		}
	}

	res = fINF_internal_renormalize(mem, res);
	assert(res.significand_bit_count == 0 || fINF_get_significand_bit(res, res.significand_bit_count) == 1);

	return res;
}

fINF fINF_mul(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round)
{
	fINF res; 

	if (a.type == FINF_NAN || b.type == FINF_NAN)
	{
		res = fINF_internal_mul_nan(a, b);
	}
	else if (a.type == FINF_INF || b.type == FINF_INF)
	{
		res = fINF_internal_mul_inf(a, b);
	}
	else if (a.type == FINF_ZERO || b.type == FINF_ZERO)
	{
		/* important to keep left-right ordering, as zero-zero addition is not commutative */
		res = fINF_internal_mul_zero(mem, a, b, round);
	}
	else
	{
		res = (a.exponent >= b.exponent) ? fINF_internal_mul(mem, a, b) : fINF_internal_mul(mem, b, a);
	}

	return res;
}

fINF fINF_div(struct arena *mem, const fINF a, const fINF b, const enum rounding_type round)
{
	fINF res; 

	res.type = FINF_ZERO;	

	return res;
}
