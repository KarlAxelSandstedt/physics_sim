#ifndef __VECTOR_MATH__
#define __VECTOR_MATH__

#include "mg_common.h"

//#define VEC_TYPE_DOUBLE
//#define __SSE__

#define VEC2_ZERO { 0.0f, 0.0f }
#define VEC3_ZERO { 0.0f, 0.0f, 0.0f }
#define VEC4_ZERO { 0.0f, 0.0f, 0.0f, 0.0f }
#define VEC2U_ZERO { 0, 0 }
#define VEC3U_ZERO { 0, 0, 0 }
#define VEC4U_ZERO { 0, 0, 0, 0 }

#ifdef MG_DEBUG
void vec2_print(const char *text, const vec2 v);
void vec3_print(const char *text, const vec3 v);
void vec4_print(const char *text, const vec4 v);

void vec2u32_print(const char *text, const vec2u32 v);
void vec3u32_print(const char *text, const vec3u32 v);
void vec4u32_print(const char *text, const vec4u32 v);

void vec2u64_print(const char *text, const vec2u64 v);
void vec3u64_print(const char *text, const vec3u64 v);
void vec4u64_print(const char *text, const vec4u64 v);

void vec2i32_print(const char *text, const vec2i32 v);
void vec3i32_print(const char *text, const vec3i32 v);
void vec4i32_print(const char *text, const vec4i32 v);

void vec2i64_print(const char *text, const vec2i64 v);
void vec3i64_print(const char *text, const vec3i64 v);
void vec4i64_print(const char *text, const vec4i64 v);
#endif

void vec2u32_set(vec2u32 dst, const u32 x, const u32 y);
void vec2u64_set(vec2u64 dst, const u64 x, const u64 y);
void vec2i32_set(vec2i32 dst, const i32 x, const i32 y);
void vec2i64_set(vec2i64 dst, const i64 x, const i64 y);

void vec3u32_set(vec3u32 dst, const u32 x, const u32 y, const u32 z);
void vec3u64_set(vec3u64 dst, const u64 x, const u64 y, const u64 z);
void vec3i32_set(vec3i32 dst, const i32 x, const i32 y, const i32 z);
void vec3i64_set(vec3i64 dst, const i64 x, const i64 y, const i64 z);

void vec4u32_set(vec4u32 dst, const u32 x, const u32 y, const u32 z, const u32 w);
void vec4u64_set(vec4u64 dst, const u64 x, const u64 y, const u64 z, const u64 w);
void vec4i32_set(vec4i32 dst, const i32 x, const i32 y, const i32 z, const i32 w);
void vec4i64_set(vec4i64 dst, const i64 x, const i64 y, const i64 z, const i64 w);
                                        
void vec2u32_copy(vec2u32 dst, const vec2u32 src); 
void vec2u64_copy(vec2u64 dst, const vec2u64 src); 
void vec2i32_copy(vec2i32 dst, const vec2i32 src); 
void vec2i64_copy(vec2i64 dst, const vec2i64 src); 
                                       
void vec3u32_copy(vec3u32 dst, const vec3u32 src); 
void vec3u64_copy(vec3u64 dst, const vec3u64 src); 
void vec3i32_copy(vec3i32 dst, const vec3i32 src); 
void vec3i64_copy(vec3i64 dst, const vec3i64 src); 
                                      
void vec4u32_copy(vec4u32 dst, const vec4u32 src); 
void vec4u64_copy(vec4u64 dst, const vec4u64 src); 
void vec4i32_copy(vec4i32 dst, const vec4i32 src); 
void vec4i64_copy(vec4i64 dst, const vec4i64 src); 

void vec2_translate_scaled(vec2 dst, const vec2 to_scale, const vec_type scale); 
void vec3_translate_scaled(vec3 dst, const vec3 to_scale, const vec_type scale);
void vec4_translate_scaled(vec4 dst, const vec4 to_scale, const vec_type scale);

void vec2_negative(vec2 v);
void vec3_negative(vec3 v);
void vec4_negative(vec4 v);

vec_type vec2_distance(const vec2 a, const vec2 b); 
vec_type vec3_distance(const vec3 a, const vec3 b);
vec_type vec4_distance(const vec4 a, const vec4 b);

vec_type vec2_distance_squared(const vec2 a, const vec2 b); 
vec_type vec3_distance_squared(const vec3 a, const vec3 b);
vec_type vec4_distance_squared(const vec4 a, const vec4 b);

void vec2_set(vec2 dst, const vec_type x, const vec_type y);
void vec2_copy(vec2 dst, const vec2 src);
void vec2_add(vec2 dst, const vec2 a, const vec2 b);
void vec2_sub(vec2 dst, const vec2 a, const vec2 b); /* a - b */
void vec2_mul(vec2 dst, const vec2 a, const vec2 b);
void vec2_div(vec2 dst, const vec2 a, const vec2 b); /* a / b */
void vec2_scale(vec2 dst, const vec2 src, const vec_type scale);
vec_type vec2_length(const vec2 a);
void vec2_normalize(vec2 dst, const vec2 a);
void vec2_translate(vec2 dst, const vec2 translation);
void vec2_add_constant(vec2 dst, const vec_type c);
void vec2_mul_constant(vec2 dst, const vec_type c);
vec_type vec2_dot(const vec2 a, const vec2 b);
void vec2_interpolate(vec2 dst, const vec2 a, const vec2 b, const vec_type alpha);
void vec2_interpolate_piecewise(vec2 dst, const vec2 a, const vec2 b, const vec2 alpha);

void vec3_set(vec3 dst, const vec_type x, const vec_type y, const vec_type z);
void vec3_copy(vec3 dst, const vec3 src);
void vec3_add(vec3 dst, const vec3 a, const vec3 b);
void vec3_sub(vec3 dst, const vec3 a, const vec3 b); /*	a - b */
void vec3_mul(vec3 dst, const vec3 a, const vec3 b);
void vec3_div(vec3 dst, const vec3 a, const vec3 b); /* a / b */
void vec3_scale(vec3 dst, const vec3 src, const vec_type scale);
vec_type vec3_length(const vec3 a);
void vec3_normalize(vec3 dst, const vec3 a);
void vec3_translate(vec3 dst, const vec3 translation);
void vec3_add_constant(vec3 dst, const vec_type c);
void vec3_mul_constant(vec3 dst, const vec_type c);
vec_type vec3_dot(const vec3 a, const vec3 b);
void vec3_cross(vec3 dst, const vec3 a, const vec3 b); /* a cross b */
void vec3_rotate_y(vec3 dst, const vec3 a, const vec_type angle);
void vec3_interpolate(vec3 dst, const vec3 a, const vec3 b, const vec_type alpha);
void vec3_interpolate_piecewise(vec3 dst, const vec3 a, const vec3 b, const vec3 alpha);
/* (a x b) x c */
void vec3_triple_product(vec3 dst, const vec3 a, const vec3 b, const vec3 c);

/* (a-center) x (b-center) */
void vec3_recenter_cross(vec3 dst, const vec3 center, const vec3 a, const vec3 b);

void vec4_set(vec4 dst, const vec_type x, const vec_type y, const vec_type z, const vec_type w);
void vec4_copy(vec4 dst, const vec4 src);
void vec4_add(vec4 dst, const vec4 a, const vec4 b);
void vec4_sub(vec4 dst, const vec4 a, const vec4 b); /*	a - b */
void vec4_mul(vec4 dst, const vec4 a, const vec4 b);
void vec4_div(vec4 dst, const vec4 a, const vec4 b); /* a / b */
void vec4_scale(vec4 dst, const vec4 src, const vec_type scale);
vec_type vec4_length(const vec4 a);
void vec4_normalize(vec4 dst, const vec4 a);
void vec4_translate(vec4 dst, const vec4 translation);
void vec4_add_constant(vec4 dst, const vec_type c);
void vec4_mul_constant(vec4 dst, const vec_type c);
vec_type vec4_dot(const vec4 a, const vec4 b);
void vec4_interpolate(vec4 dst, const vec4 a, const vec4 b, const vec_type alpha);
void vec4_interpolate_piecewise(vec4 dst, const vec4 a, const vec4 b, const vec4 alpha);

#endif
