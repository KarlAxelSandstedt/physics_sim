#include "mgl_primitives.h"

void mgl_quad_centered_at(struct mgl_quad *quad, const vec3 center, const vec3 normal, const vec2 side_lengths, const vec4 color)
{
	const vec3 left = { 1.0f, 0.0f, 0.0f };
	const vec3 up = { 0.0f, 1.0f, 0.0f };
	vec3 tmp = { normal[0], 0.0f, normal[2] };
	vec3 xz;
	vec3_normalize(xz, tmp);
	f32 angle_1, angle_2;
	if (tmp[0] == 0 && tmp[2] == 0)
	{
		angle_1 = 0.0f;	
	}
	else
	{
		if (xz[2] > 0.0f)
		{
			angle_1 = -acosf(vec3_dot(xz, left)) + MM_PI_F / 2.0f;
		}
		else
		{
			angle_1 = acosf(vec3_dot(xz, left)) -  MM_PI_F / 2.0f;
		}
	}

	if (normal[2] > 0.0f)
	{
		angle_2 = acosf(vec3_dot(normal, up)) - MM_PI_F / 2.0f;
	}
	else
	{
		angle_2 = MM_PI_F / 2.0f - acosf(vec3_dot(normal, up));
	}
	
	mat3 rot;
	sequential_rotation_matrix(rot, up, angle_1, left, angle_2);

	vec4_copy(quad->points[0].c, color);
	vec4_copy(quad->points[1].c, color);
	vec4_copy(quad->points[2].c, color);
	vec4_copy(quad->points[3].c, color);

	if (normal[2] > 0.0f)
	{
		vec3_set(tmp, side_lengths[0] / 2.0f, -side_lengths[1] / 2.0f, 0.0f);
		mat3_vec_mul(quad->points[0].p, rot, tmp);
		tmp[1] += side_lengths[1];
		mat3_vec_mul(quad->points[1].p, rot, tmp);
		tmp[0] -= side_lengths[0];
		mat3_vec_mul(quad->points[2].p, rot, tmp);
		tmp[1] -= side_lengths[1];
		mat3_vec_mul(quad->points[3].p, rot, tmp);
	}
	else
	{
		vec3_set(tmp, -side_lengths[0] / 2.0f, -side_lengths[1] / 2.0f, 0.0f);
		mat3_vec_mul(quad->points[0].p, rot, tmp);
		tmp[1] += side_lengths[1];
		mat3_vec_mul(quad->points[1].p, rot, tmp);
		tmp[0] += side_lengths[0];
		mat3_vec_mul(quad->points[2].p, rot, tmp);
		tmp[1] -= side_lengths[1];
		mat3_vec_mul(quad->points[3].p, rot, tmp);
	}

	vec3_translate(quad->points[0].p, center);
	vec3_translate(quad->points[1].p, center);
	vec3_translate(quad->points[2].p, center);
	vec3_translate(quad->points[3].p, center);
}
