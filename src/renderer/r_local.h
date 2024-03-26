#ifndef __RENDERER_LOCAL_H__
#define __RENDERER_LOCAL_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "r_public.h"
#include "mgl.h"
#include "mmath.h"

/******************** r_init.c ********************/

void r_compile_shader(u32 *prg, const char *v_filepath, const char *f_filepath);	/* compile shader */

/******************** r_camera.c ********************/

#ifdef MG_DEBUG
void camera_print(const struct camera *cam);
#endif

/**
 * position - position in world
 * left, up, forward - normalized vectors representing orthogonal axes of camera
 * fov_x - field of view in radians
 * fz_near - distance to nearest frustum plane (AFFECTS Depth calculations, see Z-fighting)
 * fz_far - distance to farther frustum plane (AFFECTS Depth calculations, see Z-fighting)
 * aspect_ratio - w/h
 */
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
	       	const f32 fov_x);

void camera_update_axes(struct camera *cam);
void camera_update_angles(struct camera *cam, const f32 yaw_delta, const f32 pitch_delta);

/* Retrieve side lengths of frustum projection plane */
void frustum_projection_plane_sides(f32 *width, f32 *height, const f32 plane_distance, const f32 fov_x, const f32 aspect_ratio);
/* Retreive camera frustum plane in world space coordinates */
void frustum_projection_plane_world_space(vec3 bottom_left, vec3 upper_right, const struct camera *cam);

/* Retreive camera frustum plane in camera space coordinates */
void frustum_projection_plane_camera_space(vec3 bottom_left, vec3 upper_right, const struct camera *cam);

/* Transform window pixel position to its corresponding world space coordinate */
void window_space_to_world_space(vec3 world_pixel, const vec2i32 pixel, const vec2i32 window_sides, const struct camera * cam);

/******************** r_main.c ********************/

/******************** r_buffer.c ********************/

void drawbuffer_new(struct gl_state *state, struct drawbuffer *buf, const u64 vbo_size, const u64 ebo_size, const enum drawbuffer_type type, const u32 prg);
void drawbuffer_clear(struct drawbuffer *buf);
void drawbuffer_free(struct gl_state *state, struct drawbuffer *buf);
void drawbuffer_bind(struct gl_state *state, const struct drawbuffer *buf);
void drawbuffer_buffer_data(struct gl_state *state, const struct drawbuffer *buf);
void drawbuffer_draw(struct gl_state *state, const struct drawbuffer *buf, const GLenum mode);
void drawbuffer_draw_partial(struct gl_state *state, const struct drawbuffer *buf, const GLenum mode, const i32 i_count, const i32 i_offset);

/******************** r_primitive.c ********************/

void line_push(struct drawbuffer *buf, const vec3 v_1, const vec3 v_2, const vec4 color);
void line_push_random_color(struct drawbuffer *buf, const vec3 v_1, const vec3 v_2);

#endif
