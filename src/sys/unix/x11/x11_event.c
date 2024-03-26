#include <string.h>

#include "mg_common.h"
#include "x11_public.h"
#include "x11_local.h"
#include "system_common.h"

void (*system_push_events)(struct window *win);

i32 X11_system_event_queue(const struct system_event *event)
{
	if (g_sys_queue->num_events < MAX_SYSTEM_EVENTS)
	{
		memcpy(&g_sys_queue->events[g_sys_queue->num_events], event, sizeof(struct system_event));
		g_sys_queue->num_events += 1;
		return 0;
	}
	else
	{
		return -1;	
	}
}

void X11_push_system_events(struct window *win)
{
	XEvent event;
       	XFlush(win->display);
       	const i32 num_events = XEventsQueued(win->display, QueuedAfterFlush);
       	for (i32 i = 0; i < num_events; ++i) {
       		XNextEvent(win->display, &event);
       		switch (event.type)
       		{
       			case ButtonPress:
       			{
				struct system_event input =
				{
					.type = SYSTEM_BUTTON_PRESSED,
					.value = { .button = X11_mg_button_lookup(((XButtonEvent *) &event)->state) },
				};
				SYS_EVENT_ADD(&input);
       			} break; 

       			case ButtonRelease:
       			{
				struct system_event input =
				{
					.type = SYSTEM_BUTTON_RELEASED,
					.value = { .button = X11_mg_button_lookup(((XButtonEvent *) &event)->state) },
				};
				SYS_EVENT_ADD(&input);
       			} break; 

       			case KeyPress:
       			{
       				KeySym symbol = XLookupKeysym((XKeyEvent *) &event, 0);
       				struct system_event input =
       				{
       					.type = SYSTEM_KEY_PRESSED,
       					.value = { .key = X11_mg_keycode_lookup(symbol) },
       				};
       				SYS_EVENT_ADD(&input);
       			} break; 

       			case KeyRelease:
       			{
				KeySym symbol = XLookupKeysym((XKeyEvent *) &event, 0);
       				struct system_event input =
       				{
       					.type = SYSTEM_KEY_RELEASED,
       					.value = { .key = X11_mg_keycode_lookup(symbol) },
       				};
       				SYS_EVENT_ADD(&input);
       			} break;

       			case MotionNotify:
       			{
       				const XMotionEvent *motion_event = (XMotionEvent *) &event;
       				Window root_return, child_return;
				vec2i32 X11_coordinates;
       				i32 root_x, root_y, win_x, win_y;
       				u32 mask;
       				const Bool in_window = XQueryPointer(win->display
       					       ,win->window
       					       ,&root_return
       					       ,&child_return
       					       ,&root_x
       					       ,&root_y
       					       ,(i32 *) &X11_coordinates[0]
       					       ,(i32 *) &X11_coordinates[1]
       					       ,&mask);

       				struct system_event input =
       				{
       					.type = SYSTEM_CURSOR_POSITION,
       				};
				X11_window_coordinates_to_system_coordinates(win, input.value.cursor_position, X11_coordinates);
       				SYS_EVENT_ADD(&input);
       			} break;
       
       			case EnterNotify:
  			{
				struct system_event notify =
				{
					.type = SYSTEM_WINDOW_CURSOR_ENTER,
				};
				SYS_EVENT_ADD(&notify);
       			} break;

       			case LeaveNotify:
       			{
				struct system_event notify = 
				{
					.type = SYSTEM_WINDOW_CURSOR_LEAVE,
				};
				SYS_EVENT_ADD(&notify);
			} break;

       			case FocusIn:
       			{
				struct system_event notify =
				{
					.type = SYSTEM_WINDOW_FOCUS_IN,
				};
				SYS_EVENT_ADD(&notify);
       			} break;

       			case FocusOut:
       			{
				struct system_event notify =
				{
					.type = SYSTEM_WINDOW_FOCUS_OUT,
				};
				SYS_EVENT_ADD(&notify);
       			} break;

       			case Expose:
       			{
				struct system_event notify =
				{
					.type = SYSTEM_WINDOW_EXPOSE,
				};
				SYS_EVENT_ADD(&notify);
       			} break;

       			case ConfigureNotify:
       			{
				struct system_event config =
				{
					.type = SYSTEM_WINDOW_CONFIG,
				};
				SYS_EVENT_ADD(&config);
       			} break;

			case UnmapNotify: 
			{
				struct system_event config =
				{
					.type = SYSTEM_WINDOW_MINIMIZE,
				};
				SYS_EVENT_ADD(&config);
			} break;

			case MapNotify: { printf("MapNotify\n");} break;
			case MappingNotify: { printf("MappingNotify\n"); } break;
			case ReparentNotify: { printf("ReparentNotify\n"); } break;
			case VisibilityNotify: { printf("VisibilityNotify\n"); } break;

       			default:
       			{
       				printf("Non-supported event!\n");	
       			} break;
       		}
       	}
}

void X11_init_event_pointers(void)
{
	system_push_events = &X11_push_system_events;
}
