#ifndef __MG_WIDGET_H__
#define __MG_WIDGET_H__

#include <stdlib.h>

#include "mg_mempool.h"
#include "mg_font.h"
#include "mg_common.h"
#include "vector.h"
#include "hash_index.h"
#include "dynamic_array.h"

/**
 * (O) ui_sort
 * (O) ui_get_needed_bufsize
 * (O) ui_buffer_data (<= Count num indices)
 * (O) Setup vao outside
 * (O) Setup buffer outside
 * (O) Get data
 * (O) Update shader for rectangle drawing and Draw
 * (O) Update shader for rectangle border drawing and Draw
 * (O) Create child and Draw
 * (O) Create grandchild and Draw
 * (O) Get screen coordinate
 * (O) Get clip-space aspect ratioed click coord (-1,1) x (-1,1) => (a,b) x (-1,1)
 * (O) Set Hot at end of frame
 * (O) Setup simple interaction for active/nonactive unit
 * (O) Set active/nonactive at building phase and remove hot, if the id is hot
 * (O) if hot, change background of dummy window
 * (O) if active, change border color
 * (O) Cleanup code, winMain => widget
 * (O) Port to linux
 * (O) struct ui_size
 * (O) Implement childsum type
 * (O) Implement bar with buttons of different colors
 * (O) Implement bar size restriction
 * (O) Implement button squish in bar
 * () Implement text button 
 * 	(O) Set button height
 * 	(O) Add padding size to sides
 * 	(O) Determine width of text
 * 	(O) Setup depth text batching (Determine num characters to render for each level, after depth sorting)
 * 	(O) Determine new membuf sizes (ui_drawbuffer_size, ui_indexbuffer_size)
 * 	(O) Generate needed renderdata
 * 	(O) Setup text/ui shader to handle text
 * 	(O) Setup and copy to membufs for text rendering (Should use same bufs and shaders as widgets)
 * 	(O) Implement "text / unit depth" so draw order is Unit0 => Text0 => Unit1 => Text1 => ...
 *
 * 	() After layout and solving violations, positions can be written to membuf
 * 	(O) Center text To middle of button
 * 	() Fix for arbitrary aspect ratio and screen width
 * 	() Gen different fonts for differnet pixel heights
 * () Implement [IF NONTEXT => set textcolor to {0.0f, 0.0f, 0.0f, 0.0f}.]
 * () Implement [IF TEXT    => set br_color, bg_color {0.0f, 0.0f, 0.0f, 0.0f}.]
 * () Implement pixel size (See hollow_bar pixel size is float... not int)
 * () Implement text bar 
 * () Implement text box
 * () Implement text window
 * () Implement Log Window
 * () Implement Scrollable List of Items (See part-4)
 * () Implement Scrollable Log Window 
 * () Implement hollow bar (Fixed size)
 * () Implement hollow bar with text buttons
 * () Implement squishing of text buttons in hollow bar
 *
 * Questions:
 * () Implement (Pixels Vs Floating point)
 * () Shader input => ubo instead of vertex attribs?
 * () Determine how to cache persistent data
 * () Determine input struct => ui_interaction
 */


/**
 * DESCRIPTION: In Immediate Mode GUI's, the user interface is recreated and drawn each frame. The user creates
 * 	a ui using the builder code, and, after the ui has been built, the user can determine which widget is hot.
 * 	The next frame as the building code is being called once again, a hot widget can become active depending
 * 	on the user input, and the widget's functionality. As long as a widget is active, no other widget can become
 * 	hot (soon to be interacted with). The Core code changes ui state while the builder code is the code to
 * 	be interfaced with the user. Note, the ui should contain a root widget which all widget's is related to.
 *
 *	 ______________      _______      ________________      _______      _____________________
 *	|Build / Active| => |Set Hot| => |Sort After Depth| => |Draw UI| => |Cache persistent data|
 *	 --------------      -------      ----------------      -------      ---------------------
 */

/************************************************************************/
/*				UNIT_CORE				*/
/************************************************************************/

#define UI_DEPTH_MAX		64
#define VISUAL_DEPTH_MAX	16

struct ui_visual {
	vec4 background_color[VISUAL_DEPTH_MAX];
	vec4 border_color[VISUAL_DEPTH_MAX];
	vec4 text_color[VISUAL_DEPTH_MAX];
	u8 background_color_index;
	u8 border_color_index;
	u8 text_color_index;
};

enum unit_size_type {
	UNIT_SIZE_PIXEL,	/* User defined size */
	UNIT_SIZE_TEXT,		/* Size depends on input text and font */	
	UNIT_SIZE_CHILDSUM,	/* Size depends on children */
	UNIT_SIZE_PERCPARENT,	/* Size depends on parent */
	UNIT_SIZE_COUNT,
};

struct ui_size {
	enum unit_size_type type[2];
	vec2 position;	/* relative to parent (May change later) */
	vec2 size;	/* wanted size (size may change later in frame) */
	vec2 adherence;	/* % of original size along axes we actually get to use */
	vec2 strictness; /* Lower bound for adherence */
	vec2 p_tl; /* final position */
	vec2 p_br;
};

struct ui_unit_depth {
	u16 index;	/* corresponds to unit at units[index] */
	u8 depth;	/* Corresponds to depth of unit in hierarchy */
};

/**
 * struct ui_unit_interaction - Contains Information about interactions with a ui_unit.
 */
struct ui_interaction {
	struct ui_unit *unit;
	vec2 cursor;
	vec2 delta;
	u32 clicked : 1;
	u32 pressed : 1;
	u32 released : 1;
};

struct ui_state {
	struct hash_index *hash;
	struct d_array *units;
	struct d_array *unit_depth;
	struct ui_unit *unit_stack[UI_DEPTH_MAX];
	u32 stack_index;
	struct ui_visual visual;	/* Visual context */
	i32 depth_batch[UI_DEPTH_MAX]; /* [0] == indices to draw at depth 0, [1] == indices at depth 1, ... */
	i32 depth_text_batch[UI_DEPTH_MAX]; /* [0] == indices to draw at depth 0, [1] == indices at depth 1, ... */
	i32 depth;

	struct ui_interaction comm;
	const char *active;
	const char *hot;
};

enum unit_flag {
	UNIT_DRAW_BORDER 	= (1 << 0),
	UNIT_DRAW_BACKGROUND	= (1 << 1),
	UNIT_DRAW_TEXT		= (1 << 2),
};

struct ui_unit {
	struct ui_unit *parent;	/* If NULL, a root unit */
	struct ui_unit *prev;	/* Previoug sibling */
	struct ui_unit *next;	/* Next sibling */
	struct ui_unit *first;	/* First child */
	struct ui_unit *last;	/* Last child */

	const char *id;
	i32 key;
	u32 flags;

	u32 text_len;
	const u32 *text;	/* utf8 decoded */

	vec2 p_tl;
	vec2 p_br;
	vec4 background_color;
	vec4 border_color;
	vec4 text_color;

	struct ui_size size;
};

struct ui_state *ui_context_init(struct arena *mem, const i32 granularity);	/* Granularity should be power of 2 */
struct ui_state ui_context_create(const i32 granularity);	/* Granularity should be power of 2 */
void 		ui_context_destroy(struct ui_state *context);
struct ui_unit *ui_unit_create(struct ui_state *context, const char *id, u32 flags);
i32 		ui_unit_lookup(const struct ui_state *context, const char *id);

/************************************************************************/
/*			       WIDGET_BUILDER				*/
/************************************************************************/

void 	ui_unit_push(struct ui_state *context, struct ui_unit *parent);
void 	ui_unit_pop(struct ui_state *context);
void 	ui_hot(struct ui_state *ui_context);
void	ui_cache(struct ui_state *ui_context); /* Cache persistent data, clear rest, done last in frame */
void	ui_autolayout(struct ui_state *ui_context); /* Autolayout built widgets after their size types */

/* TODO(Axel): Much optimization / simplification can be done in buffer layouts, indices and such
 *	This should, if needed, only be done after most of the shading code of widgets have stabalized.
 *	As of now, we do not know how the shading will be done, and what is needed.
 */
void	ui_unit_depth_sort(struct arena *mem, struct ui_state *context); /* stack usage of arena */
size_t	ui_drawbuffer_size(const struct ui_state *context);
size_t 	ui_indexbuffer_size(const struct ui_state *context);
size_t  ui_drawbuffer_stride(void);
void 	ui_drawbuffer_data(struct ui_state *context, u8 *drawbuf, i32 *ibuf, const struct mg_font *font, const vec2 screen_size);

void 	ui_visual_set_default(struct ui_state *context, const vec4 background_color, const vec4 border_color, const vec4 text_color);
void	ui_push_background_color(struct ui_state *context, const vec4 color);
void	ui_pop_background_color(struct ui_state *context);
void	ui_push_border_color(struct ui_state *context, const vec4 color);
void	ui_pop_border_color(struct ui_state *context);
void	ui_push_text_color(struct ui_state *context, const vec4 color);
void	ui_pop_text_color(struct ui_state *context);

#define UI_SCOPE(PUSH, POP)	(PUSH); for (i32 i = 0; i < 1; ++i, (POP))

#define BACKGROUND_COLOR(context, color)	UI_SCOPE(ui_push_background_color((context), (color)), ui_pop_background_color((context)))
#define BORDER_COLOR(context, color)		UI_SCOPE(ui_push_border_color((context), (color)), ui_pop_border_color((context)))
#define TEXT_COLOR(context, color)		UI_SCOPE(ui_push_text_color((context), (color)), ui_pop_text_color((context)))

/* Fixed size widget which wont draw anything */
struct ui_unit *ui_void(struct ui_state *context, const char *id); 
struct ui_unit *ui_text_window(struct ui_state *context, const char *id, const f32 window_height, const f32 side_padding, const i32 screen_width, const float aspect_ratio, const struct mg_font *font, u32 *text, u32 text_len);
									   
									   

struct ui_unit *ui_dummy_window(struct ui_state *context, const char *id); //TODO(Axel): Remove
struct ui_unit *ui_dummy_bar(struct ui_state *context, const char *id, const vec2 bar_position, const f32 height); //TODO(Axel): Remove
struct ui_unit *ui_dummy_list(struct ui_state *context, const char *id, const vec2 list_position, const f32 width); //TODO(Axel): Remove
struct ui_unit *ui_dummy_button(struct ui_state *context, const char *id, const vec2 button_size);  //TODO(Axel): Remove
struct ui_unit *ui_hollow_bar(struct ui_state *context, const char *id, const vec2 size);

void ui_internal_text_to_renderdata(u8 *drawbuf, i32 *ibuf, const size_t stride, const i32 min_index, const struct mg_font *font, const vec2 screen_size, const u32 *text, const u32 len, const vec2 center, const vec4 text_color);
void ui_internal_unit_autosize(struct ui_state *ui_context, struct ui_unit *unit);
void ui_internal_solve_violations(struct ui_state *ui_context);
void ui_internal_adherence(struct ui_unit *parent);

#endif
