#include <assert.h>
#include <float.h>
#include <math.h>
//#include "nmmintrin.h"
//#include "immintrin.h"

#include "mmath.h"

//#include "SSE.h"

#ifdef MG_DEBUG

void vec2_print(const char *text, const vec2 v)
{
	printf("%s: (%f, %f), \n", text, v[0], v[1]);
}

void vec3_print(const char *text, const vec3 v)
{
	printf("%s: (%f, %f, %f), \n", text, v[0], v[1], v[2]);
}

void vec4_print(const char *text, const vec4 v)
{
	printf("%s: (%f, %f, %f, %f), \n", text, v[0], v[1], v[2], v[3]);
}

void vec2u32_print(const char *text, const vec2u32 v)
{
	printf("%s: (%u, %u), \n", text, v[0], v[1]);
}

void vec3u32_print(const char *text, const vec3u32 v)
{
	printf("%s: (%u, %u, %u), \n", text, v[0], v[1], v[2]);
}

void vec4u32_print(const char *text, const vec4u32 v)
{
	printf("%s: (%u, %u, %u, %u), \n", text, v[0], v[1], v[2], v[3]);
}

void vec2u64_print(const char *text, const vec2u64 v)
{
	printf("%s: (%lu, %lu), \n", text, v[0], v[1]);
}

void vec3u64_print(const char *text, const vec3u64 v)
{
	printf("%s: (%lu, %lu, %lu), \n", text, v[0], v[1], v[2]);
}

void vec4u64_print(const char *text, const vec4u64 v)
{
	printf("%s: (%lu, %lu, %lu, %lu), \n", text, v[0], v[1], v[2], v[3]);
}

void vec2i32_print(const char *text, const vec2i32 v)
{
	printf("%s: (%i, %i), \n", text, v[0], v[1]);
}

void vec3i32_print(const char *text, const vec3i32 v)
{
	printf("%s: (%i, %i, %i), \n", text, v[0], v[1], v[2]);
}

void vec4i32_print(const char *text, const vec4i32 v)
{
	printf("%s: (%i, %i, %i, %i), \n", text, v[0], v[1], v[2], v[3]);
}

void vec2i64_print(const char *text, const vec2i64 v)
{
	printf("%s: (%li, %li), \n", text, v[0], v[1]);
}

void vec3i64_print(const char *text, const vec3i64 v)
{
	printf("%s: (%li, %li, %li), \n", text, v[0], v[1], v[2]);
}

void vec4i64_print(const char *text, const vec4i64 v)
{
	printf("%s: (%li, %li, %li, %li), \n", text, v[0], v[1], v[2], v[3]);
}

#endif


void vec2u32_set(vec2u32 dst, const u32 x, const u32 y)
{
	dst[0] = x;
	dst[1] = y;
}

void vec2u64_set(vec2u64 dst, const u64 x, const u64 y)
{
	dst[0] = x;
	dst[1] = y;
}

void vec2i32_set(vec2i32 dst, const i32 x, const i32 y)
{
	dst[0] = x;
	dst[1] = y;
}
void vec2i64_set(vec2i64 dst, const i64 x, const i64 y)
{
	dst[0] = x;
	dst[1] = y;
}

void vec3u32_set(vec3u32 dst, const u32 x, const u32 y, const u32 z)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}

void vec3u64_set(vec3u64 dst, const u64 x, const u64 y, const u64 z)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}

void vec3i32_set(vec3i32 dst, const i32 x, const i32 y, const i32 z)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}

void vec3i64_set(vec3i64 dst, const i64 x, const i64 y, const i64 z)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}

void vec4u32_set(vec4u32 dst, const u32 x, const u32 y, const u32 z, const u32 w)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

void vec4u64_set(vec4u64 dst, const u64 x, const u64 y, const u64 z, const u64 w)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

void vec4i32_set(vec4i32 dst, const i32 x, const i32 y, const i32 z, const i32 w)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

void vec4i64_set(vec4i64 dst, const i64 x, const i64 y, const i64 z, const i64 w)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}                                       

void vec2u32_copy(vec2u32 dst, const vec2u32 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
}

void vec2u64_copy(vec2u64 dst, const vec2u64 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
}

void vec2i32_copy(vec2i32 dst, const vec2i32 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
}

void vec2i64_copy(vec2i64 dst, const vec2i64 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
}
                                       
void vec3u32_copy(vec3u32 dst, const vec3u32 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void vec3u64_copy(vec3u64 dst, const vec3u64 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void vec3i32_copy(vec3i32 dst, const vec3i32 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void vec3i64_copy(vec3i64 dst, const vec3i64 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}
                                      
void vec4u32_copy(vec4u32 dst, const vec4u32 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void vec4u64_copy(vec4u64 dst, const vec4u64 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void vec4i32_copy(vec4i32 dst, const vec4i32 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void vec4i64_copy(vec4i64 dst, const vec4i64 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void vec2_negative(vec2 v)
{
	v[0] = -v[0];
	v[1] = -v[1];
}

void vec3_negative(vec3 v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void vec4_negative(vec4 v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
	v[3] = -v[3];
}

void vec2_scale(vec2 dst, const vec2 src, const vec_type scale)
{
	dst[0] = scale * src[0];
	dst[1] = scale * src[1];
}

void vec3_scale(vec3 dst, const vec3 src, const vec_type scale)
{
	dst[0] = scale * src[0];
	dst[1] = scale * src[1];
	dst[2] = scale * src[2];
}

void vec4_scale(vec4 dst, const vec4 src, const vec_type scale)
{
	dst[0] = scale * src[0];
	dst[1] = scale * src[1];
	dst[2] = scale * src[2];
	dst[3] = scale * src[3];
}

void vec2_translate_scaled(vec2 dst, const vec2 to_scale, const vec_type scale)
{
	dst[0] += scale * to_scale[0];
	dst[1] += scale * to_scale[1];
}              
               
void vec3_translate_scaled(vec3 dst, const vec3 to_scale, const vec_type scale)
{
	dst[0] += scale * to_scale[0];
	dst[1] += scale * to_scale[1];
	dst[2] += scale * to_scale[2];
}              

void vec4_translate_scaled(vec4 dst, const vec4 to_scale, const vec_type scale)
{
	dst[0] += scale * to_scale[0];
	dst[1] += scale * to_scale[1];
	dst[2] += scale * to_scale[2];
	dst[3] += scale * to_scale[3];
}

vec_type vec2_distance(const vec2 a, const vec2 b)
{
	vec_type sum = (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]);
#ifdef VEC_TYPE_DOUBLE
	return sqrt(sum);
#else 
	return sqrtf(sum);
#endif
}

vec_type vec3_distance(const vec3 a, const vec3 b)
{
	vec_type sum = (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]) + (b[2]-a[2])*(b[2]-a[2]);
#ifdef VEC_TYPE_DOUBLE
	return sqrt(sum);
#else 
	return sqrtf(sum);
#endif
}

vec_type vec4_distance(const vec4 a, const vec4 b)
{
	vec_type sum = (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]) + (b[2]-a[2])*(b[2]-a[2]) + (b[3]-a[3])*(b[3]-a[3]);
#ifdef VEC_TYPE_DOUBLE
	return sqrt(sum);
#else 
	return sqrtf(sum);
#endif
}

vec_type vec2_distance_squared(const vec2 a, const vec2 b)
{
	return (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]);
}

vec_type vec3_distance_squared(const vec3 a, const vec3 b)
{
	return (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]) + (b[2]-a[2])*(b[2]-a[2]);
}

vec_type vec4_distance_squared(const vec4 a, const vec4 b)
{
	return (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]) + (b[2]-a[2])*(b[2]-a[2]) + (b[3]-a[3])*(b[3]-a[3]);
}



void vec2_set(vec2 dst, const vec_type x, const vec_type y)
{
	dst[0] = x;
	dst[1] = y;
}

void vec2_copy(vec2 dst, const vec2 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
}

void vec2_add(vec2 dst, const vec2 a, const vec2 b)
{
	dst[0] = a[0] + b[0];
	dst[1] = a[1] + b[1];
}

void vec2_sub(vec2 dst, const vec2 a, const vec2 b)
{
	dst[0] = a[0] - b[0];
	dst[1] = a[1] - b[1];
}

void vec2_mul(vec2 dst, const vec2 a, const vec2 b)
{
	dst[0] = a[0] * b[0];
	dst[1] = a[1] * b[1];
}

void vec2_div(vec2 dst, const vec2 a, const vec2 b)
{
	assert(b[0] != 0.0f && b[1] != 0.0f);

	dst[0] = a[0] / b[0];
	dst[1] = a[1] / b[1];
}

vec_type vec2_length(const vec2 a)
{
	vec_type sum = a[0]*a[0] + a[1]*a[1];
#ifdef VEC_TYPE_DOUBLE
	return sqrt(sum);
#else 
	return sqrtf(sum);
#endif
}

//TODO one mul operation instead of two!
void vec2_normalize(vec2 dst, const vec2 a)
{
	vec_type length = vec2_length(a);
	vec2_copy(dst, a);
	vec2_mul_constant(dst, 1/length);
}


void vec2_translate(vec2 dst, const vec2 translation)
{
	dst[0] += translation[0];
	dst[1] += translation[1];
}

void vec2_add_const(vec2 dst, const vec_type c)
{
	dst[0] += c;
	dst[1] += c;
}

void vec2_mul_constant(vec2 dst, const vec_type c)
{
	dst[0] *= c;
	dst[1] *= c;
}


vec_type vec2_dot(const vec2 a, const vec2 b)
{
	return a[0] * b[0] + a[1] * b[1];
}

void vec2_interpolate(vec2 dst, const vec2 a, const vec2 b, const vec_type alpha)
{
#ifdef VEC_TYPE_DOUBLE
	dst[0] = a[0] * alpha + b[0] * (1.0 - alpha);
	dst[1] = a[1] * alpha + b[1] * (1.0 - alpha);
#else 
	dst[0] = a[0] * alpha + b[0] * (1.0f - alpha);
	dst[1] = a[1] * alpha + b[1] * (1.0f - alpha);
#endif
}

void vec2_interpolate_piecewise(vec2 dst, const vec2 a, const vec2 b, const vec2 alpha)
{
#ifdef VEC_TYPE_DOUBLE
	dst[0] = a[0] * alpha[0] + b[0] * (1.0 - alpha[0]);
	dst[1] = a[1] * alpha[1] + b[1] * (1.0 - alpha[1]);
#else                                                     
	dst[0] = a[0] * alpha[0] + b[0] * (1.0f - alpha[0]);
	dst[1] = a[1] * alpha[1] + b[1] * (1.0f - alpha[1]);
#endif
}

void vec3_set(vec3 dst, const vec_type x, const vec_type y, const vec_type z)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
}

void vec3_copy(vec3 dst, const vec3 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void vec3_add(vec3 dst, const vec3 a, const vec3 b)
{
	dst[0] = a[0] + b[0];
	dst[1] = a[1] + b[1];
	dst[2] = a[2] + b[2];
}

void vec3_sub(vec3 dst, const vec3 a, const vec3 b)
{
	dst[0] = a[0] - b[0];
	dst[1] = a[1] - b[1];
	dst[2] = a[2] - b[2];
}

void vec3_mul(vec3 dst, const vec3 a, const vec3 b)
{
	dst[0] = a[0] * b[0];
	dst[1] = a[1] * b[1];
	dst[2] = a[2] * b[2];
}

void vec3_div(vec3 dst, const vec3 a, const vec3 b)
{
	assert(b[0] != 0.0f && b[1] != 0.0f && b[2] != 0.0f);

	dst[0] = a[0] / b[0];
	dst[1] = a[1] / b[1];
	dst[2] = a[2] / b[2];
}

vec_type vec3_length(const vec3 a)
{
	vec_type sum = a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
#ifdef VEC_TYPE_DOUBLE
	return sqrt(sum);
#else 
	return sqrtf(sum);
#endif
}

//TODO one mul operation instead of two!
void vec3_normalize(vec3 dst, const vec3 a)
{
	vec_type length = vec3_length(a);
	vec3_copy(dst, a);
	vec3_mul_constant(dst, 1/length);
}

void vec3_translate(vec3 dst, const vec3 translation)
{
	dst[0] += translation[0];
	dst[1] += translation[1];
	dst[2] += translation[2];
}

void vec3_mul_constant(vec3 dst, const vec_type c)
{
	dst[0] *= c;
	dst[1] *= c;
	dst[2] *= c;
}

void vec3_add_const(vec3 dst, const vec_type c)
{
	dst[0] += c;
	dst[1] += c;
	dst[2] += c;
}

vec_type vec3_dot(const vec3 a, const vec3 b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vec3_cross(vec3 dst, const vec3 a, const vec3 b)
{
	dst[0] = a[1] * b[2] - a[2] * b[1];
	dst[1] = a[2] * b[0] - a[0] * b[2];
	dst[2] = a[0] * b[1] - a[1] * b[0];
}

void vec3_recenter_cross(vec3 dst, const vec3 center, const vec3 a, const vec3 b)
{
	vec3 a_c, b_c;
	vec3_sub(a_c, a, center);
	vec3_sub(b_c, b, center);
	vec3_cross(dst, a_c, b_c);
}
/* CCW */
void vec3_rotate_y(vec3 dst, const vec3 a, const vec_type angle)
{
	mat3 rot;
       	mat3_set(rot, cos(angle), 0.0f, sin(angle),
			    0.0f, 1.0f, 0.0f, 
			    -sin(angle), 0.0f,  cos(angle));
	vec3_mat_mul(dst, a, rot);	
}

void vec3_interpolate(vec3 dst, const vec3 a, const vec3 b, const vec_type alpha)
{
#ifdef VEC_TYPE_DOUBLE
	dst[0] = a[0] * alpha + b[0] * (1.0 - alpha);
	dst[1] = a[1] * alpha + b[1] * (1.0 - alpha);
	dst[2] = a[2] * alpha + b[2] * (1.0 - alpha);
#else 
	dst[0] = a[0] * alpha + b[0] * (1.0f - alpha);
	dst[1] = a[1] * alpha + b[1] * (1.0f - alpha);
	dst[2] = a[2] * alpha + b[2] * (1.0f - alpha);
#endif
}

void vec3_interpolate_piecewise(vec3 dst, const vec3 a, const vec3 b, const vec3 alpha)
{
#ifdef VEC_TYPE_DOUBLE
	dst[0] = a[0] * alpha[0] + b[0] * (1.0 - alpha[0]);
	dst[1] = a[1] * alpha[1] + b[1] * (1.0 - alpha[1]);
	dst[2] = a[2] * alpha[2] + b[2] * (1.0 - alpha[2]);
#else                                                     
	dst[0] = a[0] * alpha[0] + b[0] * (1.0f - alpha[0]);
	dst[1] = a[1] * alpha[1] + b[1] * (1.0f - alpha[1]);
	dst[2] = a[2] * alpha[2] + b[2] * (1.0f - alpha[2]);
#endif
}

void vec3_triple_product(vec3 dst, const vec3 a, const vec3 b, const vec3 c)
{
	vec3 tmp;
	vec3_cross(tmp, a, b);
	vec3_cross(dst, tmp, c);
}

void vec4_set(vec4 dst, const vec_type x, const vec_type y, const vec_type z, const vec_type w)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

void vec4_copy(vec4 dst, const vec4 src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void vec4_add(vec4 dst, const vec4 a, const vec4 b)
{
	dst[0] = a[0] + b[0];
	dst[1] = a[1] + b[1];
	dst[2] = a[2] + b[2];
	dst[3] = a[3] + b[3];
}

void vec4_sub(vec4 dst, const vec4 a, const vec4 b)
{
	dst[0] = a[0] - b[0];
	dst[1] = a[1] - b[1];
	dst[2] = a[2] - b[2];
	dst[3] = a[3] - b[3];
}

void vec4_mul(vec4 dst, const vec4 a, const vec4 b)
{
	dst[0] = a[0] * b[0];
	dst[1] = a[1] * b[1];
	dst[2] = a[2] * b[2];
	dst[3] = a[3] * b[3];
}

void vec4_div(vec4 dst, const vec4 a, const vec4 b)
{
	assert(b[0] != 0.0f && b[1] != 0.0f && b[2] != 0.0f && b[3] != 0.0f);

	dst[0] = a[0] / b[0];
	dst[1] = a[1] / b[1];
	dst[2] = a[2] / b[2];
	dst[3] = a[3] / b[3];
}

vec_type vec4_length(const vec4 a)
{
	vec_type sum = a[0]*a[0] + a[1]*a[1] + a[2]*a[2] + a[3]*a[3];
#ifdef VEC_TYPE_DOUBLE
	return sqrt(sum);
#else 
	return sqrtf(sum);
#endif
}

//TODO one mul operation instead of two!
void vec4_normalize(vec4 dst, const vec4 a)
{
	vec_type length = vec4_length(a);
	vec4_copy(dst, a);
	vec4_mul_constant(dst, 1/length);
}

void vec4_translate(vec4 dst, const vec4 translation)
{
	dst[0] += translation[0];
	dst[1] += translation[1];
	dst[2] += translation[2];
	dst[3] += translation[3];
}

void vec4_add_const(vec4 dst, const vec_type c)
{
	dst[0] += c;
	dst[1] += c;
	dst[2] += c;
	dst[3] += c;
}

void vec4_mul_constant(vec4 dst, const vec_type c)
{
	dst[0] *= c;
	dst[1] *= c;
	dst[2] *= c;
	dst[3] *= c;
}

vec_type vec4_dot(const vec4 a, const vec4 b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

void vec4_interpolate(vec4 dst, const vec4 a, const vec4 b, const vec_type alpha)
{
#ifdef VEC_TYPE_DOUBLE
	dst[0] = a[0] * alpha + b[0] * (1.0 - alpha);
	dst[1] = a[1] * alpha + b[1] * (1.0 - alpha);
	dst[2] = a[2] * alpha + b[2] * (1.0 - alpha);
	dst[3] = a[3] * alpha + b[3] * (1.0 - alpha);
#else 
	dst[0] = a[0] * alpha + b[0] * (1.0f - alpha);
	dst[1] = a[1] * alpha + b[1] * (1.0f - alpha);
	dst[2] = a[2] * alpha + b[2] * (1.0f - alpha);
	dst[3] = a[3] * alpha + b[3] * (1.0f - alpha);
#endif
}

void vec4_interpolate_piecewise(vec4 dst, const vec4 a, const vec4 b, const vec4 alpha)
{
#ifdef VEC_TYPE_DOUBLE
	dst[0] = a[0] * alpha[0] + b[0] * (1.0 - alpha[0]);
	dst[1] = a[1] * alpha[1] + b[1] * (1.0 - alpha[1]);
	dst[2] = a[2] * alpha[2] + b[2] * (1.0 - alpha[2]);
	dst[3] = a[3] * alpha[3] + b[3] * (1.0 - alpha[3]);
#else                                                     
	dst[0] = a[0] * alpha[0] + b[0] * (1.0f - alpha[0]);
	dst[1] = a[1] * alpha[1] + b[1] * (1.0f - alpha[1]);
	dst[2] = a[2] * alpha[2] + b[2] * (1.0f - alpha[2]);
	dst[3] = a[3] * alpha[3] + b[3] * (1.0f - alpha[3]);
#endif
}
