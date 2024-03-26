#include "math_debug_local.h"

void ieee32_bits_print(FILE *file, const u32 bits)
{
	const union ieee32 val = { .bits = bits };
	ieee32_print(file, val);
}

void ieee32_print(FILE *file, const union ieee32 val)
{
	i32 i = 31;
	fprintf(file, "ieee32:\t");
	fprintf(file, "%u", ieee32_bit(val, i--));
	fprintf(file, " ");
	for (; 22 < i; --i) { fprintf(file, "%u", ieee32_bit(val, i)); }
	fprintf(file, " ");
	for (; 0 <= i; --i) { fprintf(file, "%u", ieee32_bit(val, i)); }
	fprintf(file, "\n");
}

u32 ieee32_bit(const union ieee32 f, const u32 bit)
{
	assert(bit <= 31);

	return (f.bits & (1 << bit)) >> bit;
}

u64 ieee32_leading_zero_count(const union ieee32 f)
{
	u64 zeroes = 0;

	for (i64 i = IEEE32_SIGNIFICAND_LENGTH-1; 0 <= i; --i, ++zeroes)
	{
		if (ieee32_bit(f, i))
		{
			break;
		}
	}

	return zeroes;
}

union ieee32 ieee32_set(const u32 sign, const i32 exponent, const u32 significand)
{
	assert(-127 <= exponent && exponent <= 128);
	union ieee32 val = 
	{ 
		.bits =   (sign << (IEEE32_SIGNIFICAND_LENGTH + IEEE32_EXPONENT_LENGTH))
			| ((u32) (exponent + IEEE32_BIAS) << IEEE32_SIGNIFICAND_LENGTH) 
			| (significand & IEEE32_SIGNIFICAND_MASK)
	};

	return val;
}

u32 ieee32_exponent(const union ieee32 f)
{
	return (f.bits & IEEE32_EXPONENT_MASK) >> IEEE32_SIGNIFICAND_LENGTH;
}

u32 ieee32_significand(const union ieee32 f)
{
	return f.bits & IEEE32_SIGNIFICAND_MASK;
}

u32 ieee32_significand_length_without_trailing_zeroes(union ieee32 src)
{
	u32 len = 23;
	for (; 0 < len; --len)
	{
		const u32 i = 23 - len;
		const u32 bit_i = (src.bits >> i) & 0x1;
		if (bit_i)
		{
			break;
		}
	}
	return len;
}

u32 f32_sign(const f32 f)
{
	union ieee32 val = { .f = f };
	return ieee32_sign(val);
}

u32 ieee32_sign(const union ieee32 f)
{
	return (f.bits & IEEE32_SIGN_MASK) >> (IEEE32_EXPONENT_LENGTH + IEEE32_SIGNIFICAND_LENGTH);
}

u32 ieee32_test_nan(const f32 f)
{
	union ieee32 val = { .f = f };	

	const u32 is_nan = (((val.bits & IEEE32_EXPONENT_MASK) == IEEE32_EXPONENT_MASK) 
			 && ((val.bits & IEEE32_SIGNIFICAND_MASK) > 0))
		? 1
		: 0;

	return is_nan;
}

u32 ieee32_test_positive_inf(const f32 f)
{
	union ieee32 val = { .f = f };

	const u32 is_positive_inf = (val.bits == IEEE32_EXPONENT_MASK) ? 1 : 0;

	return is_positive_inf;
}

u32 ieee32_test_negative_inf(const f32 f)
{
	union ieee32 val = { .f = f };

	const u32 is_negative_inf = (val.bits == (IEEE32_SIGN_MASK | IEEE32_EXPONENT_MASK)) ? 1 : 0;

	return is_negative_inf;
}

u32 ieee32_test_normal(const f32 f)
{
	union ieee32 val = { .f = f };

	const i32 s = (i32) ((val.bits & IEEE32_EXPONENT_MASK) >> IEEE32_SIGNIFICAND_LENGTH) - IEEE32_BIAS;

	return (IEEE32_MIN_EXPONENT <= s && s <= IEEE32_MAX_EXPONENT) ? 1 : 0;
}

u32 ieee32_test_subnormal(const f32 f)
{
	union ieee32 val = { .f = f };

	return (((val.bits & IEEE32_EXPONENT_MASK) == 0) && ((val.bits & IEEE32_SIGNIFICAND_MASK) > 0))
		? 1
		: 0;
}

u32 ieee32_test_positive_zero(const f32 f)
{
	union ieee32 val = { .f = f };

	const u32 is_positive_zero = (val.bits == 0) ? 1 : 0;

	return is_positive_zero;
}

u32 ieee32_test_negative_zero(const f32 f)
{
	union ieee32 val = { .f = f };

	const u32 is_negative_zero = (val.bits == IEEE32_SIGN_MASK) ? 1 : 0;

	return is_negative_zero;
}


union ieee32 ieee32_nan(void)
{
	union ieee32 val = { .bits = IEEE32_EXPONENT_MASK | IEEE32_SIGNIFICAND_MASK };

	return val;
}

union ieee32 ieee32_inf(const u32 sign)
{
	union ieee32 val = { .bits = IEEE32_EXPONENT_MASK };

	val.bits |= (sign) ? IEEE32_SIGN_MASK : 0;

	return val;
}

union ieee32 ieee32_zero(const u32 sign)
{
	union ieee32 val;

	val.bits = (sign) ? IEEE32_SIGN_MASK : 0;

	return val;
}

f32 f32_inf(const u32 sign)
{
	union ieee32 val = { .bits = IEEE32_EXPONENT_MASK };

	val.bits |= (sign) ? IEEE32_SIGN_MASK : 0;

	return val.f;
}


f32 f32_zero(const u32 sign)
{
	union ieee32 val;

	val.bits = (sign) ? IEEE32_SIGN_MASK : 0;

	return val.f;
}

f32 f32_nan(void)
{
	union ieee32 val = { .bits = IEEE32_EXPONENT_MASK | IEEE32_SIGNIFICAND_MASK };

	return val.f;
}


f32 f32_nan_(const u32 sign, const u32 significand)
{
	union ieee32 val = { .bits = sign | IEEE32_EXPONENT_MASK | significand };

	return val.f;
}

union ieee32 ieee32_max_positive_subnormal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGNIFICAND_MASK };

	return val;
}

union ieee32 ieee32_min_positive_subnormal(void)
{
	union ieee32 val = { .bits = 0x1 };

	return val;
}

union ieee32 ieee32_max_negative_subnormal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | 0x1 };

	return val;
}

union ieee32 ieee32_min_negative_subnormal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | IEEE32_SIGNIFICAND_MASK };

	return val;
}

union ieee32 ieee32_max_positive_normal(void)
{
	union ieee32 val = { .bits = (IEEE32_EXPONENT_MASK & 0x7f000000) | IEEE32_SIGNIFICAND_MASK };

	return val;
}

union ieee32 ieee32_min_positive_normal(void)
{
	union ieee32 val = { .bits = (IEEE32_EXPONENT_MASK & 0x00800000) };

	return val;
}

union ieee32 ieee32_max_negative_normal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | (IEEE32_EXPONENT_MASK & 0x00800000) };

	return val;
}

union ieee32 ieee32_min_negative_normal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | (IEEE32_EXPONENT_MASK & 0x7f000000) | IEEE32_SIGNIFICAND_MASK};

	return val;
};



f32 f32_max_positive_subnormal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGNIFICAND_MASK };

	return val.f;
}

f32 f32_min_positive_subnormal(void)
{
	union ieee32 val = { .bits = 0x1 };

	return val.f;
}

f32 f32_max_negative_subnormal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | 0x1 };

	return val.f;
}

f32 f32_min_negative_subnormal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | IEEE32_SIGNIFICAND_MASK };

	return val.f;
}

f32 f32_max_positive_normal(void)
{
	union ieee32 val = { .bits = (IEEE32_EXPONENT_MASK & 0x7f000000) | IEEE32_SIGNIFICAND_MASK };

	return val.f;
}

f32 f32_min_positive_normal(void)
{
	union ieee32 val = { .bits = (IEEE32_EXPONENT_MASK & 0x00800000) };

	return val.f;
}

f32 f32_max_negative_normal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | (IEEE32_EXPONENT_MASK & 0x00800000) };

	return val.f;
}

f32 f32_min_negative_normal(void)
{
	union ieee32 val = { .bits = IEEE32_SIGN_MASK | (IEEE32_EXPONENT_MASK & 0x7f000000) | IEEE32_SIGNIFICAND_MASK };

	return val.f;
}

f32 ieee32_add(const f32 a, const f32 b)
{
	union ieee32 a_val = { .f = a };
	union ieee32 b_val = { .f = b };

	return 0.0f;
}

f32 ieee32_sub(const f32 a, const f32 b)
{
	union ieee32 a_val = { .f = a };
	union ieee32 b_val = { .f = b };

	return 0.0f;
}

f32 ieee32_mul(const f32 a, const f32 b)
{
	union ieee32 a_val = { .f = a };
	union ieee32 b_val = { .f = b };

	return 0.0f;
}

f32 ieee32_div(const f32 a, const f32 b)
{
	union ieee32 a_val = { .f = a };
	union ieee32 b_val = { .f = b };

	return 0.0f;
}

enum ieee_type ieee32_classify(const f32 f)
{
	if (ieee32_test_nan(f)) return IEEE_NAN;	
	else if (ieee32_test_positive_inf(f) || ieee32_test_negative_inf(f)) return IEEE_INF;	
	else if (ieee32_test_positive_zero(f) || ieee32_test_negative_zero(f)) return IEEE_ZERO;	
	else if (ieee32_test_subnormal(f)) return IEEE_SUBNORMAL;	
	else return IEEE_NORMAL;	
}
