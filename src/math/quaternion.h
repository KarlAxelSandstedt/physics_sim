#ifndef __QUATERNION_H__
#define __QUATERNION_H__

/**
 * QUATERNION RULES:
 *	
 *		A Y
 *		|
 *		|
 *		.------> X
 *	       /
 *	      L Z
 *
 *	      i^2 = j^2 = k^2 = -1
 *
 *	      (point i,j,k) * (axis i,j,k) = CW rotation [rules for i,j,k multiplication]
 *
 *	      ij =  k
 *	      ji = -k
 *	      ik = -j
 *	      ki =  j
 *	      jk =  i
 *	      kj = -i
 */

#ifdef VEC_TYPE_DOUBLE 
typedef double vec_type;
#else
typedef float vec_type;
#endif

/* { x, y, z, w }, w is real part */
typedef vec_type quat[4];

/* Quaternion Creation */
/*(axis does not have to be normalized) */
void axis_angle_to_quaternion(quat dst, const vec3 axis, const vec_type angle);
/*(axis have to be normalized) */
void unit_axis_angle_to_quaternion(quat dst, const vec3 axis, const vec_type angle);

/* Quaternion Operations and Functions */
void quat_set(quat dst, const vec_type x, const vec_type y, const vec_type z, const vec_type w);
void quat_add(quat dst, const quat p, const quat q);
void quat_sub(quat dst, const quat p, const quat q);
void quat_mult(quat dst, const quat p, const quat q);
void quat_scale(quat dst, const vec_type scale);
void quat_copy(quat dst, const quat src);
void quat_conj(quat conj, const quat q);
void quat_inv(quat inv, const quat q);
vec_type quat_norm(const quat q);
void quat_normalize(quat q);

/**
 * Quaternion Rotation operation matrix Q in: qvq* = Qv
 * q = [cos(t/2), sin(t/2)v] where |v| = 1 and v is the rotation axis. t is wanted angle rotation. For some point
 * v, the achieved rotation is calculated as qvq*.
 */
void quat_to_mat3(mat3 dst, const quat q);
void quat_to_mat4(mat4 dst, const quat q);

#endif
