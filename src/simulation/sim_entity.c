#include "sim_public.h"
#include "sim_local.h"

void entity_construct_random(struct arena *mem, struct simulation *sim, const u64 index, const f32 min_radius, const f32 max_radius, const u32 min_v_count, const u32 max_v_count, struct arena_collection *mem_tmp, const vec3 pos)
{
	++sim->entity_count;
	sim->entities[index].id = index;
	sim->entities[index].active = 1;
	vec4_set(sim->visuals[index].color, gen_rand_f(), gen_rand_f(), gen_rand_f(), 0.5f);
	rbp_construct_random(mem, &sim->pipeline, index,  min_radius, max_radius, min_v_count, max_v_count, mem_tmp, pos);
}

void entity_construct_random_at_origin(struct arena *mem, struct simulation *sim, const u64 index, const f32 min_radius, const f32 max_radius, const u32 min_v_count, const u32 max_v_count, struct arena_collection *mem_tmp)
{
	const vec3 pos = { 0.0f, 0.0f, 0.0f };
	entity_construct_random(mem, sim, index, min_radius, max_radius, min_v_count, max_v_count, mem_tmp, pos);
}

void entity_push_convex_hull(struct drawbuffer *buf, struct simulation *sim, const u64 index)
{
	struct rigid_body *body = &sim->pipeline.bodies[index];
	convex_hull_cs_step_draw(sim->mem_frame, &sim->mem_tmp.arenas[1], &sim->mem_tmp.arenas[2], &sim->mem_tmp.arenas[3], &sim->mem_tmp.arenas[4], (f32 *)body->mesh.v, body->mesh.v_count, 0.0001f, body->mesh.v_count, 7583458, buf, sim->visuals[index].color, 0);
}
