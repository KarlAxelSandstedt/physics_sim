#include "r_public.h"
#include "r_local.h"

#ifdef MG_DEBUG
#include <stdio.h>

void camera_print(const struct camera *cam)
{
	printf("POS: (%f, %f, %f)\n", cam->position[0], cam->position[1], cam->position[2]);
	printf("RIGHT: (%f, %f, %f)\n", cam->left[0], cam->left[1], cam->left[2]);
	printf("UP: (%f, %f, %f)\n", cam->up[0], cam->up[1], cam->up[2]);
	printf("DIR: (%f, %f, %f)\n", cam->forward[0], cam->forward[1], cam->forward[2]);
	printf("ASPECT, FOV_X, FZ_NEAR, FZ_FAR: (%f, %f, %f, %f)\n", cam->aspect_ratio, cam->fov_x, cam->fz_near, cam->fz_far);
	printf("YAW, PITCH: (%f, %f)\n", cam->yaw, cam->pitch);
}

#endif

void camera_construct(struct camera *cam,
		const vec3 position,
	       	const vec3 left,
	       	const vec3 up,
	       	const vec3 forward,
		const f32 yaw,
		const f32 pitch,
	       	const f32 fz_near,
	       	const f32 fz_far,
	       	const f32 aspect_ratio,
	       	const f32 fov_x)
{
	assert(fov_x > 0.0f && fov_x < MM_PI_F);
	assert(fz_near > 0.0f);
	assert(fz_far > fz_near);
	assert(aspect_ratio > 0);
	
	vec3_copy(cam->position, position);
	vec3_copy(cam->left, left);
	vec3_copy(cam->up, up);
	vec3_copy(cam->forward, forward);
	cam->yaw = yaw;
	cam->pitch = pitch;
	cam->fz_near = fz_near;
	cam->fz_far = fz_far;
	cam->aspect_ratio = aspect_ratio;
	cam->fov_x = fov_x;
}

void camera_update_axes(struct camera *cam)
{
	vec3 left = {1.0f, 0.0f, 0.0f};
	vec3 up = {0.0f, 1.0f, 0.0f};
	vec3 forward = {0.0f, 0.0f, 1.0f};

	mat3 rot;
	sequential_rotation_matrix(rot, up, cam->yaw, left, cam->pitch);

	mat3_vec_mul(cam->left, rot, left);
	mat3_vec_mul(cam->up, rot, up);
	mat3_vec_mul(cam->forward, rot, forward);
}

void camera_update_angles(struct camera *cam, const f32 yaw_delta, const f32 pitch_delta)
{
	cam->yaw += yaw_delta;
	if (cam->yaw >= MM_PI_F)
	{
		cam->yaw -= MM_PI_2_F;
	}
	else if (cam->yaw <= -MM_PI_F)
	{
		cam->yaw += MM_PI_2_F;
	}

	if (cam->pitch + pitch_delta > MM_PI_F / 2.0f - 0.50f)
	{
		cam->pitch = MM_PI_F / 2.0f - 0.50f;
	}
	else if (cam->pitch + pitch_delta < 0.50f - MM_PI_F / 2.0f)
	{
		cam->pitch = 0.50f - MM_PI_F / 2.0f;
	}
	else 
	{
		cam->pitch += pitch_delta;
	}
}

void frustum_projection_plane_sides(f32 *width, f32 *height, const f32 plane_distance, const f32 fov_x, const f32 aspect_ratio)
{
	*width = 2.0f * plane_distance * tanf(fov_x / 2.0f);
	*height = *width / aspect_ratio;
}

void frustum_projection_plane_camera_space(vec3 bottom_left, vec3 upper_right, const struct camera *cam)
{
	f32 frustum_width, frustum_height;
	frustum_projection_plane_sides(&frustum_width, &frustum_height, cam->fz_near, cam->fov_x, cam->aspect_ratio);
	vec3_set(bottom_left, frustum_width / 2.0f, -frustum_height / 2.0f, cam->fz_near);
	vec3_set(upper_right, -frustum_width / 2.0f, frustum_height / 2.0f, cam->fz_near);
}

void frustum_projection_plane_world_space(vec3 bottom_left, vec3 upper_right, const struct camera *cam)
{
	f32 frustum_width, frustum_height;
	frustum_projection_plane_sides(&frustum_width, &frustum_height, cam->fz_near, cam->fov_x, cam->aspect_ratio);

	vec3 v;
	vec3 left = {1.0f, 0.0f, 0.0f};
	vec3 up = {0.0f, 1.0f, 0.0f};
	mat3 rot;
	sequential_rotation_matrix(rot, up, cam->yaw, left, cam->pitch);

	vec3_set(v, frustum_width / 2.0f, -frustum_height / 2.0f, cam->fz_near);
	mat3_vec_mul(bottom_left, rot, v);
	vec3_translate(bottom_left, cam->position);

	v[0] -= frustum_width;
	v[1] += frustum_height;
	mat3_vec_mul(upper_right, rot, v);
	vec3_translate(upper_right, cam->position);
}

void window_space_to_world_space(vec3 world_pixel, const vec2i32 pixel, const vec2i32 window_sides, const struct camera * cam)
{
	mat3 rot;
	vec3 bl, tr, camera_pixel;

	vec3 left = {1.0f, 0.0f, 0.0f};
	vec3 up = {0.0f, 1.0f, 0.0f};
	sequential_rotation_matrix(rot, up, cam->yaw, left, cam->pitch);

	const vec3 alphas = { 1.0f - ((f32) pixel[0]) / window_sides[0], 1.0f - ((f32) pixel[1]) / window_sides[1], 1.0f };	
	frustum_projection_plane_camera_space(bl, tr, cam);
	vec3_interpolate_piecewise(camera_pixel, bl, tr, alphas);	
	mat3_vec_mul(world_pixel, rot, camera_pixel);
	vec3_translate(world_pixel, cam->position);
}
