#include "win_public.h"
#include "win_local.h"

void (*system_push_events)(struct window *win);

void (*cursor_show)(struct window *win);
void (*cursor_hide)(struct window *win);
void (*cursor_grab)(struct window *win);
void (*cursor_ungrab)(struct window *win);
void (*cursor_warp_to_center)(struct window *win);

void (*window_config_update)(struct window *win);
void (*window_maximize)(struct window *win);
void (*window_bordered)(struct window *win, const i8 border);
void (*window_restore_border_size)(struct window *win);

void win_init_function_pointers(void)
{
	system_push_events = &win_system_push_events;
	cursor_show = &win_cursor_show;
	cursor_hide = &win_cursor_hide;
	cursor_grab = &win_cursor_grab;
	cursor_ungrab = &win_cursor_grab;
	cursor_warp_to_center = &win_cursor_warp_to_center;
	window_config_update = &win_window_config_update;
	window_bordered = &win_window_bordered;
	window_maximize = &win_window_maximize;
	window_restore_border_size = &win_window_restore_border_size;

}
