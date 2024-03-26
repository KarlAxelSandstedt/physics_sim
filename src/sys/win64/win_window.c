#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include "mg_common.h"
#include "win_public.h"
#include "win_local.h"
#include <GL\gl.h>
#include "GL\glcorearb.h"
#include "GL\wglext.h"

void win_cursor_show(struct window *win)
{

}

void win_cursor_hide(struct window *win)
{

}

void win_cursor_grab(struct window *win)
{

}

void win_cursor_ungrab(struct window *win)
{

}

void win_cursor_warp_to_center(struct window *win)
{

}

void win_window_config_update(struct window *win)
{
	fprintf(stderr, "conig update\n");
}

void win_window_maximize(struct window *win)
{

}

void win_window_bordered(struct window *win, const i8 border)
{

}

void win_window_restore_border_size(struct window *win)
{

}

struct window win_init_window_resources(char *title, const vec2u32 position, const vec2u32 size, const u32 border_size)
{
	HINSTANCE hInstance;
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, &hInstance);

	struct window win =
	{
		.title = title,
		.position = { position[0], position[1] },
		.size = { size[0], size[1] },
		.win_class =
		{
			.style = CS_OWNDC,
			.lpfnWndProc = &windows_event_callback,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hInstance,
			.hIcon = 0,
			.hCursor = 0,
			.hbrBackground = 0,
	 		.lpszMenuName = 0,
			.lpszClassName = "TextRenderingClass",
		},
	};
	RegisterClassA(&win.win_class);
	
	win.win_handle = CreateWindowExA(
		0,
		win.win_class.lpszClassName,
		"TextRenderingWindow",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, 
		CW_USEDEFAULT,
		size[0],
		size[1],
		0,
		0,
		hInstance,
		0);

	if (IsWindow(win.win_handle))
	{
		DWORD dwStyle = GetWindowLongPtr(win.win_handle, GWL_STYLE);
		DWORD dwExStyle = GetWindowLongPtr(win.win_handle, GWL_EXSTYLE);
		HMENU menu = GetMenu(win.win_handle);

		win.screen_size[0] = GetSystemMetrics(SM_CXSCREEN);
		win.screen_size[1] = GetSystemMetrics(SM_CYSCREEN);

		RECT rc = { 0, 0, win.screen_size[0], win.screen_size[1] };
	
		AdjustWindowRectEx(&rc, dwStyle, menu ? TRUE : FALSE, dwExStyle);
	
		SetWindowPos(win.win_handle, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);
	}

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;

	win.device_context = GetDC(win.win_handle);
	i32 pixel_format = ChoosePixelFormat(win.device_context, &pfd);
	if (pixel_format == 0)
	{
		fprintf(stderr, "Couldn't find appropriate pixel format for device context\n");
		exit(0);
	}

	if (!SetPixelFormat(win.device_context, pixel_format, &pfd))
	{
		fprintf(stderr, "Failed to set pixel format for device context\n");
		exit(0);
	}

	/* FAKE CONTEXT */
	HGLRC fake_context = wglCreateContext(win.device_context);
	if (fake_context == NULL)
	{
		fprintf(stderr, "Failed to create GL context\n");
		exit(0);
	}

	if (!wglMakeCurrent(win.device_context, fake_context))
	{
		fprintf(stderr, "Failed to set GL context\n");
		exit(0);
	}

	HGLRC (*wglCreateContextAttribsARB)(HDC, HGLRC, const int *) = NULL;
	wglCreateContextAttribsARB = (HGLRC (*)(HDC, HGLRC, const int *)) wglGetProcAddress("wglCreateContextAttribsARB");

	wglMakeCurrent(win.device_context, 0);
	wglDeleteContext(fake_context);
	
	const int attrib_list[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0
	};

	win.gl_context = wglCreateContextAttribsARB(win.device_context, 0, attrib_list);
	if (win.gl_context == NULL)
	{
		fprintf(stderr, "Failed to create GL extended context\n");
		exit(0);
	}

	if (!wglMakeCurrent(win.device_context, win.gl_context))
	{
		fprintf(stderr, "Failed to set GL extended context\n");
		exit(0);
	}

	return win;
}

void win_release_window_resources(struct window *win)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(win->gl_context);
	DestroyWindow(win->win_handle);
}
