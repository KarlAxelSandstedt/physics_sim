#include "r_public.h"
#include "r_local.h"
#include "mgl.h"
#include "timer.h"

void r_draw(struct render_state *re, struct window *win)
{
	mglClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	mglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawbuffer_draw(&re->gl, &re->entity_buf, GL_TRIANGLES);
	//mglDisable(GL_DEPTH_TEST);
	drawbuffer_draw(&re->gl, &re->color_buf, GL_LINES);
	//mglEnable(GL_DEPTH_TEST);

	mglSwapBuffers(win);
}

void update_uniforms(struct render_state *re, const struct simulation *sim, struct arena *mem, const f64 delta)
{
	mat4 perspective, view;
	struct camera *cam = &re->cam;

	perspective_matrix(perspective, cam->aspect_ratio, cam->fov_x, cam->fz_near, cam->fz_far);
	view_matrix(view, cam->position, cam->left, cam->up, cam->forward);

	GLint pp_addr = mglGetUniformLocation(re->color_prg, "perspective");
	GLint view_addr = mglGetUniformLocation(re->color_prg, "view");
	mglUseProgram(re->color_prg);	
	mglUniformMatrix4fv(pp_addr, 1, GL_FALSE, (f32 *) perspective);
	mglUniformMatrix4fv(view_addr, 1, GL_FALSE, (f32 *) view);

	pp_addr = mglGetUniformLocation(re->lightning_prg, "perspective");
	view_addr = mglGetUniformLocation(re->lightning_prg, "view");
	GLint light_position_addr = mglGetUniformLocation(re->lightning_prg, "light_position");
	mglUseProgram(re->lightning_prg);	
	mglUniformMatrix4fv(pp_addr, 1, GL_FALSE, (f32 *) perspective);
	mglUniformMatrix4fv(view_addr, 1, GL_FALSE, (f32 *) view);
	mglUniform3f(light_position_addr, re->cam.position[0], re->cam.position[1], re->cam.position[2]);

	vec3ptr transforms = (vec3ptr) arena_push_packed(mem, NULL, sizeof(vec3)*sim->entity_count);
	for (u64 i = 0; i < sim->entity_count; ++i)
	{
		if (sim->entities[i].active)
		{
			vec3_copy(transforms[i], sim->pipeline.bodies[i].position);
		}
	}
	i32 loc = mglGetUniformLocation(re->lightning_prg, "transform");
	mglUniform3fv(loc, sim->entity_count, (f32 *) transforms);
	arena_pop_packed(mem, sizeof(vec3)*sim->entity_count);

	loc = mglGetUniformLocation(re->lightning_prg, "collision");
	mglUniform1iv(loc, sim->entity_count, sim->phy_out.collisions);

	assert(sim->entity_count <= 256 && "We have to do partial buffering and drawing of transforms");

	mglUseProgram(0);
	re->gl.prg_bound = 0;
}

	vec3 cam_local_velocity = { 0.0f, 0.0f, 0.0f };
void r_main(struct render_state *re, struct graphics_context *gtx, struct simulation *sim, struct ui_state *ui, const f64 delta)
{
	for (u32 i = 0; i < g_r_cmd_queue->cmd_count; ++i)
	{
		struct r_cmd *cmd = g_r_cmd_queue->cmds + i;
		switch (cmd->type)
		{
			case R_CMD_MOTION:
			{
				camera_update_angles(&re->cam, -cmd->value.motion[0] / 300.0f, -cmd->value.motion[1] / 300.0f);
			} break;

			case R_CMD_LEFT:     { cam_local_velocity[0] += (cmd->value.pressed) ?  5.0f : -5.0f; } break;
			case R_CMD_RIGHT:    { cam_local_velocity[0] += (cmd->value.pressed) ? -5.0f :  5.0f; } break;
			case R_CMD_FORWARD:  { cam_local_velocity[2] += (cmd->value.pressed) ?  5.0f : -5.0f; } break;
			case R_CMD_BACKWARD: { cam_local_velocity[2] += (cmd->value.pressed) ? -5.0f :  5.0f; } break;
			case R_CMD_VIEWPORT: { mglViewport(0, 0, gtx->win.size[0], gtx->win.size[1]); } break;

			case R_CMD_COUNT: assert(0 && "Should not happen"); break;
			default: 	  assert(0 && "To be implemented"); break;
		}	
	}
	r_cmd_queue_clear();

	camera_update_axes(&re->cam);
	re->cam.position[0] += delta * (cam_local_velocity[0] * re->cam.left[0] +  cam_local_velocity[2] * re->cam.forward[0]);
	re->cam.position[1] += delta * (cam_local_velocity[1] + cam_local_velocity[2] * re->cam.forward[1]);
	re->cam.position[2] += delta * (cam_local_velocity[0] * re->cam.left[2] +  cam_local_velocity[2] * re->cam.forward[2]);

	/* (1) push camera uniforms */
	update_uniforms(re, sim, sim->mem_frame, delta);

	vec4 dbvt_color = { 1.0f, 0.0f, 0.0f, 0.7f };
	dbvt_push_lines(&re->color_buf, &sim->pipeline.dynamic_tree, dbvt_color);

	/* (2) fill drawbuffers */
	for (u64 i = 0; i < sim->entity_count; ++i)
	{
		if (sim->pipeline.bodies[i].active)
		{
			entity_push_convex_hull(&re->entity_buf, sim, i);	

			struct AABB world_box = sim->pipeline.bodies[i].local_box;
			vec3_translate(world_box.center, sim->pipeline.bodies[i].position);
			AABB_push_lines(&re->color_buf, &world_box, dbvt_color); 
		}
	}

	for (u32 i = 0; i < sim->phy_out.point_pairs_count; ++i)
	{
		line_push_random_color(&re->color_buf, sim->phy_out.closest_point_pairs[2*i], sim->phy_out.closest_point_pairs[2*i + 1]);
	}

	/* (3) send data to gpu */
	drawbuffer_buffer_data(&re->gl, &re->entity_buf);
	drawbuffer_buffer_data(&re->gl, &re->color_buf);

	/* (4) draw */
	r_draw(re, &gtx->win);

	drawbuffer_clear(&re->entity_buf);
	drawbuffer_clear(&re->color_buf);
}
