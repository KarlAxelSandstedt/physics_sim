#include <stdlib.h>
#include <string.h>

#include "system_public.h"
#include "system_local.h"

struct graphics_context *graphics_context_init(struct arena *mem, char *title, const vec2u32 position, const vec2u32 size, const u32 border_size)
{
	static i32 initiated = 0;
	if (initiated++)
	{
		//sys_error(__FILE__, __LINE__, "ERROR: trying to initialize graphics context twice");
		//sys_exit();
	}

	struct graphics_context cpy = { 0 };
	struct graphics_context *gc = NULL;

#if __GAPI__ == __X11__	
	X11_mgl_init();
	cpy.win = X11_init_window_resources(title, position, size, border_size);
	cpy.gl = X11_get_gl_config();
#elif __GAPI__ == __WAYLAND__
#elif __GAPI__ == __WIN64__
	cpy.win = win_init_window_resources(title, position, size, border_size);
	win_mgl_init();
	cpy.gl = win_get_gl_config();
#endif
	if (mem)
	{
		gc = arena_push(mem, &cpy, sizeof(struct graphics_context));
	}
	else
	{
		gc = malloc(sizeof(struct graphics_context));
		memcpy(gc, &cpy, sizeof(struct graphics_context));
	}

	return gc;
}

void graphics_context_destroy(struct graphics_context *gc)
{
#if __GAPI__ == __X11__	
	X11_release_window_resources(&gc->win);
	X11_mgl_shutdown(&gc->gl);
#elif __GAPI__ == __WAYLAND__
#elif __GAPI__ == __WIN64__
	win_release_window_resources(&gc->win);
	win_mgl_shutdown(&gc->gl);
#endif
}

void gl_config_log(struct gl_config *gl, FILE *file)
{
	fprintf(file, "GL Vendor           - %s\n", gl->gl_vendor);
	fprintf(file, "GL Renderer         - %s\n", gl->gl_renderer);
	fprintf(file, "GL Version          - %s\n", gl->gl_version);
	fprintf(file, "GL Shading Language - %s\n", gl->gl_shading_language);
}

void gc_update(struct graphics_context *gc)
{
	if (gc->win.cursor_locked_to_window)
	{
		cursor_warp_to_center(&gc->win);		
	}
}
