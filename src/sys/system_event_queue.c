#include <string.h>

#include "system_public.h"
#include "system_local.h"

struct system_event_queue *g_sys_queue;

#ifdef MG_DEBUG
#include "mg_string.h"
//
//mg_string system_event_queue_debug_string(struct arena *mem, const struct system_event_queue *queue)
//{
//	return mg_string_empty();
//}
//
//mg_string system_event_debug_string(char *buf, const size_t buf_size, const struct system_event *event)
//{
//	switch (event->type)			
//	{
//		case MG_KEY_PRESSED:
//		{
//			return mg_string_format(buf, buf_size, key_pressed_string.str, event->timestamp, mg_keycode_to_string[event->value.key]);
//		} break;
//
//		case MG_KEY_RELEASED:
//		{
//			return mg_string_format(buf, buf_size, key_released_string.str, event->timestamp, mg_keycode_to_string[event->value.key]);				
//		} break;
//
//		case MG_BUTTON_PRESSED:
//		{
//			return mg_string_format(buf, buf_size, button_pressed_string.str, event->timestamp, event->value.button);				
//		} break;
//
//		case MG_BUTTON_RELEASED:
//		{
//			return mg_string_format(buf, buf_size, button_released_string.str, event->timestamp, event->value.button);				
//		} break;
//
//		case MG_MOTION:
//		{
//			return mg_string_format(buf, buf_size, motion_string.str, event->timestamp, event->value.motion[0], event->value.motion[1]);				
//		} break;
//
//		default:
//		{
//			assert(false && "Should not happen");
//		} break;
//	}
//			
//	return mg_string_empty();
//}

#endif

void system_event_queue_new(struct arena *arena)
{
	struct system_event_queue queue = 
	{ 
		.num_events = 0,
	};

	if (arena)
	{
		g_sys_queue = (struct system_event_queue *) arena_push(arena, &queue, sizeof(struct system_event_queue));
	}
	else
	{
		g_sys_queue = malloc(sizeof(struct system_event_queue));
		memcpy(g_sys_queue, &queue, sizeof(struct system_event_queue));
	}
}

i32 system_event_queue(const struct system_event *event)
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

void system_event_clear(void)
{
	g_sys_queue->num_events = 0;
}
