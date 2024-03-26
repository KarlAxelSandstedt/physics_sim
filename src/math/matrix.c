#include "mmath.h"

#ifdef MG_DEBUG

void mat2_print(const char *text, mat2 m)
{
	printf("%s:\n| %f %f |\n| %f %f |\n", text,
		       	m[0][0], m[1][0],
		       	m[0][1], m[1][1]
			);
}
void mat3_print(const char *text, mat3 m)
{
	printf("%s:\n| %f %f %f |\n| %f %f %f |\n| %f %f %f |\n", text,
		       	m[0][0], m[1][0], m[2][0],
		       	m[0][1], m[1][1], m[2][1],
		       	m[0][2], m[1][2], m[2][2] 
			);
}
void mat4_print(const char *text, mat4 m)
{
	printf("%s:\n| %f %f %f %f |\n| %f %f %f %f |\n| %f %f %f %f |\n| %f %f %f %f |\n", text,
		       	m[0][0], m[1][0], m[2][0], m[3][0],
		       	m[0][1], m[1][1], m[2][1], m[3][1],
		       	m[0][2], m[1][2], m[2][2], m[3][2],
		       	m[0][3], m[1][3], m[2][3], m[3][3]
			);
}

#endif

void mat2_set(mat2 dst, const matrix_type a11, const matrix_type a21,
			const matrix_type a12, const matrix_type a22)
{
	dst[0][0] = a11; dst[0][1] = a21;
	dst[1][0] = a12; dst[1][1] = a22;
}

void mat3_set(mat3 dst, const matrix_type a11, const matrix_type a21, const matrix_type a31,
			const matrix_type a12, const matrix_type a22, const matrix_type a32,  
			const matrix_type a13, const matrix_type a23, const matrix_type a33)
{
	dst[0][0] = a11; dst[0][1] = a21; dst[0][2] = a31;
	dst[1][0] = a12; dst[1][1] = a22; dst[1][2] = a32;
	dst[2][0] = a13; dst[2][1] = a23; dst[2][2] = a33;
}

void mat4_set(mat4 dst, const matrix_type a11, const matrix_type a21, const matrix_type a31, const matrix_type a41,
			const matrix_type a12, const matrix_type a22, const matrix_type a32, const matrix_type a42, 
			const matrix_type a13, const matrix_type a23, const matrix_type a33, const matrix_type a43, 
			const matrix_type a14, const matrix_type a24, const matrix_type a34, const matrix_type a44)
{
	dst[0][0] = a11; dst[0][1] = a21; dst[0][2] = a31; dst[0][3] = a41;
	dst[1][0] = a12; dst[1][1] = a22; dst[1][2] = a32; dst[1][3] = a42;
	dst[2][0] = a13; dst[2][1] = a23; dst[2][2] = a33; dst[2][3] = a43;
	dst[3][0] = a14; dst[3][1] = a24; dst[3][2] = a34; dst[3][3] = a44;
}


void mat2_set_columns(mat2 dst, const vec2 c1, const vec2 c2)
{
	dst[0][0] = c1[0];
	dst[0][1] = c1[1];
	dst[1][0] = c2[0];
	dst[1][1] = c2[1];
}

void mat3_set_columns(mat3 dst, const vec3 c1, const vec3 c2, const vec3 c3)
{

	dst[0][0] = c1[0];
	dst[0][1] = c1[1];
	dst[0][2] = c1[2];
	dst[1][0] = c2[0];
	dst[1][1] = c2[1];
	dst[1][2] = c2[2];
	dst[2][0] = c3[0];
	dst[2][1] = c3[1];
	dst[2][2] = c3[2];
}

void mat4_set_columns(mat4 dst, const vec4 c1, const vec4 c2, const vec4 c3, const vec4 c4)
{
	dst[0][0] = c1[0];
	dst[0][1] = c1[1];
	dst[0][2] = c1[2];
	dst[0][3] = c1[3];
	dst[1][0] = c2[0];
	dst[1][1] = c2[1];
	dst[1][2] = c2[2];
	dst[1][3] = c2[3];
	dst[2][0] = c3[0];
	dst[2][1] = c3[1];
	dst[2][2] = c3[2];
	dst[2][3] = c3[3];
	dst[3][0] = c4[0];
	dst[3][1] = c4[1];
	dst[3][2] = c4[2];
	dst[3][3] = c4[3];
}

void mat2_set_rows(mat2 dst, const vec2 r1, const vec2 r2)
{
	dst[0][0] = r1[0];
	dst[1][0] = r1[1];
	dst[0][1] = r2[0];
	dst[1][1] = r2[1];
}

void mat3_set_rows(mat3 dst, const vec3 r1, const vec3 r2, const vec3 r3)
{
	dst[0][0] = r1[0];
	dst[1][0] = r1[1];
	dst[2][0] = r1[2];
	dst[0][1] = r2[0];
	dst[1][1] = r2[1];
	dst[2][1] = r2[2];
	dst[0][2] = r3[0];
	dst[1][2] = r3[1];
	dst[2][2] = r3[2];
}

void mat4_set_rows(mat4 dst, const vec4 r1, const vec4 r2, const vec4 r3, const vec4 r4)
{
	dst[0][0] = r1[0];
	dst[1][0] = r1[1];
	dst[2][0] = r1[2];
	dst[3][0] = r1[3];
	dst[0][1] = r2[0];
	dst[1][1] = r2[1];
	dst[2][1] = r2[2];
	dst[3][1] = r2[3];
	dst[0][2] = r3[0];
	dst[1][2] = r3[1];
	dst[2][2] = r3[2];
	dst[3][2] = r3[3];
	dst[0][3] = r4[0];
	dst[1][3] = r4[1];
	dst[2][3] = r4[2];
	dst[3][3] = r4[3];
}

void mat2_identity(mat2 dst)
{
	mat2_set(dst, 1,0,0,1);
}

void mat3_identity(mat3 dst)
{
	mat3_set(dst, 1,0,0,0,1,0,0,0,1);
}

void mat4_identity(mat4 dst)
{
	mat4_set(dst, 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
}


void vec2_mat_mul(vec2 dst, const vec2 vec, mat2 mat)
{
	dst[0] = vec[0] * mat[0][0] + vec[1] *  mat[0][1];
	dst[1] = vec[0] * mat[1][0] + vec[1] *  mat[1][1];
}

void vec3_mat_mul(vec3 dst, const vec3 vec, mat3 mat)
{
	dst[0] = vec[0] * mat[0][0] + vec[1] *  mat[0][1] + vec[2] * mat[0][2];
	dst[1] = vec[0] * mat[1][0] + vec[1] *  mat[1][1] + vec[2] * mat[1][2];
	dst[2] = vec[0] * mat[2][0] + vec[1] *  mat[2][1] + vec[2] * mat[2][2];
}

void vec4_mat_mul(vec4 dst, const vec4 vec, mat4 mat)
{
	dst[0] = vec[0] * mat[0][0] + vec[1] *  mat[0][1] + vec[2] * mat[0][2] + vec[3] * mat[0][3];
	dst[1] = vec[0] * mat[1][0] + vec[1] *  mat[1][1] + vec[2] * mat[1][2] + vec[3] * mat[1][3];
	dst[2] = vec[0] * mat[2][0] + vec[1] *  mat[2][1] + vec[2] * mat[2][2] + vec[3] * mat[2][3];
	dst[3] = vec[0] * mat[3][0] + vec[1] *  mat[3][1] + vec[2] * mat[3][2] + vec[3] * mat[3][3];
}

void mat2_vec_mul(vec2 dst, mat2 mat, const vec2 vec)
{
	dst[0] = vec[0] * mat[0][0] + vec[1] *  mat[1][0];
	dst[1] = vec[0] * mat[0][1] + vec[1] *  mat[1][1];
}

void mat3_vec_mul(vec3 dst, mat3 mat, const vec3 vec)
{
	dst[0] = vec[0] * mat[0][0] + vec[1] *  mat[1][0] + vec[2] * mat[2][0];
	dst[1] = vec[0] * mat[0][1] + vec[1] *  mat[1][1] + vec[2] * mat[2][1];
	dst[2] = vec[0] * mat[0][2] + vec[1] *  mat[1][2] + vec[2] * mat[2][2];
}

void mat4_vec_mul(vec4 dst, mat4 mat, const vec4 vec)
{
	dst[0] = vec[0] * mat[0][0] + vec[1] *  mat[1][0] + vec[2] * mat[2][0] + vec[3] * mat[3][0];
	dst[1] = vec[0] * mat[0][1] + vec[1] *  mat[1][1] + vec[2] * mat[2][1] + vec[3] * mat[3][1];
	dst[2] = vec[0] * mat[0][2] + vec[1] *  mat[1][2] + vec[2] * mat[2][2] + vec[3] * mat[3][2];
	dst[3] = vec[0] * mat[0][3] + vec[1] *  mat[1][3] + vec[2] * mat[2][3] + vec[3] * mat[3][3];
}

// dst = [a][b]
void mat2_mult(mat2 dst, mat2 a, mat2 b)
{
	dst[0][0] = a[0][0]*b[0][0] + a[1][0]*b[0][1];
	dst[0][1] = a[0][1]*b[0][0] + a[1][1]*b[0][1];

	dst[1][0] = a[0][0]*b[1][0] + a[1][0]*b[1][1];
	dst[1][1] = a[0][1]*b[1][0] + a[1][1]*b[1][1];
}

// dst = [a][b]
void mat3_mult(mat3 dst, mat3 a, mat3 b)
{
	dst[0][0] = a[0][0]*b[0][0] + a[1][0]*b[0][1] + a[2][0]*b[0][2];
	dst[0][1] = a[0][1]*b[0][0] + a[1][1]*b[0][1] + a[2][1]*b[0][2];
	dst[0][2] = a[0][2]*b[0][0] + a[1][2]*b[0][1] + a[2][2]*b[0][2];

	dst[1][0] = a[0][0]*b[1][0] + a[1][0]*b[1][1] + a[2][0]*b[1][2];
	dst[1][1] = a[0][1]*b[1][0] + a[1][1]*b[1][1] + a[2][1]*b[1][2];
	dst[1][2] = a[0][2]*b[1][0] + a[1][2]*b[1][1] + a[2][2]*b[1][2];

	dst[2][0] = a[0][0]*b[2][0] + a[1][0]*b[2][1] + a[2][0]*b[2][2];
	dst[2][1] = a[0][1]*b[2][0] + a[1][1]*b[2][1] + a[2][1]*b[2][2];
	dst[2][2] = a[0][2]*b[2][0] + a[1][2]*b[2][1] + a[2][2]*b[2][2];
}

// dst = [a][b]
void mat4_mult(mat4 dst, mat4 a, mat4 b)
{
	dst[0][0] = a[0][0]*b[0][0] + a[1][0]*b[0][1] + a[2][0]*b[0][2] + a[3][0]*b[0][3];
	dst[0][1] = a[0][1]*b[0][0] + a[1][1]*b[0][1] + a[2][1]*b[0][2] + a[3][1]*b[0][3];
	dst[0][2] = a[0][2]*b[0][0] + a[1][2]*b[0][1] + a[2][2]*b[0][2] + a[3][2]*b[0][3];
	dst[0][3] = a[0][3]*b[0][0] + a[1][3]*b[0][1] + a[2][3]*b[0][2] + a[3][3]*b[0][3];

	dst[1][0] = a[0][0]*b[1][0] + a[1][0]*b[1][1] + a[2][0]*b[1][2] + a[3][0]*b[1][3];
	dst[1][1] = a[0][1]*b[1][0] + a[1][1]*b[1][1] + a[2][1]*b[1][2] + a[3][1]*b[1][3];
	dst[1][2] = a[0][2]*b[1][0] + a[1][2]*b[1][1] + a[2][2]*b[1][2] + a[3][2]*b[1][3];
	dst[1][3] = a[0][3]*b[1][0] + a[1][3]*b[1][1] + a[2][3]*b[1][2] + a[3][3]*b[1][3];

	dst[2][0] = a[0][0]*b[2][0] + a[1][0]*b[2][1] + a[2][0]*b[2][2] + a[3][0]*b[2][3];
	dst[2][1] = a[0][1]*b[2][0] + a[1][1]*b[2][1] + a[2][1]*b[2][2] + a[3][1]*b[2][3];
	dst[2][2] = a[0][2]*b[2][0] + a[1][2]*b[2][1] + a[2][2]*b[2][2] + a[3][2]*b[2][3];
	dst[2][3] = a[0][3]*b[2][0] + a[1][3]*b[2][1] + a[2][3]*b[2][2] + a[3][3]*b[2][3];

	dst[3][0] = a[0][0]*b[3][0] + a[1][0]*b[3][1] + a[2][0]*b[3][2] + a[3][0]*b[3][3];
	dst[3][1] = a[0][1]*b[3][0] + a[1][1]*b[3][1] + a[2][1]*b[3][2] + a[3][1]*b[3][3];
	dst[3][2] = a[0][2]*b[3][0] + a[1][2]*b[3][1] + a[2][2]*b[3][2] + a[3][2]*b[3][3];
	dst[3][3] = a[0][3]*b[3][0] + a[1][3]*b[3][1] + a[2][3]*b[3][2] + a[3][3]*b[3][3];
}

void mat2_add(mat2 dst, mat2 a, mat2 b)
{
	dst[0][0] = a[0][0] + b[0][0];  
        dst[0][1] = a[0][1] + b[0][1];

        dst[1][0] = a[1][0] + b[1][0];
        dst[1][1] = a[1][1] + b[1][1];
}

void mat3_add(mat3 dst, mat3 a, mat3 b)
{
	dst[0][0] = a[0][0] + b[0][0];  
        dst[0][1] = a[0][1] + b[0][1];
        dst[0][2] = a[0][2] + b[0][2];

        dst[1][0] = a[1][0] + b[1][0];
        dst[1][1] = a[1][1] + b[1][1];
        dst[1][2] = a[1][2] + b[1][2];

        dst[2][0] = a[2][0] + b[2][0];
        dst[2][1] = a[2][1] + b[2][1];
        dst[2][2] = a[2][2] + b[2][2];
}

void mat4_add(mat4 dst, mat4 a, mat4 b)
{
	dst[0][0] = a[0][0] + b[0][0];  
        dst[0][1] = a[0][1] + b[0][1];
        dst[0][2] = a[0][2] + b[0][2];
        dst[0][3] = a[0][3] + b[0][3];

        dst[1][0] = a[1][0] + b[1][0];
        dst[1][1] = a[1][1] + b[1][1];
        dst[1][2] = a[1][2] + b[1][2];
        dst[1][3] = a[1][3] + b[1][3];

        dst[2][0] = a[2][0] + b[2][0];
        dst[2][1] = a[2][1] + b[2][1];
        dst[2][2] = a[2][2] + b[2][2];
        dst[2][3] = a[2][3] + b[2][3];
	
        dst[3][0] = a[3][0] + b[3][0];
        dst[3][1] = a[3][1] + b[3][1];
        dst[3][2] = a[3][2] + b[3][2];
        dst[3][3] = a[3][3] + b[3][3];
}
