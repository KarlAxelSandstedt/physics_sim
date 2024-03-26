#ifndef __WIN_PUBLIC_H__
#define __WIN_PUBLIC_H__

#include <windows.h>
#include "mg_common.h"
#include "system_common.h"
#include "mg_buffer.h"

/************************* win_init.c *************************/

void win_init_function_pointers(void);

/************************* win_window.c *************************/

struct window
{
	char *title;
	vec2u32 position;
	vec2u32 size;	
	vec2u32 screen_size;
	u32 border_width;

	vec2i32 counter_motion;
	i8 counter_motion_waiting;
	i8 cursor_locked_to_window;	/* reset cursor to middle of window */
	i8 cursor_in_window;
	i8 skip_next_motion;
	i8 focused;
	i8 visible;
	i8 border;
	
	WNDCLASSA win_class;
	HWND win_handle;
	HDC device_context;
	HGLRC gl_context;
};

struct window win_init_window_resources(char *title, const vec2u32 position, const vec2u32 size, const u32 border_size);
void win_release_window_resources(struct window *win);

/************************* win_gl.c *************************/

void win_mgl_init(void);
void win_mgl_shutdown(struct gl_config *gl);
struct gl_config win_get_gl_config(void);

/************************* win_input.c *************************/

/************************* win_IO.c *************************/

struct mg_buffer win_file_raw_dump(struct arena *mem, const char *pathname);
u64 win_file_read(struct mg_buffer *buf, const char *pathname);
struct mg_buffer win_file_init_buf(struct arena *mem, const char *pathname);

/************************* win_event.c *************************/

#endif
