#ifndef __MATRIX_MATH__
#define __MATRIX_MATH__

#include "vector.h"

typedef vec2 mat2[2];
typedef vec3 mat3[3];
typedef vec4 mat4[4];

typedef vec_type matrix_type;

#ifdef MG_DEBUG
void mat2_print(const char *text, mat2 m);
void mat3_print(const char *text, mat3 m);
void mat4_print(const char *text, mat4 m);
#endif

/* Fill in column-major order */
void mat2_set(mat2 dst, const matrix_type a11, const matrix_type a21,
    			const matrix_type a12, const matrix_type a22);

/* Fill in column-major order */
void mat3_set(mat3 dst, const matrix_type a11, const matrix_type a21, const matrix_type a31,
			const matrix_type a12, const matrix_type a22, const matrix_type a32,  
			const matrix_type a13, const matrix_type a23, const matrix_type a33);

/* Fill in column-major order */
void mat4_set(mat4 dst, const matrix_type a11, const matrix_type a21, const matrix_type a31, const matrix_type a41,
			const matrix_type a12, const matrix_type a22, const matrix_type a32, const matrix_type a42, 
			const matrix_type a13, const matrix_type a23, const matrix_type a33, const matrix_type a43, 
			const matrix_type a14, const matrix_type a24, const matrix_type a34, const matrix_type a44);

void mat2_set_columns(mat2 dst, const vec2 c1, const vec2 c2);
void mat3_set_columns(mat3 dst, const vec3 c1, const vec3 c2, const vec3 c3);
void mat4_set_columns(mat4 dst, const vec4 c1, const vec4 c2, const vec4 c3, const vec4 c4);

void mat2_set_rows(mat2 dst, const vec2 r1, const vec2 r2);
void mat3_set_rows(mat3 dst, const vec3 r1, const vec3 r2, const vec3 r3);
void mat4_set_rows(mat4 dst, const vec4 r1, const vec4 r2, const vec4 r3, const vec4 r4);

void mat2_identity(mat2 dst);
void mat3_identity(mat3 dst);
void mat4_identity(mat4 dst);

/* dst = vec*mat */
void vec2_mat_mul(vec2 dst, const vec2 vec, mat2 mat);
void vec3_mat_mul(vec3 dst, const vec3 vec, mat3 mat);
void vec4_mat_mul(vec4 dst, const vec4 vec, mat4 mat);

/* dst = mat * vec */
void mat2_vec_mul(vec2 dst, mat2 mat, const vec2 vec);
void mat3_vec_mul(vec3 dst, mat3 mat, const vec3 vec);
void mat4_vec_mul(vec4 dst, mat4 mat, const vec4 vec);

void mat2_add(mat2 dst, mat2 a, mat2 b);
void mat3_add(mat3 dst, mat3 a, mat3 b);
void mat4_add(mat4 dst, mat4 a, mat4 b);

/* dst = a*b */
void mat2_mult(mat2 dst, mat2 a, mat2 b);
void mat3_mult(mat3 dst, mat3 a, mat3 b);
void mat4_mult(mat4 dst, mat4 a, mat4 b);

#endif
