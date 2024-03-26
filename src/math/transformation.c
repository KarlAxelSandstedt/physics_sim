#include "mmath.h"

void sequential_rotation_matrix(mat3 dst, const vec3 axis_1, const f32 angle_1, const vec3 axis_2, const f32 angle_2)
{
	vec3 axis_snd;
	mat3 r_1, r_2;
	rotation_matrix(r_1, axis_1, angle_1);
	mat3_vec_mul(axis_snd, r_1, axis_2);
	rotation_matrix(r_2, axis_snd, angle_2);
	mat3_mult(dst, r_2, r_1);
}

void rotation_matrix(mat3 dst, const vec3 axis, const vec_type angle)
{
#ifdef VEC_TYPE_DOUBLE 
	const f64 w = cos(angle / 2.0);
	vec3 pure_quat;
	vec3_scale(pure_quat, axis, sin(angle / 2.0));

	const f64 tr_part = 2.0*w*w - 1.0;
	const f64 q12 = 2.0*pure_quat[0]*pure_quat[1];
	const f64 q13 = 2.0*pure_quat[0]*pure_quat[2];
	const f64 q10 = 2.0*pure_quat[0]*w;
	const f64 q23 = 2.0*pure_quat[1]*pure_quat[2];
	const f64 q20 = 2.0*pure_quat[1]*w;
	const f64 q30 = 2.0*pure_quat[2]*w;
	mat3_set(dst, tr_part + 2.0*pure_quat[0]*pure_quat[0], q12 + q30, q13 - q20,
		      q12 - q30, tr_part + 2.0*pure_quat[1]*pure_quat[1], q23 + q10,
		      q13 + q20, q23 - q10, tr_part + 2.0*pure_quat[2]*pure_quat[2]);
#else
	const f32 w = cosf(angle / 2.0f);
	vec3 pure_quat;
	vec3_scale(pure_quat, axis, sinf(angle / 2.0f));

	const f32 tr_part = 2.0f*w*w - 1.0f;
	const f32 q12 = 2.0f*pure_quat[0]*pure_quat[1];
	const f32 q13 = 2.0f*pure_quat[0]*pure_quat[2];
	const f32 q10 = 2.0f*pure_quat[0]*w;
	const f32 q23 = 2.0f*pure_quat[1]*pure_quat[2];
	const f32 q20 = 2.0f*pure_quat[1]*w;
	const f32 q30 = 2.0f*pure_quat[2]*w;
	mat3_set(dst, tr_part + 2.0f*pure_quat[0]*pure_quat[0], q12 + q30, q13 - q20,
		      q12 - q30, tr_part + 2.0f*pure_quat[1]*pure_quat[1], q23 + q10,
		      q13 + q20, q23 - q10, tr_part + 2.0f*pure_quat[2]*pure_quat[2]);
#endif
}

void vec3_rotate_center(vec3 src_rotated, mat3 rotation, const vec3 center, const vec3 src)
{
	vec3 tmp;
	vec3_sub(src_rotated, src, center);
	mat3_vec_mul(tmp, rotation, src_rotated);
	vec3_add(src_rotated, tmp, center);
}

void perspective_matrix(mat4 dst, const vec_type aspect_ratio, const vec_type fov_x, const vec_type fz_near, const vec_type fz_far)
{
#ifdef VEC_TYPE_DOUBLE 
	mat4_set(dst, 
		     1.0 / tan(fov_x / 2.0), 0.0, 0.0, 0.0,
	             0.0, aspect_ratio / tan(fov_x / 2.0), 0.0, 0.0,
		     0.0, 0.0, (fz_near + fz_far) / (fz_near - fz_far), -1.0,
		     0.0, 0.0, (2.0 * fz_near * fz_far) / (fz_near - fz_far), 0.0);
#else
	mat4_set(dst, 
		     1.0f / tanf(fov_x / 2.0f), 0.0f, 0.0f, 0.0f,
	             0.0f, aspect_ratio / tanf(fov_x / 2.0f), 0.0f, 0.0f,
		     0.0f, 0.0f, (fz_near + fz_far) / (fz_near - fz_far), -1.0f,
		     0.0f, 0.0f, (2.0f * fz_near * fz_far) / (fz_near - fz_far), 0.0f);
#endif
}

void view_matrix(mat4 dst, const vec3 position, const vec3 left, const vec3 up, const vec3 forward)
{
#ifdef VEC_TYPE_DOUBLE 
	mat4 basis_change, translation;
	mat4_set(basis_change,
			-left[0], up[0], -forward[0], 0.0,
			-left[1], up[1], -forward[1], 0.0,
			-left[2], up[2], -forward[2], 0.0,
			0.0, 0.0, 0.0, 1.0);
	mat4_set(translation,
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			-position[0], -position[1], -position[2], 1.0);
	mat4_mult(dst, basis_change, translation);
#else
	/**
	 * (1) Translation to camera center 
	 * (2) Change to Camera basis
	 * (3) anything infront of camera must be reflected in x,z values againt (0,0), since
	 * Opengl expects camera looking down -Z axis, so mult left, and forward axes by (-1) .
	 */
	mat4 basis_change, translation;
	mat4_set(basis_change,
			-left[0], up[0], -forward[0], 0.0f,
			-left[1], up[1], -forward[1], 0.0f,
			-left[2], up[2], -forward[2], 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	mat4_set(translation,
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-position[0], -position[1], -position[2], 1.0f);
	mat4_mult(dst, basis_change, translation);
#endif
}

void view_matrix_look_at(mat4 dst, const vec3 position, const vec3 target)
{
	vec3 tmp, relative, dir;
	vec3_sub(relative, target, position);
	vec3_normalize(dir, relative);
#ifdef VEC_TYPE_DOUBLE
	vec3_set(tmp, 0.0, 1.0, 0.0);
	const vec_type pitch = MM_PI_F / 2.0 - acos(vec3_dot(tmp, dir));

	relative[1] = 0.0;
	vec3_normalize(dir, relative);
	vec3_set(tmp, 1.0, 0.0, 0.0);

	vec_type yaw;
	if (dir[2] > 0.0) {
		yaw  = acos(vec3_dot(tmp, dir));	
	} else {
		yaw  = -acos(vec3_dot(tmp, dir));	
	}
#else
	vec3_set(tmp, 0.0f, 1.0f, 0.0f);
	const f32 pitch = MM_PI_F / 2.0f - acosf(vec3_dot(tmp, dir));

	relative[1] = 0.0f;
	vec3_normalize(dir, relative);
	vec3_set(tmp, 1.0f, 0.0f, 0.0f);

	f32 yaw;
	if (dir[2] < 0.0f) {
		yaw  = acosf(vec3_dot(tmp, dir));	
	} else {
		yaw  = -acosf(vec3_dot(tmp, dir));	
	}
#endif
	view_matrix_yaw_pitch(dst, position, yaw, pitch);
}

void view_matrix_yaw_pitch(mat4 dst, const vec3 position, const vec_type yaw, const vec_type pitch)
{
	vec3 left, up, forward, tmp;
	mat3 rot;
	quat q;
#ifdef VEC_TYPE_DOUBLE 
	const vec_type cy = cos(yaw / 2.0);
	const vec_type cp = cos(pitch / 2.0);
	const vec_type sy = sin(yaw / 2.0);
	const vec_type sp = sin(pitch / 2.0);
	quat_set(q, sy*sp, sy*cp, cy*sp, cy*cp);
	quat_to_mat3(rot, q);

	vec3_set(tmp, 1.0, 0.0, 0.0);
	mat3_vec_mult(left, tmp, rot);

	vec3_set(tmp, 0.0, 1.0, 0.0);
	mat3_vec_mult(up, tmp, rot);

	vec3_set(tmp, 0.0, 0.0, 1.0);
	mat3_vec_mult(forward, tmp, rot);

	mat4 basis_change, translation;
	mat4_set(basis_change,
			-left[0], up[0], -forward[0], 0.0,
			-left[1], up[1], -forward[1], 0.0,
			-left[2], up[2], -forward[2], 0.0,
			0.0, 0.0, 0.0, 1.0);
	mat4_set(translation,
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			-position[0], -position[1], -position[2], 1.0);
	mat4_mult(dst, basis_change, translation);
#else
	const f32 cy = cosf(yaw / 2.0f);
	const f32 cp = cosf(pitch / 2.0f);
	const f32 sy = sinf(yaw / 2.0f);
	const f32 sp = sinf(pitch / 2.0f);
	quat_set(q, sy*sp, sy*cp, cy*sp, cy*cp);
	quat_to_mat3(rot, q);

	/* Assume no rotation is equivalent to looking down positive x-axis */
	vec3_set(tmp, 0.0f, 0.0f, -1.0f);
	mat3_vec_mul(left, rot, tmp);

	vec3_set(tmp, 0.0f, 1.0f, 0.0f);
	mat3_vec_mul(up, rot, tmp);

	vec3_set(tmp, 1.0f, 0.0f, 0.0f);
	mat3_vec_mul(forward, rot, tmp);

	mat4 basis_change, translation;
	mat4_set(basis_change,
			-left[0], up[0], -forward[0], 0.0f,
			-left[1], up[1], -forward[1], 0.0f,
			-left[2], up[2], -forward[2], 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	mat4_set(translation,
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			-position[0], -position[1], -position[2], 1.0f);
	mat4_mult(dst, basis_change, translation);
#endif
}
