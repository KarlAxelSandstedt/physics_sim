#ifndef __SYS_INFO_H__
#define __SYS_INFO_H__

/************************************************************************/
/* 			System level define controls 			*/
/************************************************************************/

#include "stdio.h"
#include "mg_common.h"
#include "mg_mempool.h"
#include "mg_buffer.h"
#include "mmath.h"
#include "bit_vector.h"
#include "system_common.h"

#if __GAPI__ == __X11__
#include "x11_public.h"
#elif __GAPI__ == __WAYLAND__
#elif __GAPI__ == __WIN64__
#include "win_public.h"
#endif

/************************************************************************/
/* 			graphics abstraction layer 			*/
/************************************************************************/

/*
 * window coordinate system:
 *
 *  (0,Y) ------------------------- (X,Y)
 *    |				      |
 *    |				      |
 *    |				      |
 *    |				      |
 *    |				      |
 *  (0,0) ------------------------- (X,0)
 *
 *  Since we are using a right handed coordinate system to describe the world, and the camera
 *  looks down the +Z axis, an increase in X or Y in the screen space means an "increase" from
 *  the camera's perspective as well. We must ensure that the x11, win64 or wayland window
 *  events giving screen coordinates are translated into this format.
 *
 *		A (Y)
 *		|
 *		|	(X)
 *		|------->
 *	       /
 *            / 
 *	     V (Z)
 */

struct graphics_context
{
	struct window win;
	struct gl_config gl;
};

struct graphics_context *graphics_context_init(struct arena *mem, char *title, const vec2u32 position, const vec2u32 size, const u32 border_size);
void graphics_context_destroy(struct graphics_context *gc);
void gl_config_log(struct gl_config *gl, FILE *file);
void gc_update(struct graphics_context *gc);

/* X11, Wayland and Win64 API abstraction */
extern void (*cursor_warp_to_center)(struct window *win);	/* Generates a counter motion event, if needed, to mitigate any unwanted motion event generated by the platform */
extern void (*cursor_show)(struct window *win);
extern void (*cursor_hide)(struct window *win);
extern void (*cursor_grab)(struct window *win);
extern void (*cursor_ungrab)(struct window *win);

extern void (*window_config_update)(struct window *win);
extern void (*window_maximize)(struct window *win);
extern void (*window_bordered)(struct window *win, const i8 border);
extern void (*window_restore_border_size)(struct window *win);

/************************************************************************/
/* 				System Inititation			*/
/************************************************************************/

/* Initiate/cleanup system resources such as timers, input handling, system events, ... */
void system_resources_init(struct arena *mem);
void system_resources_cleanup(void);

/************************************************************************/
/* 				System Events 				*/
/************************************************************************/

/**
 * System Event Queue - Platform independent queue for handling of System level events such as keyboard
 * 	and mouse input. 
 */
void 	system_event_queue_new(struct arena *mem);
i32  	system_event_queue(const struct system_event *event); /* memcpy event into queue if queue isn't filled, return -1 on failure, 0 on success */
void 	system_event_clear(void); /* clear queue of events */

/* push system events (if any) onto the global system queue g_sys_queue, set to appropriate platform dependent function */
extern void (*system_push_events)(struct window *win);

/************************************************************************/
/* 			system mouse/keyboard handling 			*/
/************************************************************************/

/**
 * Program specific keycodes. node that given a previous entry's value, if the next one has not a specific value, 
 * it will be set to the previous value + 1. We should not override any keycodes for lowercase letters, nor numerics.
 * Keep this under 255 and we can keep everythin in a char!
 */
void input_state_init(struct arena *mem);

#if	__GAPI__ == __X11__
#define SYMBOL	KeySym
#elif	__GAPI__ == __WIN64__
#define SYMBOL	i32
#endif

extern enum mg_keycode (*keycode_lookup)(const SYMBOL s);
const char *mg_keycode_to_string(const i32 key);
const char *mg_button_to_string(const i32 button);

/************************************************************************/
/* 				System IO 				*/
/************************************************************************/

struct mg_buffer file_raw_dump(struct arena *mem, const char *pathname);
struct mg_buffer file_init_buf(struct arena *mem, const char *pathname);
u64 file_read(struct mg_buffer *buf, const char *pathname);

#endif
