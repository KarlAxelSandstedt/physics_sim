#include "r_local.h"
#include "r_public.h"

void line_push(struct drawbuffer *buf, const vec3 v_1, const vec3 v_2, const vec4 color)
{
	assert(buf->type == DRAWBUFFER_COLOR);

	vec2i32 i = { buf->next_index, buf->next_index + 1};
	buf->next_index += 2;

	arena_push_packed(&buf->v_buf, v_1, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	arena_push_packed(&buf->v_buf, v_2, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	arena_push_packed(&buf->i_buf, i, sizeof(vec2i32));
}

void line_push_random_color(struct drawbuffer *buf, const vec3 v_1, const vec3 v_2)
{
	vec4 color = { gen_rand_f(), gen_rand_f(), gen_rand_f(), 1.0f };
	line_push(buf, v_1, v_2, color);	
}
