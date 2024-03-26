#include "win_public.h"
#include "win_local.h"
#include "system_common.h"

#define SYS_EVENT_ADD(event_addr) (win_system_event_queue(event_addr))

i32 win_system_event_queue(const struct system_event *event)
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

LRESULT CALLBACK windows_event_callback(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
	struct system_event event;
	LRESULT result = 0;

    	switch (message)
	{ 
    		case WM_SIZE: 
		{
			fprintf(stderr, "WM_SIZE\n");
		} break; 

    		case WM_CLOSE: 
		{
			event.type = SYSTEM_WINDOW_CLOSE;
			SYS_EVENT_ADD(&event);
		} break; 

		case WM_DESTROY:
		{
			fprintf(stderr, "WM_DESTROY\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			fprintf(stderr, "WM_ACTIVATEAPP\n");
		} break;

		case WM_SYSKEYDOWN: 
		{ 
			fprintf(stderr, "WM_SYSKEYDOWN\n");
		}

		case WM_SYSKEYUP:
		{
			fprintf(stderr, "WM_SYSKEYUP\n");
		} break;

		case WM_KEYDOWN: 
		{ 
			event.type = SYSTEM_KEY_PRESSED;
			event.value.key = win_mg_keycode_lookup((u32) w_param);
			SYS_EVENT_ADD(&event);
		} break;

		case WM_KEYUP:
		{
			event.type = SYSTEM_KEY_RELEASED;
			event.value.key = win_mg_keycode_lookup((u32) w_param);
			SYS_EVENT_ADD(&event);
		} break;

		default:
		{
			result = DefWindowProc(window_handle, message, w_param, l_param);
		} break;
	}

	return result;
}

void win_system_push_events(struct window *win)
{
	MSG message;
	while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}
