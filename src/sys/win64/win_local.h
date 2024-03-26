#ifndef __WIN_LOCAL_H__
#define __WIN_LOCAL_H__

#include <windows.h>
#include "mg_common.h"
#include "system_common.h"
#include "mg_buffer.h"
#include "win_public.h"


/************************* win_init.c *************************/

/************************* win_window.c *************************/

void win_cursor_show(struct window *win);
void win_cursor_hide(struct window *win);
void win_cursor_grab(struct window *win);
void win_cursor_ungrab(struct window *win);
void win_cursor_warp_to_center(struct window *win);

void win_window_config_update(struct window *win);
void win_window_maximize(struct window *win);
void win_window_bordered(struct window *win, const i8 border);
void win_window_restore_border_size(struct window *win);

/************************* win_gl.c *************************/

/************************* win_input.c *************************/

enum mg_keycode win_mg_keycode_lookup(const u32 symbol);

/************************* win_IO.c *************************/

/************************* win_event.c *************************/

void win_system_push_events(struct window *win);
LRESULT CALLBACK windows_event_callback(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

#endif
