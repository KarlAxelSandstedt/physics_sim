#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mg_common.h"
#include "system_common.h"
#include "x11_public.h"
#include "x11_local.h"
#include "mgl.h"

/* bit definitions for MwmHints.flags */
#define MWM_HINTS_FUNCTIONS (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_HINTS_INPUT_MODE    (1L << 2)
#define MWM_HINTS_STATUS    (1L << 3)

/* bit definitions for MwmHints.functions */
#define MWM_FUNC_ALL        (1L << 0)
#define MWM_FUNC_RESIZE     (1L << 1)
#define MWM_FUNC_MOVE       (1L << 2)
#define MWM_FUNC_MINIMIZE   (1L << 3)
#define MWM_FUNC_MAXIMIZE   (1L << 4)
#define MWM_FUNC_CLOSE      (1L << 5)

/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL       (1L << 0)
#define MWM_DECOR_BORDER    (1L << 1)
#define MWM_DECOR_RESIZEH   (1L << 2)
#define MWM_DECOR_TITLE     (1L << 3)
#define MWM_DECOR_MENU      (1L << 4)
#define MWM_DECOR_MINIMIZE  (1L << 5)
#define MWM_DECOR_MAXIMIZE  (1L << 6)

void (*cursor_show)(struct window *win);
void (*cursor_hide)(struct window *win);
void (*cursor_grab)(struct window *win);
void (*cursor_ungrab)(struct window *win);
void (*cursor_warp_to_center)(struct window *win);
				
void (*window_config_update)(struct window *win);
void (*window_maximize)(struct window *win);
void (*window_bordered)(struct window *win, const i8 border);
void (*window_restore_border_size)(struct window *win);

struct motif_hint
{
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long inputMode;
	unsigned long status;
};

struct net_extent_hint
{
	u32 left;
	u32 right;
	u32 top;
	u32 bottom;
};

static void X11_get_window_header_height(struct window *win)
{
	Atom extent_hint = XInternAtom(win->display, "_NET_FRAME_EXTENTS", True);

	if (extent_hint == None)
	{
		assert(0 && "fix _NET_FRAME_EXTENTS");
	}

	Atom type;
	i32 format;
	unsigned long items;
	unsigned long bytes_left;
	unsigned long *extent; /* left | right | top | bottom */

	if (XGetWindowProperty(win->display, win->window, extent_hint, 0, 4, False, AnyPropertyType, 
			&type,
			&format,
			&items,
			&bytes_left,
			(u8 **) &extent) != Success)
	{
		assert(0 && "Failed to retrieve header height\n");
	}

	assert(bytes_left == 0 && "Failed to get XGetWindowProperty in one go, fix");
	assert(items == 4 && "Wrong number of items returned in _NET_FRAME_EXTENTS, fix");

	win->header_height = extent[2] - extent[3];

	XFree(extent);
}

void X11_window_restore_border_size(struct window *win)
{
	if (win->border)
	{
		win->position[0] = win->border_position[0];
		win->position[1] = win->border_position[1];
		win->size[0] = win->border_size[0];
		win->size[1] = win->border_size[1];
		
		XWindowChanges changes =
		{
			.x = win->position[0],
			.y = win->position[1],
			.width =  win->size[0],
			.height = win->size[1],
		};

		XConfigureWindow(win->display, win->window, CWX | CWY | CWWidth | CWHeight, &changes);
	}
}

static void X11_window_save_border_size(struct window *win)
{
	win->border_position[0] = win->position[0];
	win->border_position[1] = win->position[1];
	win->border_size[0] = win->size[0];
	win->border_size[1] = win->size[1];
}

void X11_window_maximize(struct window *win)
{
	XWindowChanges changes =
	{
		.x = 0,
		.y = 0,
		.width = win->screen_size[0],
		.height = win->screen_size[1],
	};

	win->position[0] = 0;
	win->position[1] = 0;
	win->size[0] = win->screen_size[0];
	win->size[1] = win->screen_size[1];

	XConfigureWindow(win->display, win->window, CWX | CWY | CWWidth | CWHeight, &changes);
}

void X11_window_bordered(struct window *win, const i8 border)
{
	if (win->border)
	{
		X11_window_save_border_size(win);
	}

 	/* Get "Atom" or "property_id" of MOTIF_WM_HINTS, recognized by many WMs. */
	Atom property_id = XInternAtom(win->display, "_MOTIF_WM_HINTS", True);
	if (property_id == None)
	{
		fprintf(stderr, "Error %s:%i - Failed to retrieve X11 fullscreen atom\n", __FILE__, __LINE__);
		return;
	}

	struct motif_hint hint = 
	{ 
		.flags = MWM_HINTS_DECORATIONS,
		.functions = 0,
		.decorations = border,	/* turn off decorations */
		.inputMode = 0,
		.status = 0,
	};

	XChangeProperty(win->display, win->window, property_id, property_id, sizeof(32)*8, PropModeReplace, (u8*) &hint, sizeof(struct motif_hint) / sizeof(long));

	win->border = border;
}

void X11_window_config_update(struct window *win)
{
	XWindowAttributes attributes;
       	XGetWindowAttributes(win->display, win->window, &attributes);

	Window garbage;
	vec2i32 root_position;
	if (XTranslateCoordinates(win->display, win->window, attributes.root, 0, 0, &root_position[0], &root_position[1], &garbage) == False)
	{
		assert(0 && "Big bad, src window and root window on different screens?");
	}

       	win->position[0] = root_position[0] - attributes.x;
       	win->position[1] = root_position[1] - attributes.y - win->header_height;
       	win->size[0] = attributes.width;
       	win->size[1] = attributes.height;
}

void X11_window_coordinates_to_system_coordinates(const struct window *win, vec2i32 sys_coordinates, const vec2i32 X11_coordinates)
{
	sys_coordinates[0] = X11_coordinates[0]; 
	sys_coordinates[1] = win->size[1] - 1 - X11_coordinates[1];
}

void system_coordinates_to_X11_window_coordinates(const struct window *win, vec2i32 X11_coordinates, const vec2i32 sys_coordinates)
{
	X11_coordinates[0] = sys_coordinates[0];
	X11_coordinates[1] = win->size[1] - 1 - sys_coordinates[1];
}

struct window X11_init_window_resources(char *title, const vec2u32 position, const vec2u32 size, const u32 border_width)
{
	struct window win =
	{
		.title = title,
		.position = { position[0], position[1] },
		.size = { size[0], size[1] },
		.border_width = border_width,
	};

	X11_window_new(&win);
	X11_init_window_pointers();
	X11_init_event_pointers();
	X11_get_window_header_height(&win);

	return win;
}

void X11_release_window_resources(struct window *win)
{
	X11_window_close(win);
}

#define MAX_MESSAGE_SIZE 256

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK \
		| FocusChangeMask | EnterWindowMask \
		| LeaveWindowMask | ExposureMask | StructureNotifyMask)

i32 X11_error_handler(Display *display, XErrorEvent *event)
{
	char buf[MAX_MESSAGE_SIZE];

	XGetErrorText(display, event->error_code, buf, MAX_MESSAGE_SIZE);
	fprintf(stderr, "System FATAL: %s\n", buf);

	return 0;
}

void X11_cursor_hide(struct window *win)
{
	Pixmap pixmap = XCreatePixmap(win->display, win->window, 1, 1, 1);
	XColor color = 
	{
		.pixel = 0,
		.blue = 0,
		.flags = 0x1,
	};

	Cursor hidden_cursor = XCreatePixmapCursor(win->display, pixmap, pixmap, &color, &color, 0, 0);

	XDefineCursor(win->display, win->window, hidden_cursor);
	
	XFreePixmap(win->display, pixmap);
}

void X11_cursor_show(struct window *win)
{

}

void X11_cursor_grab(struct window *win)
{
	XWarpPointer(win->display, None, win->window, 0, 0, 0, 0, win->screen_size[0]/2, win->screen_size[1]/2);
	XGrabPointer(win->display, win->window, false, MOUSE_MASK, GrabModeAsync, GrabModeAsync, 
		win->window, None, CurrentTime);

}

void X11_cursor_ungrab(struct window *win)
{
	XWarpPointer(win->display, None, win->window, 0, 0, win->size[0], win->size[1], win->size[0]/2, win->size[1]/2);
}

void X11_cursor_warp_to_center(struct window *win)
{
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

	vec2i32 system_coordinates;
	X11_window_coordinates_to_system_coordinates(win, system_coordinates, X11_coordinates);
	const vec2i32 system_center = { win->size[0]/2, win->size[1]/2 };
	vec2i32 X11_center;
	system_coordinates_to_X11_window_coordinates(win, X11_center, system_center);

	struct system_event counter_motion = 
	{
		.type = SYSTEM_COUNTER_MOTION,
		.value =
		{
			.cursor_position = 
			{ 
				system_coordinates[0] - system_center[0], 
				system_coordinates[1] - system_center[1], 
			},
		}
	};

	SYS_EVENT_ADD(&counter_motion);
	XWarpPointer(win->display, None, win->window, 0, 0, 0, 0, X11_center[0], X11_center[1]);
}

void X11_window_new(struct window *win)
{
	XSetErrorHandler(&X11_error_handler);

	win->display = XOpenDisplay(NULL);

	win->screen_num = DefaultScreen(win->display);
	Window root_window = RootWindow(win->display, win->screen_num);
	const u32 valuemask = CWEventMask | CWColormap;

	/* Setup opengl graphics context */
	i32 pixelbuf_attributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo *visual_info = mglXChooseVisual(win->display, win->screen_num, pixelbuf_attributes);
	if (visual_info == NULL)
	{
		fprintf(stderr, "System FATAL: X11 failed to setup opengl XVisualInfo\n");
	}

	Colormap colormap = XCreateColormap(win->display, root_window, visual_info->visual, AllocNone); 
	
	XSetWindowAttributes set_attributes =
	{
		.colormap = colormap,
		.event_mask = X_MASK
	};

	win->window = XCreateWindow(win->display,
				root_window,
				win->position[0],
				win->position[1],
				win->size[0],
				win->size[1],
				win->border_width,
				visual_info->depth,
				InputOutput,
				visual_info->visual,
				valuemask,
				&set_attributes);

	Screen *screen = ScreenOfDisplay(win->display, win->screen_num);
	win->screen_size[0] = WidthOfScreen(screen);
	win->screen_size[1] = HeightOfScreen(screen);
	win->border = 1;

	XMapWindow(win->display, win->window);
	
	GLXContext gl_ctx = mglXCreateContext(win->display, visual_info, NULL, GL_TRUE);
	mglXMakeCurrent(win->display, win->window, gl_ctx);

	XTextProperty x_title;
	if (XStringListToTextProperty(&win->title, 1, &x_title) != 0)
	{
		XSetWMName(win->display, win->window, &x_title);
	}
	else
	{
		fprintf(stderr, "System WARNING: X11 failed to set window title\n");
	}
}

void X11_window_close(struct window *win)
{
	XUnmapWindow(win->display, win->window);
	XDestroyWindow(win->display, win->window);
	XCloseDisplay(win->display);
}

void X11_init_window_pointers(void)
{
	cursor_show = &X11_cursor_show;
	cursor_hide = &X11_cursor_hide;
	cursor_grab = &X11_cursor_grab;
	cursor_ungrab = &X11_cursor_grab;
	cursor_warp_to_center = &X11_cursor_warp_to_center;
	window_config_update = &X11_window_config_update;
	window_bordered = &X11_window_bordered;
	window_maximize = &X11_window_maximize;
	window_restore_border_size = &X11_window_restore_border_size;
}
