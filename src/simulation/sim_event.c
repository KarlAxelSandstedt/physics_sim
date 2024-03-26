#include "sim_public.h"
#include "sim_local.h"
#include "system_public.h"
#include "system_common.h"
#include "r_common.h"

void sim_process_system_events(struct simulation *sim, struct ui_state *ui, struct graphics_context *gc)
{
	system_push_events(&gc->win);
	
	/**
	 * Event loop:
	 * 	(1) - determine whether the user is interacting with the ui
	 *	      or the simulation.
	 *	(2) - On interaction, sys with an entity, get it's id.
	 *	(3) - When using the id, verify the entity still exists, 
	 *		if it exists, we may interacting with it, say,
	 *		by setting it's visual state:
	 *		if (mouse_over_entity(id))
	 *		{
	 *			entity.visuals.color = HIGHLIGHT_COLOR;
	 *		}
	 */
	for (u32 i = 0; i < g_sys_queue->num_events; ++i) 
	{
		const struct system_event *event = &g_sys_queue->events[i];
		switch (event->type)
		{
			case SYSTEM_BUTTON_PRESSED:
			{
				if (event->value.button < MG_BUTTON_NONMAPPED)
				{
					g_input_state->button_pressed[event->value.button] = 1;
					printf("Pressed: %s\n", mg_button_to_string(event->value.button));
				}
			} break;

			case SYSTEM_BUTTON_RELEASED:
			{
				if (event->value.button < MG_BUTTON_NONMAPPED)
				{
					g_input_state->button_pressed[event->value.button] = 0;
					printf("Released: %s\n", mg_button_to_string(event->value.button));
				}
			} break;

			case SYSTEM_KEY_PRESSED:
			{
				bit_vec_set_bit(&g_input_state->key_pressed, event->value.key, 1);
				switch (event->value.key)
				{
					case MG_L:
					case MG_l:
					{
						gc->win.cursor_locked_to_window = (gc->win.cursor_locked_to_window) ? 0 : 1;
					} break;

					case MG_A:
					case MG_a:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_LEFT,
							.value.pressed = 1,
						};
						r_cmd_add(&cam_movement);
					} break;

					case MG_D:
					case MG_d:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_RIGHT,
							.value.pressed = 1,
						};
						r_cmd_add(&cam_movement);
					} break;

					case MG_S:
					case MG_s:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_BACKWARD,
							.value.pressed = 1,
						};
						r_cmd_add(&cam_movement);
					} break;

					case MG_O:
					case MG_o:
					{
						sim->speed_scale *= 2.0f;
					} break;

					case MG_P:
					case MG_p:
					{
						sim->speed_scale *= 0.5f;
					} break;
		
					case MG_R:
					case MG_r:
					{
						sim->speed_scale = 1.0f;
					} break;

					case MG_W:
					case MG_w:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_FORWARD,
							.value.pressed = 1,
						};
						r_cmd_add(&cam_movement);
					} break;

					case MG_SHIFT: { g_input_state->shift_pressed = 1; } break;
					default:
					{
						printf("Unhandled Press: %s\n", mg_keycode_to_string(event->value.key));
					} break;
				}
			} break;

			case SYSTEM_KEY_RELEASED:
			{
				bit_vec_set_bit(&g_input_state->key_pressed, event->value.key, 0);
				switch (event->value.key)
				{
					case MG_F12: 
					{ 
						window_bordered(&gc->win, 1 - gc->win.border); 
						if (!gc->win.border)
						{
							window_maximize(&gc->win);
						}
						else
						{
							window_restore_border_size(&gc->win);
						}

						struct r_cmd cmd_viewport = 
						{
							.type = R_CMD_VIEWPORT,
						};
						r_cmd_add(&cmd_viewport);
					} break;

					case MG_A:
					case MG_a:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_LEFT,
							.value.pressed = 0,
						};
						r_cmd_add(&cam_movement);
					} break;

					case MG_D:
					case MG_d:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_RIGHT,
							.value.pressed = 0,
						};
						r_cmd_add(&cam_movement);
					} break;

					case MG_S:
					case MG_s:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_BACKWARD,
							.value.pressed = 0,
						};
						r_cmd_add(&cam_movement);
					} break;

					case MG_W:
					case MG_w:
					{
						struct r_cmd cam_movement = 
						{
							.type = R_CMD_FORWARD,
							.value.pressed = 0,
						};
						r_cmd_add(&cam_movement);
					} break;


					case MG_Q: 
					case MG_q: sim->running = 0; break;
					case MG_SHIFT: { g_input_state->shift_pressed = 0; } break;
					default:
					{
						printf("Unhandled Release: %s\n", mg_keycode_to_string(event->value.key));
					} break;
				}
			} break;

			case SYSTEM_CURSOR_POSITION:
			{
				if (!gc->win.skip_next_motion)
				{
				      	struct r_cmd cam_motion =
				      	{
						.type = R_CMD_MOTION,
				      		.value.motion = 
				      		{
				      			event->value.cursor_position[0] - g_input_state->cursor_position[0],
				      			event->value.cursor_position[1] - g_input_state->cursor_position[1],
				      		},
				      	};
					
					if (gc->win.counter_motion_waiting)
					{
						cam_motion.value.motion[0] += gc->win.counter_motion[0];
						cam_motion.value.motion[1] += gc->win.counter_motion[1];
						gc->win.counter_motion_waiting = 0;
						gc->win.counter_motion[0] = 0;
						gc->win.counter_motion[1] = 0;
					}
				      	r_cmd_add(&cam_motion);
				}
				else
				{
					gc->win.skip_next_motion = 0;
				}
				
				vec2i32_copy(g_input_state->cursor_position, event->value.cursor_position);
			} break;

			case SYSTEM_COUNTER_MOTION:
			{
				/* apply counter motion on next cursor_position change */
				gc->win.counter_motion_waiting = 1;
				gc->win.counter_motion[0] += event->value.cursor_motion[0];
				gc->win.counter_motion[1] += event->value.cursor_motion[1];
			} break;

			case SYSTEM_WINDOW_CLOSE:
			{
				sim->running = 0;
			} break;

			case SYSTEM_WINDOW_CURSOR_ENTER:
			{
				gc->win.cursor_in_window = 1;
			} break;

			case SYSTEM_WINDOW_CURSOR_LEAVE:
			{
				gc->win.skip_next_motion = 1;
				gc->win.cursor_in_window = 0;
			} break;

			case SYSTEM_WINDOW_FOCUS_IN:
			{
				gc->win.focused = 1;
			} break;

			case SYSTEM_WINDOW_FOCUS_OUT:
			{
				gc->win.focused = 0;
			} break;

			case SYSTEM_WINDOW_EXPOSE:
			{
				gc->win.visible = 1;
				printf("visible\n");
			} break;

			case SYSTEM_WINDOW_CONFIG:
			{
				window_config_update(&gc->win);
			} break;

			case SYSTEM_WINDOW_MINIMIZE:
			{
				gc->win.visible = 0;
				printf("minimized\n");
			} break;

			case SYSTEM_NO_EVENT:
			{
				printf("SYSTEM_NO_EVENT\n");
			} break;
		}
	}

	system_event_clear();
}
