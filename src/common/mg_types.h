#ifndef __MG_TYPES_H__
#define __MG_TYPES_H__

#include <stdint.h>

typedef	uint8_t  u8;
typedef	uint16_t u16;
typedef	uint32_t u32;
typedef	uint64_t u64;

typedef	int8_t  i8;
typedef	int16_t i16;
typedef	int32_t i32;
typedef	int64_t i64;

typedef float	f32;
typedef double	f64;

#ifdef VEC_TYPE_DOUBLE 
	typedef double vec_type;
#else
	typedef float vec_type;
#endif

typedef vec_type vec2[2];
typedef vec_type vec3[3];
typedef vec_type vec4[4];

typedef vec_type (*vec2ptr)[2];
typedef vec_type (*vec3ptr)[3];
typedef vec_type (*vec4ptr)[4];

typedef u32 vec2u32[2];
typedef u32 vec3u32[3];
typedef u32 vec4u32[4];

typedef u32 (*vec2u32ptr)[2];
typedef u32 (*vec3u32ptr)[3];
typedef u32 (*vec4u32ptr)[4];

typedef u64 vec2u64[2];
typedef u64 vec3u64[3];
typedef u64 vec4u64[4];

typedef u64 (*vec2u64ptr)[2];
typedef u64 (*vec3u64ptr)[3];
typedef u64 (*vec4u64ptr)[4];

typedef i32 vec2i32[2];
typedef i32 vec3i32[3];
typedef i32 vec4i32[4];

typedef i32 (*vec2i32ptr)[2];
typedef i32 (*vec3i32ptr)[3];
typedef i32 (*vec4i32ptr)[4];

typedef i64 vec2i64[2];
typedef i64 vec3i64[3];
typedef i64 vec4i64[4];

typedef i64 (*vec2i64ptr)[2];
typedef i64 (*vec3i64ptr)[3];
typedef i64 (*vec4i64ptr)[4];


#endif
