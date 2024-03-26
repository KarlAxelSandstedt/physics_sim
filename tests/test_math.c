#include <stdio.h>
#include <math.h>
#include <float.h>
#include <fenv.h>

#include "test_common.h"
#include "mmath.h"
#include "math_debug_local.h"
#include "geometry.h"
#include "rigid_body.h"

static struct test_output ieee32_754_assert_type(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	/* Test is only for learning purposes, we don't care to port it to windows. */
#ifdef __LINUX__
	/* (1) Test largest, smallest NaN */
	TEST_NOT_ZERO(ieee32_test_nan(f32_nan()));
	TEST_NOT_ZERO(isnanf(f32_nan()));
	for (u32 i = 0x1; i < 0x7fffff; ++i)
	{
		const f32 nan_1 = f32_nan_(IEEE32_SIGN_MASK, i);
		const f32 nan_2 = f32_nan_(0, i);
		TEST_NOT_ZERO(isnanf(nan_1));
		TEST_NOT_ZERO(isnanf(nan_2));
		TEST_EQUAL(isnanf(nan_1), ieee32_test_nan(nan_1));
		TEST_EQUAL(isnanf(nan_2), ieee32_test_nan(nan_2));
	}
	TEST_ZERO(ieee32_test_nan(f32_inf(0)));
	TEST_ZERO(ieee32_test_nan(f32_inf(1)));
	TEST_ZERO(ieee32_test_nan(1.0f));

	/* (2) Test largest, smallest Infinity */
	TEST_NOT_ZERO(ieee32_test_positive_inf(f32_inf(0)));
	TEST_NOT_ZERO(ieee32_test_negative_inf(f32_inf(1)));
	TEST_ZERO(ieee32_test_positive_inf(f32_inf(1)));
	TEST_ZERO(ieee32_test_negative_inf(f32_inf(0)));
	TEST_ZERO(ieee32_test_positive_inf(f32_nan()));
	TEST_ZERO(ieee32_test_negative_inf(f32_nan()));
	TEST_ZERO(ieee32_test_positive_inf(1.0f));
	TEST_ZERO(ieee32_test_negative_inf(1.0f));

	/* (3) zeros */
	TEST_NOT_ZERO(ieee32_test_positive_zero(f32_zero(0)));
	TEST_NOT_ZERO(ieee32_test_negative_zero(f32_zero(1)));
	TEST_ZERO(ieee32_test_negative_zero(f32_zero(0)));
	TEST_ZERO(ieee32_test_positive_zero(f32_zero(1)));
	TEST_ZERO(ieee32_test_negative_zero(f32_nan()));
	TEST_ZERO(ieee32_test_positive_zero(f32_nan()));
	TEST_ZERO(ieee32_test_negative_zero(f32_inf(0)));
	TEST_ZERO(ieee32_test_positive_zero(f32_inf(0)));
	TEST_ZERO(ieee32_test_negative_zero(f32_inf(1)));
	TEST_ZERO(ieee32_test_positive_zero(f32_inf(1)));

	/* (4) subnormals and normals */
	TEST_NOT_ZERO(ieee32_test_subnormal(f32_max_positive_subnormal()));
	TEST_NOT_ZERO(ieee32_test_subnormal(f32_min_positive_subnormal()));
	TEST_NOT_ZERO(ieee32_test_subnormal(f32_max_negative_subnormal()));
	TEST_NOT_ZERO(ieee32_test_subnormal(f32_min_negative_subnormal()));

	TEST_NOT_ZERO(ieee32_test_normal(f32_max_positive_normal()));
	TEST_NOT_ZERO(ieee32_test_normal(f32_min_positive_normal()));
	TEST_NOT_ZERO(ieee32_test_normal(f32_max_negative_normal()));
	TEST_NOT_ZERO(ieee32_test_normal(f32_min_negative_normal()));

	TEST_ZERO(ieee32_test_subnormal(f32_max_positive_normal()));
	TEST_ZERO(ieee32_test_subnormal(f32_min_positive_normal()));
	TEST_ZERO(ieee32_test_subnormal(f32_max_negative_normal()));
	TEST_ZERO(ieee32_test_subnormal(f32_min_negative_normal()));

	TEST_ZERO(ieee32_test_normal(f32_max_positive_subnormal()));
	TEST_ZERO(ieee32_test_normal(f32_min_positive_subnormal()));
	TEST_ZERO(ieee32_test_normal(f32_max_negative_subnormal()));
	TEST_ZERO(ieee32_test_normal(f32_min_negative_subnormal()));

	TEST_EQUAL(fpclassify(f32_max_positive_subnormal()), FP_SUBNORMAL);
	TEST_EQUAL(fpclassify(f32_min_positive_subnormal()), FP_SUBNORMAL);
	TEST_EQUAL(fpclassify(f32_max_negative_subnormal()), FP_SUBNORMAL);
	TEST_EQUAL(fpclassify(f32_min_negative_subnormal()), FP_SUBNORMAL);

	TEST_EQUAL(fpclassify(f32_max_positive_normal()), FP_NORMAL);
	TEST_EQUAL(fpclassify(f32_min_positive_normal()), FP_NORMAL);
	TEST_EQUAL(fpclassify(f32_max_negative_normal()), FP_NORMAL);
	TEST_EQUAL(fpclassify(f32_min_negative_normal()), FP_NORMAL);

	TEST_EQUAL(f32_max_positive_normal(), FLT_MAX);
	TEST_EQUAL(f32_min_positive_normal(), FLT_MIN);
	TEST_EQUAL(f32_max_negative_normal(), -FLT_MIN);
	TEST_EQUAL(f32_min_negative_normal(), -FLT_MAX);

	TEST_EQUAL(f32_max_positive_subnormal(),  FLT_MIN - FLT_TRUE_MIN);
	TEST_EQUAL(f32_min_positive_subnormal(),  FLT_TRUE_MIN);
	TEST_EQUAL(f32_max_negative_subnormal(), -FLT_TRUE_MIN);
	TEST_EQUAL(f32_min_negative_subnormal(), -FLT_MIN + FLT_TRUE_MIN);

	TEST_EQUAL(-f32_inf(0), f32_inf(1));
	TEST_EQUAL(-f32_zero(0), f32_zero(1));
	TEST_EQUAL(-f32_min_positive_subnormal(), f32_max_negative_subnormal());
#endif
	return output;
}

static struct test_output ieee32_754_assert_helper(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	TEST_EQUAL(f32_sign(f32_zero(1)), 1);
	TEST_EQUAL(f32_sign(f32_zero(0)), 0);

	union ieee32 val = { .bits = 0x0 };
	TEST_EQUAL(ieee32_significand_length_without_trailing_zeroes(val), 0);
	TEST_EQUAL(ieee32_significand(val), 0x0);

	val.bits = 0x1;
	TEST_EQUAL(ieee32_significand_length_without_trailing_zeroes(val), 23);
	TEST_EQUAL(ieee32_significand(val), 0x1);

	val.bits = 0x00400000;
	TEST_EQUAL(ieee32_significand_length_without_trailing_zeroes(val), 1);
	TEST_EQUAL(ieee32_significand(val), 0x400000);

	val.bits = 0x00800000;
	TEST_EQUAL(ieee32_significand_length_without_trailing_zeroes(val), 0);
	TEST_EQUAL(ieee32_significand(val), 0x0);

	/* TODO: ieee32_exponent, ieee32_significand */

	return output;
}

static struct test_output ieee32_754_assert_order(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	const f32 nan = f32_nan();
	const f32 inf = f32_inf(0);
	const f32 neg_inf = f32_inf(1);
	const f32 zero = f32_zero(0);
	const f32 neg_zero = f32_zero(1);
	const f32 max_normal = f32_max_positive_normal();
	const f32 min_normal = f32_min_positive_normal();
	const f32 max_subnormal = f32_max_positive_subnormal();
	const f32 min_subnormal = f32_min_positive_subnormal();
	
	/* TEST NAN again NAN, INFINITIES, FP*/
	TEST_FALSE((nan > inf || nan <= inf) || (nan > neg_inf || nan <= neg_inf));
	TEST_FALSE((nan > zero || nan <= zero) || (nan > neg_zero || nan <= neg_zero));

	/* TEST INFINITIES against FP */
	TEST_TRUE(max_normal < inf && max_subnormal < inf);
	TEST_TRUE(-inf < -max_normal  && -inf < -max_subnormal);

	/* TEST zero against zero */
	TEST_TRUE(zero == neg_zero);
	TEST_TRUE(inf != neg_inf);

	/* TEST subnormal normal boundary */
	TEST_TRUE(max_subnormal < min_normal);
	TEST_TRUE(-min_normal < max_subnormal);

	return output;
}

static struct test_output ieee32_754_assert_rounding(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static struct test_output ieee32_754_assert_trap_interrupt(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static struct test_output ieee32_754_assert_addition(struct test_environment *env)
{
	/* Run test for every rounding version */

	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static struct test_output ieee32_754_assert_subtraction(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static struct test_output ieee32_754_assert_multiplication(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static struct test_output ieee32_754_assert_division(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static union ieee32 *ieee32_collection_init(u64 *len, struct arena *mem)
{
	union ieee32 collection[] =
	{
		ieee32_zero(0),
		ieee32_zero(1),
		ieee32_inf(0),
		ieee32_inf(1),
		ieee32_nan(),
		ieee32_max_positive_subnormal(),
		ieee32_min_negative_subnormal(),
		ieee32_min_positive_subnormal(),
		ieee32_max_negative_subnormal(),
		ieee32_max_positive_normal(),
		ieee32_min_negative_normal(),
		ieee32_min_positive_normal(),
		ieee32_max_negative_normal(),
		ieee32_set(0, -127, 0x300000),
		ieee32_set(0, -127, 0x000007),
		ieee32_set(0, -127, 0x000f70),
		ieee32_set(1, 0, 0),
		ieee32_set(0, -3, 0x700000),
		ieee32_set(1, 3, 0x0f0000),
		ieee32_set(0, 53, 0x020002),
		ieee32_set(0, 53, 0x010001),
		ieee32_set(0, -16, 0x400000),
		ieee32_set(0, -16, 0x000001),
		ieee32_set(0, -16, 0x7fffff),
		ieee32_set(0, -86, 0x00bf00),
		ieee32_set(0, -16, 0x000000),
		ieee32_set(0, -16, 0x400002),
		ieee32_set(0, -16, 0x200001),
		ieee32_set(0, -127, 0x400000),
		ieee32_set(0, -127, 0x200000),
		ieee32_set(0, -127, 0x100000),
		ieee32_set(0, -127, 0x080000),
		ieee32_set(0, -127, 0x040000),
		ieee32_set(0, -127, 0x020000),
		ieee32_set(0, -127, 0x010000),
		ieee32_set(0, -127, 0x08000),
		ieee32_set(0, -127, 0x04000),
		ieee32_set(0, -127, 0x02000),
		ieee32_set(0, -127, 0x01000),
		ieee32_set(0, -127, 0x00800),
		ieee32_set(0, -127, 0x00400),
		ieee32_set(0, -127, 0x00200),
		ieee32_set(0, -127, 0x00100),
		ieee32_set(0, -127, 0x00080),
		ieee32_set(0, -127, 0x00040),
		ieee32_set(0, -127, 0x00020),
		ieee32_set(0, -127, 0x00010),
		ieee32_set(0, -127, 0x00008),
		ieee32_set(0, -127, 0x00004),
		ieee32_set(0, -127, 0x00002),
		ieee32_set(0, -127, 0x00001),
		ieee32_set(0, -127, 0x08800),
		ieee32_set(0, -127, 0x04400),
		ieee32_set(0, -127, 0x02200),
		ieee32_set(0, -127, 0x01100),
		ieee32_set(0, -127, 0x00880),
		ieee32_set(0, -127, 0x00440),
		ieee32_set(0, -127, 0x00220),
		ieee32_set(0, -127, 0x00118),
		ieee32_set(0, -127, 0x00084),
		ieee32_set(0, -127, 0x00042),
		ieee32_set(0, -127, 0x00021),
		ieee32_set(0, -127, 0x00010),
		ieee32_set(0, -127, 0x00808),
		ieee32_set(0, -127, 0x00404),
		ieee32_set(0, -127, 0x00202),
		ieee32_set(0, -127, 0x00101),

	};
	*len = sizeof(collection) / sizeof(collection[0]);
	return arena_push(mem, collection, sizeof(collection));
}

static struct test_output fINF_assert_conversion_from_ieee32(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	fINF f;
	union ieee32 b;

	b = ieee32_zero(0);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0 && f.type == FINF_ZERO);
	TEST_TRUE(f.type == FINF_ZERO);

	b = ieee32_zero(1);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1 && f.type == FINF_ZERO);
	TEST_TRUE(f.type == FINF_ZERO);

	b = ieee32_inf(0);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0)
	TEST_TRUE(f.type == FINF_INF);

	b = ieee32_inf(1);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1)
	TEST_TRUE(f.type == FINF_INF);

	b = ieee32_nan();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.type == FINF_NAN);

	/* Test all boundary subnormals and normals */
	b = ieee32_max_positive_subnormal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -127);
	TEST_TRUE(f.significand_bit_count == 22);
	TEST_EQUAL(f.significand[0], 0x3f);
	TEST_EQUAL(f.significand[1], 0xff);
	TEST_EQUAL(f.significand[2], 0xff);

	b = ieee32_min_negative_subnormal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -127);
	TEST_TRUE(f.significand_bit_count == 22);
	TEST_EQUAL(f.significand[0], 0x3f);
	TEST_EQUAL(f.significand[1], 0xff);
	TEST_EQUAL(f.significand[2], 0xff);

	b = ieee32_min_positive_subnormal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -149);
	TEST_TRUE(f.significand_bit_count == 0);

	b = ieee32_max_negative_subnormal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -149);
	TEST_TRUE(f.significand_bit_count == 0);

	b = ieee32_max_positive_normal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == 127);
	TEST_TRUE(f.significand_bit_count == 23);
	TEST_EQUAL(f.significand[0], 0x7f);
	TEST_EQUAL(f.significand[1], 0xff);
	TEST_EQUAL(f.significand[2], 0xff);

	b = ieee32_min_negative_normal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == 127);
	TEST_TRUE(f.significand_bit_count == 23);
	TEST_EQUAL(f.significand[0], 0x7f);
	TEST_EQUAL(f.significand[1], 0xff);
	TEST_EQUAL(f.significand[2], 0xff);

	b = ieee32_min_positive_normal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -126);
	TEST_TRUE(f.significand_bit_count == 0);

	b = ieee32_max_negative_normal();
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -126);
	TEST_TRUE(f.significand_bit_count == 0);

	/* some random subnormals */
	b = ieee32_set(0, -127, 0x300000);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -128);
	TEST_TRUE(f.significand_bit_count == 1);

	b = ieee32_set(0, -127, 0x000007);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -147);
	TEST_TRUE(f.significand_bit_count == 2);

	b = ieee32_set(0, -127, 0x000f70);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -138);
	TEST_TRUE(f.significand_bit_count == 7);

	/* some random normals */
	b = ieee32_set(1, 0, 0);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == 0);
	TEST_TRUE(f.significand_bit_count == 0);

	b = ieee32_set(0, -3, 0x700000);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == -3);
	TEST_TRUE(f.significand_bit_count == 3);
	TEST_TRUE(f.significand[0] == 0x7);

	b = ieee32_set(1, 3, 0x0f0000);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 1);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == 3);
	TEST_TRUE(f.significand_bit_count == 7);
	TEST_TRUE(f.significand[0] == 0x0f);

	b = ieee32_set(0, 53, 0x020002);
	f = fINF_from_ieee32(env->mem_1, b);
	TEST_TRUE(f.sign == 0);
	TEST_TRUE(f.type == FINF_NORMAL);
	TEST_TRUE(f.exponent == 53);
	TEST_TRUE(f.significand_bit_count == 22);
	TEST_TRUE(f.significand[0] == 0x01);
	TEST_TRUE(f.significand[1] == 0x00);
	TEST_TRUE(f.significand[2] == 0x01);

	return output;
}

static struct test_output fINF_assert_conversion_to_ieee32(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	fINF f;
	union ieee32 b;

	u64 collection_len;
	union ieee32 *collection = ieee32_collection_init(&collection_len, env->mem_1);

	for (u32 round = 0; round < ROUNDING_COUNT; ++round)
	{
		for (u64 i = 0; i < collection_len; ++i)
		{
			f = fINF_from_ieee32(env->mem_1, collection[i]);
			TEST_EQUAL_PRINT(collection[i].bits, ieee32_from_fINF(env->mem_1, f, round).bits, ieee32_bits_print);
		}
	}

	return output;
}

static struct test_output fINF_assert_type(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static struct test_output fINF_assert_order(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

struct expected_rounding
{
	struct fINF val;
	union ieee32 rounded[5];
}; 

static struct expected_rounding *rounding_collection_init(u64 *len, struct arena *mem)
{


	u8 bits_0[] = { 0x01 };
	u8 bits_1[] = { 0x7f, 0xff, 0xff };

	/* 0 00000000 1...1 01 */
	u8 bits_2[] = { 0xff, 0xff, 0xfd };

	/* 0 00000000 1...1 11 */
	u8 bits_3[] = { 0xff, 0xff, 0xff };

	u8 bits_4[] = { 0x00, 0x00, 0x01 };
	u8 bits_5[] = { 0x00, 0x00, 0x00, 0x01 };
	u8 bits_6[] = { 0x00, 0x00, 0x00, 0x03 };
	u8 bits_7[] = { 0xff, 0xff, 0xff };
	u8 bits_8[] = { 0x01, 0xff, 0xff, 0xfd };
	u8 bits_9[] = { 0x01, 0xff, 0xff, 0xff };

	u8 bits_10[] = { 0x00, 0x00, 0x00, 0x01 };
	u8 bits_11[] = { 0x00, 0x00, 0x00, 0x01 };
	u8 bits_12[] = { 0x00, 0x00, 0x00, 0x03 };

	u8 bits_13[] = { 0x00, 0xff, 0xff, 0xff };
	u8 bits_14[] = { 0x01, 0xff, 0xff, 0xfd };
	u8 bits_15[] = { 0x01, 0xff, 0xff, 0xff };

	struct expected_rounding collection[] =
	{
		{ .val = fINF_set(mem, 0, -151, 0, NULL), .rounded = { ieee32_zero(0), ieee32_min_positive_subnormal(), ieee32_min_positive_subnormal(), ieee32_zero(0), ieee32_zero(0) } },	
		{ .val = fINF_set(mem, 0, -152, 0, NULL), .rounded = { ieee32_zero(0), ieee32_zero(0), ieee32_min_positive_subnormal(), ieee32_zero(0), ieee32_zero(0) } },	
		{ .val = fINF_set(mem, 0, -151, 1, bits_0), .rounded = { ieee32_min_positive_subnormal(), ieee32_min_positive_subnormal(), ieee32_min_positive_subnormal(), ieee32_zero(0), ieee32_zero(0) } },	
		{ .val = fINF_set(mem, 1, -151, 0, NULL), .rounded = { ieee32_zero(1), ieee32_max_negative_subnormal(), ieee32_zero(1), ieee32_max_negative_subnormal(), ieee32_zero(1) } },	
		{ .val = fINF_set(mem, 1, -152, 0, NULL), .rounded = { ieee32_zero(1), ieee32_zero(1), ieee32_zero(1), ieee32_max_negative_subnormal(), ieee32_zero(1) } },	
		{ .val = fINF_set(mem, 1, -151, 1, bits_0), .rounded = { ieee32_max_negative_subnormal(), ieee32_max_negative_subnormal(), ieee32_zero(1), ieee32_max_negative_subnormal(), ieee32_zero(1) } },	
		{ .val = fINF_set(mem, 0, -127, 23, bits_1), .rounded = { ieee32_min_positive_normal(), ieee32_min_positive_normal(), ieee32_min_positive_normal(), ieee32_max_positive_subnormal(), ieee32_max_positive_subnormal()} },
		{ .val = fINF_set(mem, 0, -127, 24, bits_2), .rounded = { ieee32_max_positive_subnormal(), ieee32_max_positive_subnormal(), ieee32_min_positive_normal(), ieee32_max_positive_subnormal(), ieee32_max_positive_subnormal() } },
		{ .val = fINF_set(mem, 0, -127, 24, bits_3), .rounded = { ieee32_min_positive_normal(), ieee32_min_positive_normal(), ieee32_min_positive_normal(), ieee32_max_positive_subnormal(), ieee32_max_positive_subnormal()} },
		{ .val = fINF_set(mem, 1, -127, 23, bits_1), .rounded = { ieee32_max_negative_normal(), ieee32_max_negative_normal(), ieee32_min_negative_subnormal(), ieee32_max_negative_normal(), ieee32_min_negative_subnormal() } },
		{ .val = fINF_set(mem, 1, -127, 24, bits_2), .rounded = { ieee32_min_negative_subnormal(), ieee32_min_negative_subnormal(), ieee32_min_negative_subnormal(), ieee32_max_negative_normal(), ieee32_min_negative_subnormal()} },	
		{ .val = fINF_set(mem, 1, -127, 24, bits_3), .rounded = { ieee32_max_negative_normal(), ieee32_max_negative_normal(), ieee32_min_negative_subnormal(), ieee32_max_negative_normal(), ieee32_min_negative_subnormal()} },
		{ .val = fINF_set(mem, 0, -126, 24, bits_4), .rounded = { ieee32_set(0, -126, 0x000000), ieee32_set(0, -126, 0x000001), ieee32_set(0, -126, 0x000001), ieee32_set(0, -126, 0x000000), ieee32_set(0, -126, 0x000000)} },	
		{ .val = fINF_set(mem, 0, -126, 25, bits_5), .rounded = { ieee32_set(0, -126, 0x000000), ieee32_set(0, -126, 0x000000), ieee32_set(0, -126, 0x000001), ieee32_set(0, -126, 0x000000), ieee32_set(0, -126, 0x000000)} },	
		{ .val = fINF_set(mem, 0, -126, 25, bits_6), .rounded = { ieee32_set(0, -126, 0x000001), ieee32_set(0, -126, 0x000001), ieee32_set(0, -126, 0x000001), ieee32_set(0, -126, 0x000000), ieee32_set(0, -126, 0x000000)} },	
		{ .val = fINF_set(mem, 1, -126, 24, bits_4), .rounded = { ieee32_set(1, -126, 0x000000), ieee32_set(1, -126, 0x000001), ieee32_set(1, -126, 0x000000), ieee32_set(1, -126, 0x000001), ieee32_set(1, -126, 0x000000)} },	
		{ .val = fINF_set(mem, 1, -126, 25, bits_5), .rounded = { ieee32_set(1, -126, 0x000000), ieee32_set(1, -126, 0x000000), ieee32_set(1, -126, 0x000000), ieee32_set(1, -126, 0x000001), ieee32_set(1, -126, 0x000000)} },	
		{ .val = fINF_set(mem, 1, -126, 25, bits_6), .rounded = { ieee32_set(1, -126, 0x000001), ieee32_set(1, -126, 0x000001), ieee32_set(1, -126, 0x000000), ieee32_set(1, -126, 0x000001), ieee32_set(1, -126, 0x000000)} },	
		{ .val = fINF_set(mem, 0, -126, 24, bits_7), .rounded = { ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000), ieee32_set(0, -126, 0x7fffff), ieee32_set(0, -126, 0x7fffff)} },
		{ .val = fINF_set(mem, 0, -126, 25, bits_8), .rounded = { ieee32_set(0, -126, 0x7fffff), ieee32_set(0, -126, 0x7fffff), ieee32_set(0, -125, 0x000000), ieee32_set(0, -126, 0x7fffff), ieee32_set(0, -126, 0x7fffff)} },
		{ .val = fINF_set(mem, 0, -126, 25, bits_9), .rounded = { ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000), ieee32_set(0, -126, 0x7fffff), ieee32_set(0, -126, 0x7fffff)} },
		{ .val = fINF_set(mem, 1, -126, 24, bits_7), .rounded = { ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000000), ieee32_set(1, -126, 0x7fffff), ieee32_set(1, -125, 0x000000), ieee32_set(1, -126, 0x7fffff)} },
		{ .val = fINF_set(mem, 1, -126, 25, bits_8), .rounded = { ieee32_set(1, -126, 0x7fffff), ieee32_set(1, -126, 0x7fffff), ieee32_set(1, -126, 0x7fffff), ieee32_set(1, -125, 0x000000), ieee32_set(1, -126, 0x7fffff)} },	
		{ .val = fINF_set(mem, 1, -126, 25, bits_9), .rounded = { ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000000), ieee32_set(1, -126, 0x7fffff), ieee32_set(1, -125, 0x000000), ieee32_set(1, -126, 0x7fffff)} },	
		{ .val = fINF_set(mem, 0, -125, 25, bits_10), .rounded = { ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000001), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000)} },
		{ .val = fINF_set(mem, 0, -125, 26, bits_11), .rounded = { ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000001), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000)} },
		{ .val = fINF_set(mem, 0, -125, 26, bits_12), .rounded = { ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000001), ieee32_set(0, -125, 0x000000), ieee32_set(0, -125, 0x000000)} },
		{ .val = fINF_set(mem, 1, -125, 26, bits_11), .rounded = { ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000001), ieee32_set(1, -125, 0x000000)} },
		{ .val = fINF_set(mem, 1, -125, 26, bits_12), .rounded = { ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000000), ieee32_set(1, -125, 0x000001), ieee32_set(1, -125, 0x000000)} },
		{ .val = fINF_set(mem, 0, -125, 25, bits_13), .rounded = { ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x3fffff), ieee32_set(0, -125, 0x3fffff)} },
		{ .val = fINF_set(mem, 0, -125, 26, bits_14), .rounded = { ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x3fffff), ieee32_set(0, -125, 0x3fffff)} },
		{ .val = fINF_set(mem, 0, -125, 26, bits_15), .rounded = { ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x400000), ieee32_set(0, -125, 0x3fffff), ieee32_set(0, -125, 0x3fffff)} },
		{ .val = fINF_set(mem, 1, -125, 25, bits_13), .rounded = { ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x3fffff), ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x3fffff)} },
		{ .val = fINF_set(mem, 1, -125, 26, bits_14), .rounded = { ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x3fffff), ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x3fffff)} },
		{ .val = fINF_set(mem, 1, -125, 26, bits_15), .rounded = { ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x3fffff), ieee32_set(1, -125, 0x400000), ieee32_set(1, -125, 0x3fffff)} },
		{ .val = fINF_set(mem, 0, 0, 24, bits_4), .rounded = { ieee32_set(0, 0, 0x000000), ieee32_set(0, 0, 0x000001), ieee32_set(0, 0, 0x000001), ieee32_set(0, 0, 0x000000), ieee32_set(0, 0, 0x000000)} },	
		{ .val = fINF_set(mem, 0, 0, 25, bits_5), .rounded = { ieee32_set(0, 0, 0x000000), ieee32_set(0, 0, 0x000000), ieee32_set(0, 0, 0x000001), ieee32_set(0, 0, 0x000000), ieee32_set(0, 0, 0x000000)} },	
		{ .val = fINF_set(mem, 0, 0, 25, bits_6), .rounded = { ieee32_set(0, 0, 0x000001), ieee32_set(0, 0, 0x000001), ieee32_set(0, 0, 0x000001), ieee32_set(0, 0, 0x000000), ieee32_set(0, 0, 0x000000)} },	
		{ .val = fINF_set(mem, 1, 0, 24, bits_4), .rounded = { ieee32_set(1, 0, 0x000000), ieee32_set(1, 0, 0x000001), ieee32_set(1, 0, 0x000000), ieee32_set(1, 0, 0x000001), ieee32_set(1, 0, 0x000000)} },	
		{ .val = fINF_set(mem, 1, 0, 25, bits_5), .rounded = { ieee32_set(1, 0, 0x000000), ieee32_set(1, 0, 0x000000), ieee32_set(1, 0, 0x000000), ieee32_set(1, 0, 0x000001), ieee32_set(1, 0, 0x000000)} },	
		{ .val = fINF_set(mem, 1, 0, 25, bits_6), .rounded = { ieee32_set(1, 0, 0x000001), ieee32_set(1, 0, 0x000001), ieee32_set(1, 0, 0x000000), ieee32_set(1, 0, 0x000001), ieee32_set(1, 0, 0x000000)} },	
		{ .val = fINF_set(mem, 0, 0, 24, bits_7), .rounded = { ieee32_set(0, 1, 0x000000), ieee32_set(0, 1, 0x000000), ieee32_set(0, 1, 0x000000), ieee32_set(0, 0, 0x7fffff), ieee32_set(0, 0, 0x7fffff)} },
		{ .val = fINF_set(mem, 0, 0, 25, bits_8), .rounded = { ieee32_set(0, 0, 0x7fffff), ieee32_set(0, 0, 0x7fffff), ieee32_set(0, 1, 0x000000), ieee32_set(0, 0, 0x7fffff), ieee32_set(0, 0, 0x7fffff)} },
		{ .val = fINF_set(mem, 0, 0, 25, bits_9), .rounded = { ieee32_set(0, 1, 0x000000), ieee32_set(0, 1, 0x000000), ieee32_set(0, 1, 0x000000), ieee32_set(0, 0, 0x7fffff), ieee32_set(0, 0, 0x7fffff)} },
		{ .val = fINF_set(mem, 1, 0, 24, bits_7), .rounded = { ieee32_set(1, 1, 0x000000), ieee32_set(1, 1, 0x000000), ieee32_set(1, 0, 0x7fffff), ieee32_set(1, 1, 0x000000), ieee32_set(1, 0, 0x7fffff)} },
		{ .val = fINF_set(mem, 1, 0, 25, bits_8), .rounded = { ieee32_set(1, 0, 0x7fffff), ieee32_set(1, 0, 0x7fffff), ieee32_set(1, 0, 0x7fffff), ieee32_set(1, 1, 0x000000), ieee32_set(1, 0, 0x7fffff)} },	
		{ .val = fINF_set(mem, 1, 0, 25, bits_9), .rounded = { ieee32_set(1, 1, 0x000000), ieee32_set(1, 1, 0x000000), ieee32_set(1, 0, 0x7fffff), ieee32_set(1, 1, 0x000000), ieee32_set(1, 0, 0x7fffff)} },	
		{ .val = fINF_set(mem, 0, 127, 24, bits_4), .rounded = { ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000)} },	
		{ .val = fINF_set(mem, 0, 127, 25, bits_5), .rounded = { ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000)} },	
		{ .val = fINF_set(mem, 0, 127, 25, bits_6), .rounded = { ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000)} },	
		{ .val = fINF_set(mem, 1, 127, 24, bits_4), .rounded = { ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000)} },	
		{ .val = fINF_set(mem, 1, 127, 25, bits_5), .rounded = { ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000)} },	
		{ .val = fINF_set(mem, 1, 127, 25, bits_6), .rounded = { ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000)} },	
		{ .val = fINF_set(mem, 0, 127, 24, bits_7), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_set(0, 127, 0x7fffff), ieee32_set(0, 127, 0x7fffff)} },
		{ .val = fINF_set(mem, 0, 127, 25, bits_8), .rounded = { ieee32_set(0, 127, 0x7fffff), ieee32_set(0, 127, 0x7fffff), ieee32_inf(0), ieee32_set(0, 127, 0x7fffff), ieee32_set(0, 127, 0x7fffff)} },
		{ .val = fINF_set(mem, 0, 127, 25, bits_9), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_set(0, 127, 0x7fffff), ieee32_set(0, 127, 0x7fffff)} },
		{ .val = fINF_set(mem, 1, 127, 24, bits_7), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_set(1, 127, 0x7fffff), ieee32_inf(1), ieee32_set(1, 127, 0x7fffff)} },
		{ .val = fINF_set(mem, 1, 127, 25, bits_8), .rounded = { ieee32_set(1, 127, 0x7fffff), ieee32_set(1, 127, 0x7fffff), ieee32_set(1, 127, 0x7fffff), ieee32_inf(1), ieee32_set(1, 127, 0x7fffff)} },	
		{ .val = fINF_set(mem, 1, 127, 25, bits_9), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_set(1, 127, 0x7fffff), ieee32_inf(1), ieee32_set(1, 127, 0x7fffff)} },	
		{ .val = fINF_set(mem, 0, 127, 25, bits_10), .rounded = { ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000)} },
		{ .val = fINF_set(mem, 0, 127, 26, bits_11), .rounded = { ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000)} },
		{ .val = fINF_set(mem, 0, 127, 26, bits_12), .rounded = { ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000001), ieee32_set(0, 127, 0x000000), ieee32_set(0, 127, 0x000000)} },
		{ .val = fINF_set(mem, 1, 127, 25, bits_10), .rounded = { ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000)} },
		{ .val = fINF_set(mem, 1, 127, 26, bits_11), .rounded = { ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000)} },
		{ .val = fINF_set(mem, 1, 127, 26, bits_12), .rounded = { ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000000), ieee32_set(1, 127, 0x000001), ieee32_set(1, 127, 0x000000)} },
		{ .val = fINF_set(mem, 0, 127, 25, bits_13), .rounded = { ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x3fffff), ieee32_set(0, 127, 0x3fffff)} },
		{ .val = fINF_set(mem, 0, 127, 26, bits_14), .rounded = { ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x3fffff), ieee32_set(0, 127, 0x3fffff)} },
		{ .val = fINF_set(mem, 0, 127, 26, bits_15), .rounded = { ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x400000), ieee32_set(0, 127, 0x3fffff), ieee32_set(0, 127, 0x3fffff)} },
		{ .val = fINF_set(mem, 1, 127, 25, bits_13), .rounded = { ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x3fffff), ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x3fffff)} },
		{ .val = fINF_set(mem, 1, 127, 26, bits_14), .rounded = { ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x3fffff), ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x3fffff)} },
		{ .val = fINF_set(mem, 1, 127, 26, bits_15), .rounded = { ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x3fffff), ieee32_set(1, 127, 0x400000), ieee32_set(1, 127, 0x3fffff)} },
		{ .val = fINF_set(mem, 0, 128, 25, bits_10), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_max_positive_normal(), ieee32_max_positive_normal() } },
		{ .val = fINF_set(mem, 0, 128, 26, bits_11), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_max_positive_normal(), ieee32_max_positive_normal() } },
		{ .val = fINF_set(mem, 0, 128, 26, bits_12), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_max_positive_normal(), ieee32_max_positive_normal() } },
		{ .val = fINF_set(mem, 1, 128, 25, bits_10), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_min_negative_normal(), ieee32_inf(1), ieee32_min_negative_normal() } },
		{ .val = fINF_set(mem, 1, 128, 26, bits_11), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_min_negative_normal(), ieee32_inf(1), ieee32_min_negative_normal() } },
		{ .val = fINF_set(mem, 1, 128, 26, bits_12), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_min_negative_normal(), ieee32_inf(1), ieee32_min_negative_normal() } },
		{ .val = fINF_set(mem, 0, 128, 25, bits_13), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_max_positive_normal(), ieee32_max_positive_normal() } },
		{ .val = fINF_set(mem, 0, 128, 26, bits_14), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_max_positive_normal(), ieee32_max_positive_normal() } },
		{ .val = fINF_set(mem, 0, 128, 26, bits_15), .rounded = { ieee32_inf(0), ieee32_inf(0), ieee32_inf(0), ieee32_max_positive_normal(), ieee32_max_positive_normal() } },
		{ .val = fINF_set(mem, 1, 128, 25, bits_13), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_min_negative_normal(), ieee32_inf(1), ieee32_min_negative_normal() } },
		{ .val = fINF_set(mem, 1, 128, 26, bits_14), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_min_negative_normal(), ieee32_inf(1), ieee32_min_negative_normal() } },
		{ .val = fINF_set(mem, 1, 128, 26, bits_15), .rounded = { ieee32_inf(1), ieee32_inf(1), ieee32_min_negative_normal(), ieee32_inf(1), ieee32_min_negative_normal() } },
	};

	*len = sizeof(collection) / sizeof(collection[0]);
	return arena_push(mem, collection, sizeof(collection));
}

static struct test_output fINF_assert_rounding(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	fINF f;
	union ieee32 b;

	u64 collection_len;
	struct expected_rounding *collection = rounding_collection_init(&collection_len, env->mem_1);

	for (u32 round = 0; round < ROUNDING_COUNT; ++round)
	{
		for (u64 i = 0; i < collection_len; ++i)
		{
			f = (env->mem_1, collection[i].val);
			TEST_EQUAL_PRINT(collection[i].rounded[round].bits, ieee32_from_fINF(env->mem_1, f, round).bits, ieee32_bits_print);
		}
	}

	return output;
}

static struct test_output fINF_assert_trap_interrupt(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

static f32 *special_arithmetic_collection_init(u64 *len, struct arena *mem)
{
	f32 collection[] =
	{
		f32_max_positive_subnormal(),
		f32_min_positive_subnormal(),
		f32_max_negative_subnormal(),
		f32_min_negative_subnormal(),
		f32_max_positive_normal(),
		f32_min_positive_normal(),
		f32_max_negative_normal(),
		f32_min_negative_normal(),
		-f32_max_positive_subnormal(),
		-f32_min_positive_subnormal(),
		-f32_max_negative_subnormal(),
		-f32_min_negative_subnormal(),
		-f32_max_positive_normal(),
		-f32_min_positive_normal(),
		-f32_max_negative_normal(),
		-f32_min_negative_normal(),
		f32_nan(),
		f32_inf(0),
		f32_inf(1),
		f32_zero(0),
		f32_zero(1),
		1.0f,
		-1.0f,
	};

	*len = sizeof(collection) / sizeof(collection[0]);
	return arena_push(mem, collection, sizeof(collection));
}

static struct test_output fINF_assert_arithmetic(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	mersenne_twister_init(env->seed);

	u64 len;
	f32 *collection = special_arithmetic_collection_init(&len, env->mem_1);

	const u64 count = 10000;
	for (u64 round = 0; round < 5; ++round)
	{
		switch (round)
		{
			case ROUNDING_NEAREST_TIES_EVEN: { fesetround(FE_TONEAREST); } break;
			/* not supported in gcc as of yet?? */
			case ROUNDING_NEAREST_TIES_AWAY_FROM_ZERO: { continue; } break;
			case ROUNDING_TOWARDS_PLUS_INF: { fesetround(FE_UPWARD); } break;
			case ROUNDING_TOWARDS_MINUS_INF: { fesetround(FE_DOWNWARD); } break;
			case ROUNDING_TOWARDS_ZERO: { fesetround(FE_TOWARDZERO); } break;
		}
		
		for (u64 j = 0; j < count; ++j)
		{
			struct arena record = *env->mem_1;

			const f32 a = gen_continuous_uniform_f(-123.45f, 643.12f);
			const f32 b = gen_continuous_uniform_f(-123.45f, 643.12f);
			const f32 c = a + b;
			const f32 d = a - b;
			const f32 e = a * b;
			const union ieee32 ci = { .f = c };
			const union ieee32 di = { .f = d };
			const union ieee32 ei = { .f = e };

			const fINF af = fINF_from_f32(env->mem_1, a);
			const fINF bf = fINF_from_f32(env->mem_1, b);
			const fINF cf = fINF_add(env->mem_1, af, bf, round);
			const fINF df = fINF_sub(env->mem_1, af, bf, round);
			const fINF ef = fINF_mul(env->mem_1, af, bf, round);

			TEST_EQUAL_PRINT(ci.bits, ieee32_from_fINF(env->mem_1, cf, round).bits, ieee32_bits_print);
			TEST_EQUAL_PRINT(di.bits, ieee32_from_fINF(env->mem_1, df, round).bits, ieee32_bits_print);
			TEST_EQUAL_PRINT(ei.bits, ieee32_from_fINF(env->mem_1, ef, round).bits, ieee32_bits_print);

			*env->mem_1 = record;
		}

		for (u64 i = 0; i < len; ++i)
		{
			for (u64 j = 0; j < len; ++j)
			{
				struct arena record = *env->mem_1;

				const f32 a = collection[i];
				const f32 b = collection[j];
				const f32 c = a + b;
				const f32 d = a - b;
				const f32 e = a * b;
				union ieee32 ci = { .f = c };
				union ieee32 di = { .f = d };
				union ieee32 ei = { .f = e };

				if (ieee32_test_nan(c)) { ci = ieee32_nan(); }
				if (ieee32_test_nan(d)) { di = ieee32_nan(); }
				if (ieee32_test_nan(e)) { ei = ieee32_nan(); }

				const fINF af = fINF_from_f32(env->mem_1, a);
				const fINF bf = fINF_from_f32(env->mem_1, b);

				const fINF cf = fINF_add(env->mem_1, af, bf, round);
				const fINF df = fINF_sub(env->mem_1, af, bf, round);
				const fINF ef = fINF_mul(env->mem_1, af, bf, round);

				TEST_EQUAL_PRINT(ci.bits, ieee32_from_fINF(env->mem_1, cf, round).bits, ieee32_bits_print);
				TEST_EQUAL_PRINT(di.bits, ieee32_from_fINF(env->mem_1, df, round).bits, ieee32_bits_print);
				TEST_EQUAL_PRINT(ei.bits, ieee32_from_fINF(env->mem_1, ef, round).bits, ieee32_bits_print);

				*env->mem_1 = record;
			}
		}
	}

	fesetround(FE_TONEAREST);


	return output;
}

static struct test_output fINF_assert_division(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	return output;
}

void gen_box(vec3 box[8], const vec3 center, const vec3 hw)
{
	vec3_set(box[0], center[0] - hw[0],  center[1] - hw[1], center[2] - hw[2]);
	vec3_set(box[1], center[0] - hw[0],  center[1] - hw[1], center[2] + hw[2]);
	vec3_set(box[2], center[0] - hw[0],  center[1] + hw[1], center[2] - hw[2]);
	vec3_set(box[3], center[0] - hw[0],  center[1] + hw[1], center[2] + hw[2]);
	vec3_set(box[4], center[0] + hw[0],  center[1] - hw[1], center[2] - hw[2]);
	vec3_set(box[5], center[0] + hw[0],  center[1] - hw[1], center[2] + hw[2]);
	vec3_set(box[6], center[0] + hw[0],  center[1] + hw[1], center[2] - hw[2]);
	vec3_set(box[7], center[0] + hw[0],  center[1] + hw[1], center[2] + hw[2]);
}

static struct test_output rigid_statics_assert(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };

	mersenne_twister_init(env->seed);

	vec3 box[8];
	f32 density = 1.0f;

	vec3 centers[9] = 
	{
		{  0.0f,  0.0f,  0.0f },
		{  2.0f,  2.0f,  2.0f },
		{  2.0f,  2.0f, -2.0f },
		{  2.0f, -2.0f,  2.0f },
		{  2.0f, -2.0f, -2.0f },
		{ -2.0f,  2.0f,  2.0f },
		{ -2.0f,  2.0f, -2.0f },
		{ -2.0f, -2.0f,  2.0f },
		{ -2.0f, -2.0f, -2.0f },
	};

	vec3 hw = { 0.5f, 0.5f, 0.5f };

	for (u64 i = 0; i < 9; ++i)
	{
		gen_box(box, centers[i], hw);
		struct tri_mesh mesh = convex_hull_construct(env->mem_1, env->mem_2, env->mem_3, env->mem_4, env->mem_5, env->mem_6, box, 8, 0.0001f);
		struct rigid_body body;
		statics_setup(&body, env->mem_1, &mesh, density);
		statics_print(stderr, &body);
	}

	return output;
}

static struct test_output (*math_tests[])(struct test_environment *) =
{
	ieee32_754_assert_type,
	ieee32_754_assert_helper,
	ieee32_754_assert_order,
	ieee32_754_assert_rounding,
	ieee32_754_assert_trap_interrupt,
	fINF_assert_conversion_from_ieee32,
	fINF_assert_conversion_to_ieee32,
	fINF_assert_type,
	fINF_assert_order,
	fINF_assert_rounding,
	fINF_assert_trap_interrupt,
	fINF_assert_arithmetic,
	rigid_statics_assert,
};

struct suite m_math_suite =
{
	.id = "math",
	.tests = math_tests,
	.test_count = sizeof(math_tests) / sizeof(math_tests[0]),
};

struct suite *math_suite = &m_math_suite;
