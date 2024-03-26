#ifndef __MATH_TRANSFORMATION__
#define __MATH_TRANSFORMATION__

#include "matrix.h"


/* axes should be normalized */
void sequential_rotation_matrix(mat3 dst, const vec3 axis_1, const f32 angle_1, const vec3 axis_2, const f32 angle_2); /* rotation matrix of axis_1(angle_1) (R) -> [R(axis_2)](angle_2) */
void perspective_matrix(mat4 dst, const vec_type aspect_ratio, const vec_type fov_x, const vec_type fz_near, const vec_type fz_far);
void view_matrix(mat4 dst, const vec3 position, const vec3 left, const vec3 up, const vec3 forward);
void view_matrix_look_at(mat4 dst, const vec3 position, const vec3 target);
void view_matrix_yaw_pitch(mat4 dst, const vec3 position, const vec_type yaw, const vec_type pitch);
void rotation_matrix(mat3 dst, const vec3 axis, const vec_type angle);
void vec3_rotate_center(vec3 src_rotated, mat3 rotation, const vec3 center, const vec3 src);

#endif
