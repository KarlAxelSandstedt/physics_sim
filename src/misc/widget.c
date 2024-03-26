#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "widget.h"
#include "mmath.h"
#include "sort.h"

const char *no_active = "NO_ACTIVE";
const char *no_hot = "NO_HOT";

void ui_unit_push(struct ui_state *context, struct ui_unit *parent)
{
	context->stack_index += 1;
	assert(context->stack_index < UI_DEPTH_MAX);
	context->unit_stack[context->stack_index] = parent;
}

void ui_unit_pop(struct ui_state *context)
{
	assert(context->stack_index != UINT32_MAX);
	context->stack_index -= 1;
}

/* Return a.depth - b.depth */
static i32 ui_internal_depth_compare(const void *a, const void *b)
{
	const struct ui_unit_depth *al = a;
	const struct ui_unit_depth *bl = b;
	return al->depth - bl->depth;
}

void ui_unit_depth_sort(struct arena *mem, struct ui_state *context)
{
	if (context->unit_depth->max_used != UINT64_MAX) {
		mergesort(mem, context->unit_depth->data, context->unit_depth->max_used + 1, sizeof(struct ui_unit_depth), &ui_internal_depth_compare);

		for (i32 i = 0; i <= (i32) context->units->max_used; ++i) {
			const struct ui_unit_depth * unit_depth = d_array_get(context->unit_depth, i);
			const struct ui_unit *unit = d_array_get(context->units, unit_depth->index);

			context->depth_batch[unit_depth->depth] += 1;
			if (unit->text) {
				context->depth_text_batch[unit_depth->depth] += (i32) unit->text_len;
			}
		}
	}
}

struct ui_state *ui_context_init(struct arena *mem, const i32 granularity)
{
	struct ui_state *utx;
	struct ui_state cpy = 
	{
		.hash = hash_new(mem, granularity, granularity),
		.units = d_array_new(mem, granularity, sizeof(struct ui_unit), granularity),
		.unit_depth = d_array_new(mem, granularity, sizeof(struct ui_unit_depth), granularity),
		.stack_index = UINT32_MAX,
		.depth = -1,
		.active = no_active,
		.hot = no_hot,
	};

	memset(&cpy.visual, 0, sizeof(cpy.visual));
	memset(&cpy.depth_batch, 0, sizeof(cpy.depth_batch));
	memset(&cpy.depth_text_batch, 0, sizeof(cpy.depth_text_batch));
	memset(&cpy.comm, 0, sizeof(cpy.comm));

	if (mem)
	{
		utx = arena_push(mem, &cpy, sizeof(cpy));
	}
	else
	{
		utx = malloc(sizeof(cpy));
		memcpy(utx, &cpy, sizeof(cpy));
	}

	return utx;
}

struct ui_state ui_context_create(const i32 granularity)
{
	assert(is_power_of_two(granularity));

	struct ui_state context = 
	{
		.hash = hash_new(NULL, granularity, granularity),
		.units = d_array_new(NULL, granularity, sizeof(struct ui_unit), granularity),
		.unit_depth = d_array_new(NULL, granularity, sizeof(struct ui_unit_depth), granularity),
		.stack_index = UINT32_MAX,
		.depth = -1,
		.active = no_active,
		.hot = no_hot,
	};

	memset(&context.visual, 0, sizeof(context.visual));
	memset(&context.depth_batch, 0, sizeof(context.depth_batch));
	memset(&context.depth_text_batch, 0, sizeof(context.depth_text_batch));
	memset(&context.comm, 0, sizeof(context.comm));

	return context;
}

void ui_context_destroy(struct ui_state *context)
{
	hash_free(context->hash);
	d_array_free(context->units);
	d_array_free(context->unit_depth);
}

i32 ui_unit_lookup(const struct ui_state *context, const char *id)
{
	const i32 key = hash_generate_key_str(id);
	i32 index;
	for (index = hash_first(context->hash, key); index != -1; index = hash_next(context->hash, index)) {
		struct ui_unit *unit = d_array_get(context->units, index);
		if (strcmp(id, unit->id) == 0) {
			break;
		}
	}

	return index;
}

struct ui_unit *ui_unit_create(struct ui_state *context, const char *id, u32 flags)
{
	assert(ui_unit_lookup(context, id) == -1);

	struct ui_unit unit = 
	{
		.parent = NULL,
		.prev = NULL,
		.next = NULL,
		.first = NULL,
		.last = NULL,
		.id = id,
		.key = hash_generate_key_str(id),
		.flags = flags,
		.text = NULL,
	};

	d_array_add(context->units, &unit);
	const i32 index = (i32) context->units->max_used;
	hash_add(context->hash, unit.key, index);
	struct ui_unit *unit_ptr = d_array_get(context->units, index);

	struct ui_unit_depth depth =
	{
		.index = index,
		.depth = 0,
	};

	/* new unit is a root unit */
	if (context->stack_index != UINT32_MAX) {
		struct ui_unit *parent = context->unit_stack[context->stack_index];
		unit_ptr->parent = parent;
		unit_ptr->prev = parent->last;
		if (parent->last) {
			parent->last->next = unit_ptr;
		} else {
			parent->first = unit_ptr;
		}
		parent->last = unit_ptr;

		const size_t parent_index = d_array_addr_to_index(context->units, parent);
		const struct ui_unit_depth *parent_depth = d_array_get(context->unit_depth, parent_index);
		depth.depth = parent_depth->depth + 1;
	}
	
	d_array_add(context->unit_depth, &depth);
	(context->depth < depth.depth) ? context->depth = depth.depth :  0;

	if (unit_ptr->flags | UNIT_DRAW_BORDER) {
		vec4_copy(unit_ptr->border_color, context->visual.border_color[context->visual.border_color_index]);
	}
	if (unit_ptr->flags | UNIT_DRAW_BACKGROUND) {
		vec4_copy(unit_ptr->background_color, context->visual.background_color[context->visual.background_color_index]);
	}
	if (unit_ptr->flags | UNIT_DRAW_TEXT) {
		vec4_copy(unit_ptr->text_color, context->visual.text_color[context->visual.text_color_index]);
	}

	return unit_ptr;
}

void ui_visual_set_default(struct ui_state *context, const vec4 background_color, const vec4 border_color, const vec4 text_color)
{
	vec4_copy(context->visual.background_color[0], background_color);
	vec4_copy(context->visual.border_color[0], border_color);
	vec4_copy(context->visual.text_color[0], text_color);
	context->visual.background_color_index = 0;
	context->visual.border_color_index = 0;
	context->visual.text_color_index = 0;
}

void ui_push_background_color(struct ui_state *context, const vec4 color)
{
	(context->visual.background_color_index < VISUAL_DEPTH_MAX - 1) ? context->visual.background_color_index += 1 : 0;
	vec4_copy(context->visual.background_color[context->visual.background_color_index], color);
}

void ui_pop_background_color(struct ui_state *context)
{
	(context->visual.background_color_index > 0) ? context->visual.background_color_index -= 1 : 0;
}

void ui_push_border_color(struct ui_state *context, const vec4 color)
{
	(context->visual.border_color_index < VISUAL_DEPTH_MAX - 1) ? context->visual.border_color_index += 1 : 0;
	vec4_copy(context->visual.border_color[context->visual.border_color_index], color);
}

void ui_pop_border_color(struct ui_state *context)
{
	(context->visual.border_color_index > 0) ? context->visual.border_color_index -= 1 : 0;
}

void ui_push_text_color(struct ui_state *context, const vec4 color)
{
	(context->visual.text_color_index < VISUAL_DEPTH_MAX - 1) ? context->visual.text_color_index += 1 : 0;
	vec4_copy(context->visual.text_color[context->visual.text_color_index], color);
}

void ui_pop_text_color(struct ui_state *context)
{
	(context->visual.text_color_index > 0) ? context->visual.text_color_index -= 1 : 0;
}

size_t ui_drawbuffer_stride(void)
{
	const size_t stride =
	{
		  sizeof(vec2)	/* coordinate */
		+ sizeof(vec2)	/* uv */
		+ sizeof(vec4)	/* background_color */
		+ sizeof(vec4)  /* border_color */
		+ sizeof(vec4)	/* text_color */ 
		+ sizeof(vec2)	/* rect_center */ 
		+ sizeof(f32)	/* corner_radius */ 
		+ sizeof(f32)	/* edge_softness */ 
		+ sizeof(f32)	/* border_thickness */ 
	};

	return stride;
}

size_t ui_drawbuffer_size(const struct ui_state *context)
{
	i32 chars_to_render = 0;
	for (i32 i = 0; i <= context->depth; ++i) {
		chars_to_render += context->depth_text_batch[i];
	}
	return (chars_to_render + context->units->max_used + 1) * ui_drawbuffer_stride() * 4;
}

size_t ui_indexbuffer_size(const struct ui_state *context)
{
	i32 chars_to_render = 0;
	for (i32 i = 0; i <= context->depth; ++i) {
		chars_to_render += context->depth_text_batch[i];
	}
	return (chars_to_render + context->units->max_used + 1) * 6 * sizeof(i32);
}

const size_t position_offset = 0;
const size_t uv_offset = 1 * sizeof(vec2);
const size_t background_color_offset = 2 * sizeof(vec2);
const size_t border_color_offset = 2 * sizeof(vec2) + 1 * sizeof(vec4);
const size_t text_color_offset = 2 * sizeof(vec2) + 2 * sizeof(vec4);
const size_t center_offset = 2 * sizeof(vec2) + 3 * sizeof(vec4);
const size_t corner_radius_offset = 3 * sizeof(vec2) + 3 * sizeof(vec4);
const size_t edge_softness_offset = 3 * sizeof(vec2) + 3 * sizeof(vec4) + 1 * sizeof(f32);
const size_t border_thickness_offset = 3 * sizeof(vec2) + 3 * sizeof(vec4) + 2 * sizeof(f32);

void ui_internal_text_to_renderdata(u8 *drawbuf, i32 *ibuf, const size_t stride, const i32 min_index, const struct mg_font *font, const vec2 screen_size, const u32 *text, const u32 len, const vec2 center, const vec4 text_color)
{
	const i32 first_char = font->first_char;
	i32 x_offset = font->codepoint_table[text[0] - first_char].lsb;

	const f32 corner_radius = 1.0f;
	const f32 edge_softness = 1.0f;
	const f32 border_thickness = 1.0f;

	const vec4 color_default = { 0.0f, 0.0f, 0.0f, 0.0f };
	vec2 position;
	vec2 uv;
	for (u32 i = 0; i < len; ++i)
	{
		const struct codepoint_info *cinfo = &font->codepoint_table[text[i] - first_char];

		ibuf[i*6 + 0] = min_index + i*4 + 0;
		ibuf[i*6 + 1] = min_index + i*4 + 1;
		ibuf[i*6 + 2] = min_index + i*4 + 2;
		ibuf[i*6 + 3] = min_index + i*4 + 0;
		ibuf[i*6 + 4] = min_index + i*4 + 2;
		ibuf[i*6 + 5] = min_index + i*4 + 3;

		vec2_set(position, x_offset + cinfo->width, cinfo->y_offset);
		vec2_set(uv, cinfo->a_x + cinfo->a_width, cinfo->a_y);
		memcpy(drawbuf + (4 * i + 0) * stride + position_offset, position, sizeof(position));
		memcpy(drawbuf + (4 * i + 0) * stride + uv_offset, uv, sizeof(uv));
		memcpy(drawbuf + (4 * i + 0) * stride + background_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 0) * stride + border_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 0) * stride + text_color_offset, text_color, sizeof(vec4));
		memcpy(drawbuf + (4 * i + 0) * stride + center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + (4 * i + 0) * stride + corner_radius_offset  	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + (4 * i + 0) * stride + edge_softness_offset  	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + (4 * i + 0) * stride + border_thickness_offset	, &border_thickness, sizeof(f32));


		position[1] += cinfo->height;
		uv[1] += cinfo->a_height;
		memcpy(drawbuf + (4 * i + 1) * stride + position_offset, position, sizeof(position));
		memcpy(drawbuf + (4 * i + 1) * stride + uv_offset, uv, sizeof(uv));
		memcpy(drawbuf + (4 * i + 1) * stride + background_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 1) * stride + border_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 1) * stride + text_color_offset, text_color, sizeof(vec4));
		memcpy(drawbuf + (4 * i + 1) * stride + center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + (4 * i + 1) * stride + corner_radius_offset  	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + (4 * i + 1) * stride + edge_softness_offset  	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + (4 * i + 1) * stride + border_thickness_offset	, &border_thickness, sizeof(f32));

		position[0] -= cinfo->width;
		uv[0] -= cinfo->a_width;
		memcpy(drawbuf + (4 * i + 2) * stride + position_offset, position, sizeof(position));
		memcpy(drawbuf + (4 * i + 2) * stride + uv_offset, uv, sizeof(uv));
		memcpy(drawbuf + (4 * i + 2) * stride + background_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 2) * stride + border_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 2) * stride + text_color_offset, text_color, sizeof(vec4));
		memcpy(drawbuf + (4 * i + 2) * stride + center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + (4 * i + 2) * stride + corner_radius_offset  	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + (4 * i + 2) * stride + edge_softness_offset  	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + (4 * i + 2) * stride + border_thickness_offset	, &border_thickness, sizeof(f32));

		position[1] -= cinfo->height;
		uv[1] -= cinfo->a_height;
		memcpy(drawbuf + (4 * i + 3) * stride + position_offset, position, sizeof(position));
		memcpy(drawbuf + (4 * i + 3) * stride + uv_offset, uv, sizeof(uv));
		memcpy(drawbuf + (4 * i + 3) * stride + background_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 3) * stride + border_color_offset, color_default, sizeof(color_default));
		memcpy(drawbuf + (4 * i + 3) * stride + text_color_offset, text_color, sizeof(vec4));
		memcpy(drawbuf + (4 * i + 3) * stride + center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + (4 * i + 3) * stride + corner_radius_offset  	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + (4 * i + 3) * stride + edge_softness_offset  	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + (4 * i + 3) * stride + border_thickness_offset	, &border_thickness, sizeof(f32));

		x_offset += cinfo->advance;
	}

	vec2 text_offset;
	vec2_set(text_offset, - (f32) x_offset / 2.0f, 0.0f);

	const f32 aspect_ratio = 1280.0f / 720.0f;
	for (u32 i = 0; i < 4 * len; ++i) {
		float *ptr = (float *) (drawbuf + i * stride + position_offset);
		vec2_translate(ptr, text_offset);
		ptr[0] = aspect_ratio * ptr[0] / screen_size[0];
		ptr[1] = ptr[1] / screen_size[1];
		vec2_translate(ptr, center);
	}
}

void ui_drawbuffer_data(struct ui_state *context, u8 *drawbuf, i32 *ibuf, const struct mg_font *font, const vec2 screen_size)
{
	const size_t stride = ui_drawbuffer_stride();

	i32 level = -1;
	size_t ui_position = 0;
	size_t text_position = 0;

	/* Save ' ' uv coordinate for texturing non-text rectangles */
	vec2 uv_br, uv_tr, uv_tl, uv_bl;
	vec4 nontext_color = { 0.0f, 0.0f, 0.0f, 0.0f };
	const i32 first_char = font->first_char;
	const struct codepoint_info *cinfo = &font->codepoint_table[' ' - first_char];
	vec2_set(uv_br, cinfo->a_x + cinfo->a_width, cinfo->a_y);
	vec2_set(uv_tr, cinfo->a_x + cinfo->a_width, cinfo->a_y + cinfo->a_height);
	vec2_set(uv_tl, cinfo->a_x, cinfo->a_y + cinfo->a_height);
	vec2_set(uv_bl, cinfo->a_x, cinfo->a_y);

	for (i32 i = 0; i <= (i32) context->units->max_used; ++i) {
		const struct ui_unit_depth * unit_depth = d_array_get(context->unit_depth, i);
		const struct ui_unit *unit = d_array_get(context->units, unit_depth->index);
		
		if (level < unit_depth->depth) {
			level = unit_depth->depth;
			ui_position = text_position;
			text_position += context->depth_batch[level];
		} else {
			ui_position += 1;
		}

		const size_t ui_offset = 4 * ui_position * stride;

		vec2 p_tr, p_bl, center;
		vec2_set(p_tr, unit->p_br[0], unit->p_tl[1]);
		vec2_set(p_bl, unit->p_tl[0], unit->p_br[1]);
		vec2_set(center, (p_tr[0] + p_bl[0]) / 2.0f, (p_tr[1] + p_bl[1]) / 2.0f);
		const f32 corner_radius = 1.0f;
		const f32 edge_softness = 1.0f;
		const f32 border_thickness = 1.0f;

		memcpy(drawbuf + ui_offset + position_offset 		, unit->p_br,  sizeof(vec2));
		memcpy(drawbuf + ui_offset + uv_offset 			, uv_br,  sizeof(vec2));
		memcpy(drawbuf + ui_offset + background_color_offset 	, unit->background_color, sizeof(vec4));
		memcpy(drawbuf + ui_offset + border_color_offset 	, unit->border_color, sizeof(vec4));
		memcpy(drawbuf + ui_offset + text_color_offset 		, nontext_color, sizeof(vec4));
		memcpy(drawbuf + ui_offset + center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + ui_offset + corner_radius_offset 	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + ui_offset + edge_softness_offset 	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + ui_offset + border_thickness_offset 	, &border_thickness, sizeof(f32));

		memcpy(drawbuf + stride + ui_offset +  position_offset 		, p_tr,  sizeof(vec2));		
		memcpy(drawbuf + stride + ui_offset +  uv_offset 		, uv_tr,  sizeof(vec2));		
		memcpy(drawbuf + stride + ui_offset +  background_color_offset 	, unit->background_color, sizeof(vec4));
		memcpy(drawbuf + stride + ui_offset +  border_color_offset 	, unit->border_color, sizeof(vec4));	
		memcpy(drawbuf + stride + ui_offset +  text_color_offset 	, nontext_color, sizeof(vec4));
		memcpy(drawbuf + stride + ui_offset +  center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + stride + ui_offset +  corner_radius_offset 	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + stride + ui_offset +  edge_softness_offset 	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + stride + ui_offset +  border_thickness_offset 	, &border_thickness, sizeof(f32));

		memcpy(drawbuf + stride * 2 + ui_offset	+ position_offset		, unit->p_tl,  sizeof(vec2));		
		memcpy(drawbuf + stride * 2 + ui_offset	+ uv_offset			, uv_tl,  sizeof(vec2));		
		memcpy(drawbuf + stride * 2 + ui_offset + background_color_offset 	, unit->background_color, sizeof(vec4));
		memcpy(drawbuf + stride * 2 + ui_offset + border_color_offset 	 	, unit->border_color, sizeof(vec4));
		memcpy(drawbuf + stride * 2 + ui_offset + text_color_offset 		, nontext_color, sizeof(vec4));
		memcpy(drawbuf + stride * 2 + ui_offset + center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + stride * 2 + ui_offset + corner_radius_offset 	 	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + stride * 2 + ui_offset + edge_softness_offset 	 	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + stride * 2 + ui_offset + border_thickness_offset 	, &border_thickness, sizeof(f32));

		memcpy(drawbuf + stride * 3 + ui_offset	+ position_offset		, p_bl,  sizeof(vec2));
		memcpy(drawbuf + stride * 3 + ui_offset	+ uv_offset			, uv_bl,  sizeof(vec2));
		memcpy(drawbuf + stride * 3 + ui_offset + background_color_offset 	, unit->background_color, sizeof(vec4));
		memcpy(drawbuf + stride * 3 + ui_offset + border_color_offset 	 	, unit->border_color, sizeof(vec4));
		memcpy(drawbuf + stride * 3 + ui_offset + text_color_offset 		, nontext_color, sizeof(vec4));
		memcpy(drawbuf + stride * 3 + ui_offset + center_offset 		, center, sizeof(vec2));
		memcpy(drawbuf + stride * 3 + ui_offset + corner_radius_offset 	 	, &corner_radius, sizeof(f32));
		memcpy(drawbuf + stride * 3 + ui_offset + edge_softness_offset 	 	, &edge_softness, sizeof(f32));
		memcpy(drawbuf + stride * 3 + ui_offset + border_thickness_offset 	, &border_thickness, sizeof(f32));

		const i32 unit_indices[6] = { (i32) ui_position*4+0, (i32) ui_position*4+1, (i32) ui_position*4+2, (i32) ui_position*4+0, (i32) ui_position*4+2, (i32) ui_position*4+3 };
		memcpy(ibuf + ui_position * 6, unit_indices, sizeof(unit_indices));

		if (unit->text) {
			ui_internal_text_to_renderdata(drawbuf + text_position * stride * 4, ibuf + text_position * 6, stride, text_position * 4, font, screen_size, unit->text, unit->text_len, center, unit->text_color);
			text_position += unit->text_len;
		}
	}
}

void ui_hot(struct ui_state *ui_context)
{
	const float x = ui_context->comm.cursor[0];
	const float y = ui_context->comm.cursor[1];

	if (strcmp(ui_context->active, no_active) != 0) {
		ui_context->hot = no_hot;
	} else {
		struct ui_unit *hot = ui_context->units->data;
		struct ui_unit *next = hot->first;
		while (next != NULL)
		{
			if (next->p_tl[0] < x && x < next->p_br[0] && y < next->p_tl[1] && next->p_br[1] < y) {
				hot = next;
				next = next->first;
			} else {
				next = next->next;
			}
		}

		ui_context->hot = hot->id;
	}
}

void ui_cache(struct ui_state *ui_context)
{
	//TODO(Axel): Cache prev useful data
	ui_context->units->max_used = UINT64_MAX;
	ui_context->unit_depth->max_used = UINT64_MAX;
	hash_clear(ui_context->hash);
	ui_context->depth = -1;
	memset(&ui_context->depth_batch, 0, sizeof(ui_context->depth_batch));
	memset(&ui_context->depth_text_batch, 0, sizeof(ui_context->depth_text_batch));
	vec2_copy(ui_context->comm.delta, ui_context->comm.cursor);
}

void ui_internal_adherence(struct ui_unit *parent)
{
	f32 x_ad = 1.0f;
	f32 y_ad = 1.0f;
	const f32 right_boundary = parent->size.position[0] + parent->size.size[0];
	const f32 bottom_boundary = parent->size.position[1] - parent->size.size[1];

	switch (parent->size.type[0]) {
		case UNIT_SIZE_CHILDSUM:
		{
			const f32 x0 = parent->first->size.position[0];
			const f32 x1 = parent->last->size.position[0] + parent->last->size.size[0];
			if (right_boundary < x1) {
				x_ad = (right_boundary - x0) / (x1 - x0);
				x_ad = (x_ad < parent->first->size.strictness[0]) ? parent->first->size.strictness[0] : x_ad;
				u32 i = 0;
				for (struct ui_unit *child = parent->first; child != NULL; ++i, child = child->next) {
					child->size.size[0] *= x_ad;
					child->size.position[0] = parent->size.position[0] + i * child->size.size[0];
				}
			}
		} break;

		default:
		{
			for (struct ui_unit *child = parent->first; child != NULL; child = child->next) {
				const f32 x0 = child->size.position[0];
				const f32 x1 = child->size.position[0] + child->size.size[0];
				if (right_boundary < x1) {
					x_ad = (right_boundary - x0) / (x1 - x0);
					x_ad = (x_ad < child->size.strictness[0]) ? child->size.strictness[0] : x_ad;
					child->size.size[0] *= x_ad;
				}
			}
		} break;
	}

	switch (parent->size.type[1]) {
		case UNIT_SIZE_CHILDSUM:
		{
			const f32 y0 = parent->first->size.position[1];
			const f32 y1 = parent->last->size.position[1] - parent->last->size.size[1];
			if (bottom_boundary > y1) {
				y_ad = (y0 - bottom_boundary) / (y0 - y1);
				y_ad = (y_ad < parent->first->size.strictness[1]) ? parent->first->size.strictness[1] : y_ad;
				u32 i = 0;
				for (struct ui_unit *child = parent->first; child != NULL; ++i, child = child->next) {
					child->size.size[1] *= y_ad;
					child->size.position[1] = parent->size.position[1] - i * child->size.size[1];
				}
			}
		} break;

		default:
		{
			for (struct ui_unit *child = parent->first; child != NULL; child = child->next) {
				const f32 y0 = child->size.position[1];
				const f32 y1 = child->size.position[1] - child->size.size[1];
				if (bottom_boundary > y1) {
					y_ad = (y0 - bottom_boundary) / (y0 - y1);
					y_ad = (y_ad < child->size.strictness[1]) ? child->size.strictness[1] : y_ad;
					child->size.size[1] *= y_ad;
				}
			}
		} break;
	}
}

/**
 * Top-Bottom constraint-solving.
 */
void ui_internal_solve_violations(struct ui_state *ui_context)
{
	struct ui_unit *unit = d_array_get(ui_context->units, 0);	
	while (unit != NULL) {
		vec2_copy(unit->p_tl, unit->size.position);
		vec2_set(unit->p_br, unit->size.position[0] + unit->size.size[0], unit->size.position[1] - unit->size.size[1]);
		if (unit->first != NULL) {
			ui_internal_adherence(unit);
			unit = unit->first;
		} else if (unit->next != NULL) {
			unit = unit->next;
		} else {
			unit = unit->parent;
			while (unit != NULL) {
				if (unit->next != NULL) {
					unit = unit->next;
					break;
				}
				unit = unit->parent;
			}
		} 
	}
}

void ui_internal_unit_autosize(struct ui_state *ui_context, struct ui_unit *unit)
{
	if (unit->size.type[0] == UNIT_SIZE_CHILDSUM) {
		unit->size.size[0] = 0.0f;
		for (struct ui_unit *child = unit->first; child != NULL; child = child->next) {
			vec2_set(child->size.position, unit->size.position[0] + unit->size.size[0], unit->size.position[1]);
			unit->size.size[0] += child->size.size[0];
		}
	} else if (unit->size.type[1] == UNIT_SIZE_CHILDSUM) {
		unit->size.size[1] = 0.0f;
		for (struct ui_unit *child = unit->first; child != NULL; child = child->next) {
			vec2_set(child->size.position, unit->size.position[0], unit->size.position[1] - unit->size.size[1]);
			unit->size.size[1] += child->size.size[1];
		}
	}
}

/**
 * Bottom-up approach: we find sizes bottom-up, with violation solving happening after.
 * During ui_building code, constant sizes and parent dependent sizes should be set, 
 * only child dependencies here. 
 */
void ui_autolayout(struct ui_state *ui_context)
{
	struct ui_unit *unit = d_array_get(ui_context->units, 0);
	while (unit != NULL) {
		if (unit->first) {
			unit = unit->first;
		} else {
			if (unit->next != NULL) {
				unit = unit->next; /* All siblings has not yet to be calculated */
			} else {
				for (unit = unit->parent; unit != NULL; unit = unit->parent) {
					ui_internal_unit_autosize(ui_context, unit);
					if (unit->next != NULL) {
						unit = unit->next;
						break;
					}
				}
			}
		}
	}
	
	ui_internal_solve_violations(ui_context);
}


struct ui_unit *ui_void(struct ui_state *context, const char *id)
{
	struct ui_unit *v = ui_unit_create(context, id, 0); 

	vec4_set(v->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
	vec4_set(v->border_color, 0.0f, 0.0f, 0.0f, 0.0f);
	v->size.type[0] = UNIT_SIZE_PIXEL;
	v->size.type[1] = UNIT_SIZE_PIXEL;

	return v;
}

struct ui_unit *ui_text_window(struct ui_state *context, const char *id, const f32 window_height, const f32 side_padding, const i32 screen_width, const float aspect_ratio, const struct mg_font *font, u32 *text, u32 text_len)
{
	const struct ui_unit *parent = context->unit_stack[context->stack_index];
	struct ui_unit *window = ui_unit_create(context, id, UNIT_DRAW_BORDER | UNIT_DRAW_TEXT);

	const f32 text_width = mg_font_text_width(font, text, text_len, aspect_ratio, screen_width);
	const f32 window_width = text_width + 2.0f * side_padding;

	window->text = text;
	window->text_len = text_len;
	window->size.type[0] = UNIT_SIZE_TEXT;
	window->size.type[1] = UNIT_SIZE_PIXEL;
	vec2_copy(window->size.position, parent->size.position);
	vec2_set(window->size.size, window_width, window_height);
	vec2_set(window->size.strictness, 0.0f, 1.0f);

	return window;
}

struct ui_unit *ui_dummy_window(struct ui_state *context, const char *id)
{
	struct ui_unit *dw = ui_unit_create(context, id, UNIT_DRAW_BORDER | UNIT_DRAW_BACKGROUND); 
	dw->size.type[0] = UNIT_SIZE_PIXEL;
	dw->size.type[1] = UNIT_SIZE_PIXEL;

	//TODO(Axel): Remove into seperate load_visuals func which uses flags to call correct loads
	const i32 is_hot = (strcmp(id, context->hot) == 0) ? 1 : 0;
	const i32 is_active = (strcmp(id, context->active) == 0) ? 1 : 0;
	if (is_active) {
		if (context->comm.released) {
			context->active = no_active;
		} else {
			vec4_set(dw->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	} else if (is_hot) {
		if (context->comm.pressed) {
			context->active = id;
		} else {
			vec4_set(dw->border_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	return dw;
}


struct ui_unit *ui_dummy_bar(struct ui_state *context, const char *id, const vec2 bar_position, const f32 height)
{
	struct ui_unit *bar = ui_unit_create(context, id, UNIT_DRAW_BORDER | UNIT_DRAW_BACKGROUND);

	bar->size.type[0] = UNIT_SIZE_CHILDSUM;
	bar->size.type[1] = UNIT_SIZE_PIXEL;
	vec2_copy(bar->size.position, bar_position);
	bar->size.size[1] = height;
	vec2_set(bar->size.strictness, 0.0f, 0.0f);
	
	return bar;
}

struct ui_unit *ui_dummy_list(struct ui_state *context, const char *id, const vec2 list_position, const f32 width)
{
	assert(width > 0.0f);

	struct ui_unit *list = ui_unit_create(context, id, UNIT_DRAW_BORDER | UNIT_DRAW_BACKGROUND);

	list->size.type[0] = UNIT_SIZE_PIXEL;
	list->size.type[1] = UNIT_SIZE_CHILDSUM;
	vec2_copy(list->size.position, list_position);
	list->size.size[0] = width;
	vec2_set(list->size.strictness, 0.0f, 0.0f);
	
	return list;
}

struct ui_unit *ui_dummy_button(struct ui_state *context, const char *id, const vec2 button_size)
{
	assert(button_size[0] > 0.0f && button_size[1] > 0.0f);

	struct ui_unit *button = ui_unit_create(context, id, UNIT_DRAW_BACKGROUND);
	
	const i32 is_hot = (strcmp(id, context->hot) == 0) ? 1 : 0;
	const i32 is_active = (strcmp(id, context->active) == 0) ? 1 : 0;

	if (is_active) {
		if (context->comm.released) {
			context->active = no_active;
		} else {
			vec4_set(button->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	} else if (is_hot) {
		if (context->comm.pressed) {
			context->active = id;
		} else {
			vec4_set(button->border_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	button->size.type[0] = UNIT_SIZE_PIXEL;
	button->size.type[1] = UNIT_SIZE_PIXEL;
	vec2_set(button->size.position, 0.0f, 0.0f);
	vec2_copy(button->size.size, button_size);
	vec2_set(button->size.strictness, 0.1f, 0.1f);

	return button;
}

struct ui_unit *ui_hollow_bar(struct ui_state *context, const char *id, const vec2 size)
{
	assert(size[0] > 0.0f && size[1] > 0.0f);

	struct ui_unit *bar = ui_unit_create(context, id, UNIT_DRAW_BACKGROUND | UNIT_DRAW_BORDER);

	bar->size.type[0] = UNIT_SIZE_PIXEL;
	bar->size.type[1] = UNIT_SIZE_PIXEL;
	vec2_copy(bar->size.size, size);
	vec2_set(bar->size.strictness, 1.0f, 1.0f);

	return bar;
}
