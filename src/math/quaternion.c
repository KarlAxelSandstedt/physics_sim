#include <math.h>
#include "mmath.h"
#include "quaternion.h"

void quat_set(quat dst, const vec_type x, const vec_type y, const vec_type z, const vec_type w)
{
	dst[0] = x;
	dst[1] = y;
	dst[2] = z;
	dst[3] = w;
}

void quat_add(quat dst, const quat p, const quat q)
{
	dst[0] = p[0] + q[0];
	dst[1] = p[1] + q[1];
	dst[2] = p[2] + q[2];
	dst[3] = p[3] + q[3];
}

void quat_sub(quat dst, const quat p, const quat q)
{
	dst[0] = p[0] - q[0];
	dst[1] = p[1] - q[1];
	dst[2] = p[2] - q[2];
	dst[3] = p[3] - q[3];
}

void quat_mult(quat dst, const quat p, const quat q)
{
	dst[0] = p[0] * q[3] + p[3] * q[0] + p[1] * q[2] - p[2] * q[1];
	dst[1] = p[1] * q[3] + p[3] * q[1] + p[2] * q[0] - p[0] * q[2];
	dst[2] = p[2] * q[3] + p[3] * q[2] + p[0] * q[1] - p[1] * q[0];
	dst[3] = p[3] * q[3] - p[0] * q[0] - p[1] * q[1] - p[2] * q[2];
}

void quat_scale(quat dst, const vec_type scale)
{
	dst[0] = dst[0] * scale;
	dst[1] = dst[1] * scale;
	dst[2] = dst[2] * scale;
	dst[3] = dst[3] * scale;
}

void quat_copy(quat dst, const quat q)
{
	dst[0] = q[0];
	dst[1] = q[1];
	dst[2] = q[2];
	dst[3] = q[3];
}

void quat_conj(quat conj, const quat q)
{
	conj[0] = -q[0];
	conj[1] = -q[1];
	conj[2] = -q[2];
	conj[3] = q[3];
}

vec_type quat_norm(const quat q)
{
#ifdef VEC_TYPE_DOUBLE 
	return sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
#else
	return sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
#endif
}

void quat_inv(quat inv, const quat q)
{
#ifdef VEC_TYPE_DOUBLE 
	vec_type norm_2_inv = 1.0 / quat_norm(q);
#else
	vec_type norm_2_inv = 1.0f / quat_norm(q);
#endif
	quat_conj(inv, q);
	quat_scale(inv, norm_2_inv);
}

void quat_normalize(quat q)
{
#ifdef VEC_TYPE_DOUBLE 
	vec_type norm_2_inv = 1.0 / quat_norm(q);
#else
	vec_type norm_2_inv = 1.0f / quat_norm(q);
#endif
	quat_scale(q, norm_2_inv);
}

/**
 * CCW rot?
 */
void quat_to_mat3(mat3 dst, const quat q)
{

#ifdef VEC_TYPE_DOUBLE 
	const double tr_part = 2.0*q[3]*q[3] - 1.0;
	const double q12 = 2.0*q[0]*q[1];
	const double q13 = 2.0*q[0]*q[2];
	const double q10 = 2.0*q[0]*q[3];
	const double q23 = 2.0*q[1]*q[2];
	const double q20 = 2.0*q[1]*q[3];
	const double q30 = 2.0*q[2]*q[3];
	mat3_set(dst, tr_part + 2.0*q[0]*q[0], q12 + q30, q13 - q20,
		      q12 - q30, tr_part + 2.0*q[1]*q[1], q23 + q10,
		      q13 + q20, q23 - q10, tr_part + 2.0*q[2]*q[2]);
#else
	const float tr_part = 2.0f*q[3]*q[3] - 1.0f;
	const float q12 = 2.0f*q[0]*q[1];
	const float q13 = 2.0f*q[0]*q[2];
	const float q10 = 2.0f*q[0]*q[3];
	const float q23 = 2.0f*q[1]*q[2];
	const float q20 = 2.0f*q[1]*q[3];
	const float q30 = 2.0f*q[2]*q[3];
	mat3_set(dst, tr_part + 2.0f*q[0]*q[0], q12 + q30, q13 - q20,
		      q12 - q30, tr_part + 2.0f*q[1]*q[1], q23 + q10,
		      q13 + q20, q23 - q10, tr_part + 2.0f*q[2]*q[2]);
#endif
}

/**
 * q is a normalised quaternion representing a CCW rotation.
 */
void quat_to_mat4(mat4 dst, const quat q)
{
#ifdef VEC_TYPE_DOUBLE 
	const f64 tr_part = 2.0*q[3]*q[3] - 1.0;
	const f64 q12 = 2.0*q[0]*q[1];
	const f64 q13 = 2.0*q[0]*q[2];
	const f64 q10 = 2.0*q[0]*q[3];
	const f64 q23 = 2.0*q[1]*q[2];
	const f64 q20 = 2.0*q[1]*q[3];
	const f64 q30 = 2.0*q[2]*q[3];
	mat4_set(dst, tr_part + 2.0*q[0]*q[0], q12 + q30, q13 - q20, 0.0,
		      q12 - q30, tr_part + 2.0*q[1]*q[1], q23 + q10, 0.0,
		      q13 + q20, q23 - q10, tr_part + 2.0*q[2]*q[2], 0.0, 
		      0.0, 0.0, 0.0, 1.0);
#else
	const f32 tr_part = 2.0f*q[3]*q[3] - 1.0f;
	const f32 q12 = 2.0f*q[0]*q[1];
	const f32 q13 = 2.0f*q[0]*q[2];
	const f32 q10 = 2.0f*q[0]*q[3];
	const f32 q23 = 2.0f*q[1]*q[2];
	const f32 q20 = 2.0f*q[1]*q[3];
	const f32 q30 = 2.0f*q[2]*q[3];
	mat4_set(dst, tr_part + 2.0f*q[0]*q[0], q12 + q30, q13 - q20, 0.0f,
		      q12 - q30, tr_part + 2.0f*q[1]*q[1], q23 + q10, 0.0f,
		      q13 + q20, q23 - q10, tr_part + 2.0f*q[2]*q[2], 0.0f, 
		      0.0f, 0.0f, 0.0f, 1.0f);
#endif
}

void axis_angle_to_quaternion(quat dst, const vec3 axis, const vec_type angle)
{
#ifdef VEC_TYPE_DOUBLE 
	const double scale = sin(angle/2.0) * vec3_length(axis);
	quat_set(dst, scale * axis[0], scale * axis[1], scale * axis[2], cos(angle/2.0));
#else
	const float scale = sinf(angle/2.0f) * vec3_length(axis);
	quat_set(dst, scale * axis[0], scale * axis[1], scale * axis[2], cosf(angle/2.0f));
#endif
}

void unit_axis_angle_to_quaternion(quat dst, const vec3 axis, const vec_type angle)
{
#ifdef VEC_TYPE_DOUBLE 
	const double scale = sin(angle/2.0);
	quat_set(dst, scale * axis[0], scale * axis[1], scale * axis[2], cos(angle/2.0));
#else
	const float scale = sinf(angle/2.0f);
	quat_set(dst, scale * axis[0], scale * axis[1], scale * axis[2], cosf(angle/2.0f));
#endif
}
