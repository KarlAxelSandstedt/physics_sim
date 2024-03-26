#ifndef __MGX11_PUBLIC_H__
#define __MGX11_PUBLIC_H__

#include "mg_common.h"
#if __GAPI__ == __X11__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "system_common.h"

struct window
{
	char *title;
	vec2u32 position;
	vec2u32 size;	
	vec2u32 border_position;	/* store old windowed / fullscreen info */
	vec2u32 border_size;	
	vec2u32 screen_size;
	u32 border_width;
	u32 header_height;

	vec2i32 counter_motion;
	i8 counter_motion_waiting;
	i8 cursor_locked_to_window;	/* reset cursor to middle of window */
	i8 cursor_in_window;
	i8 skip_next_motion;
	i8 focused;
	i8 visible;
	i8 border;
	
	Display *display;
	Window window;
	GC graphics_context;
	i32 screen_num;
};

struct window X11_init_window_resources(char *title, const vec2u32 position, const vec2u32 size, const u32 border_width);
void X11_release_window_resources(struct window *win);

void X11_mgl_init(void);
void X11_mgl_shutdown(struct gl_config *gl);
struct gl_config X11_get_gl_config(void);

#endif
#endif
