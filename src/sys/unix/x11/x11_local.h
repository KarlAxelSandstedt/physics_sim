#ifndef __MGX11_LOCAL_H__
#define __MGX11_LOCAL_H__

#include "mg_common.h"
#if __GAPI__ == __X11__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "system_common.h"

#define SYS_EVENT_ADD(event_addr) (X11_system_event_queue(event_addr))

i32 X11_error_handler(Display *display, XErrorEvent *event);
void X11_window_new(struct window *win);
void X11_window_close(struct window *win);
void X11_init_window_pointers(void);

void X11_cursor_hide(struct window *win);
void X11_cursor_show(struct window *win);
void X11_cursor_grab(struct window *win);
void X11_cursor_ungrab(struct window *win);

void X11_window_new(struct window *win);
void X11_window_close(struct window *win);
void X11_window_coordinates_to_system_coordinates(const struct window *win, vec2i32 sys_coordinates, const vec2i32 X11_coordinates);

enum mg_keycode X11_mg_keycode_lookup(const KeySym symbol);
enum mg_button X11_mg_button_lookup(const i32 state);

void X11_init_event_pointers(void);

i32 X11_system_event_queue(const struct system_event *event);

#endif
#endif

