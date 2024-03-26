#ifndef __RENDERER_PUBLIC_H__
#define __RENDERER_PUBLIC_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "r_common.h"
#include "system_public.h"
#include "sim_public.h"
#include "widget.h"

/******************** r_init.c ********************/

struct render_state *r_init(struct arena *mem, struct graphics_context *gtx);

/******************** r_camera.c ********************/

struct camera {
	vec3 position;
	vec3 up;
	vec3 forward;
	vec3 left;
	f32 yaw;
	f32 pitch;
	f32 fz_near;
	f32 fz_far;
	f32 aspect_ratio;
	f32 fov_x;
};

/******************** r_buffer.c ********************/

/******************** r_main.c ********************/

struct gl_state
{
	u32 vao_bound;
	u32 vbo_bound;
	u32 ebo_bound;
	u32 prg_bound;
};

struct render_state
{
	struct gl_state gl;

	struct camera cam;

	struct drawbuffer entity_buf;
	struct drawbuffer color_buf;
	struct drawbuffer widget_buf;

	u32 lightning_prg;
	u32 widget_prg;
	u32 color_prg;
};

void r_main(struct render_state *re, struct graphics_context *gtx, struct simulation *sim, struct ui_state *ui, const f64 delta);

#endif
