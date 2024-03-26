#include "sim_public.h"
#include "sim_local.h"
#include "mmath.h"

static void sim_clear_frame(struct simulation *sim)
{
	sim->phy_out = physics_output_cleared();
	arena_flush(sim->mem_frame);
}

void sim_main(struct simulation *sim, const f64 delta)
{
	sim_clear_frame(sim);
	sim->simulation_method(sim, sim->speed_scale * delta);
}

static void convex_volume_intersection_simulation_setup(struct simulation *sim, struct arena *mem_persistent)
{
	/* (1) Setup rng */
	mersenne_twister_init(sim->seed);

	sim->mem_persistent = mem_persistent;
	sim->mem_frame = arena_push(mem_persistent, NULL, sizeof(struct arena));
	*sim->mem_frame = arena_alloc(1024*1024);
	sim->mem_tmp.arenas = arena_push(sim->mem_persistent, NULL, 5 * sizeof(struct arena));
	sim->mem_tmp.arenas[0] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[1] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[2] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[3] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[4] = arena_alloc(64*1024);
	sim->mem_tmp.arena_count = 5;

	/* (2) Setup rigid bodies */
	/* TODO: rewrite pipeline to use non-contiguous shit...? */
	sim->pipeline = rbp_new(mem_persistent, CVI_BODIES);
	sim->entities = entities_init_default(mem_persistent, CVI_BODIES);
	sim->visuals = visuals_init_default(mem_persistent, CVI_BODIES);

	/* (3) Setup entity, rigid_body and visuals  */
	entity_construct_random_at_origin(mem_persistent, sim, CVI_LARGE_INDEX,  1.5f, 1.5f, 70, 70, &sim->mem_tmp);
	entity_construct_random_at_origin(mem_persistent, sim, CVI_SMALL1_INDEX, 0.5f, 0.5f, 70, 70, &sim->mem_tmp);
	entity_construct_random_at_origin(mem_persistent, sim, CVI_SMALL2_INDEX, 0.5f, 0.5f, 70, 70, &sim->mem_tmp);
	entity_construct_random_at_origin(mem_persistent, sim, CVI_SMALL3_INDEX, 0.5f, 0.5f, 70, 70, &sim->mem_tmp);

	/* (5) push stuff easily into render's drawbuffer */
	//struct dbvt tree = dbvt_alloc(&global_arena, num_bodies);
	//convex_bodies_init_tree(&tree, bodies, num_bodies);
	//drawbuffer_buffer_data(&hull_buffer);
}

static void convex_volume_intersection_determine_positions(f32 *interpolation, vec3 s1, vec3 s2, vec3 s3, vec3 e1, vec3 e2, vec3 e3, const f32 time, const f32 test_time)
{
	const f32 half_width = 1.5f;

	const vec4 big_color = { 0.1f, 0.4f, 0.8f, 0.2f };
	const vec4 small_color = { 0.1f, 0.8f, 0.4f, 0.2f };
	const vec4 intersect_color = {1.0f, 0.7f, 0.f, 0.5f };

	*interpolation = 0.0f;

	/* (1) determine rigid body routes */
	
	/* X-Test */
	if (time <= test_time / 3.0f)
	{
		vec3_set(s1,  0.5f + half_width,  0.0f, half_width);
		vec3_set(s2,  0.5f + half_width,  0.0f, 0.0f);
		vec3_set(s3,  0.5f + half_width,  0.0f, -half_width);
		vec3_set(e1, -0.5f - half_width, 0.0f,  half_width);
		vec3_set(e2, -0.5f - half_width, 0.0f,  0.0f);
		vec3_set(e3, -0.5f - half_width, 0.0f,  -half_width);

		if (time - 0.0f <= test_time / 9.0f) 
		{
			s1[1] = half_width;
			s2[1] = half_width;
			s3[1] = half_width;
			e1[1] = half_width;
			e2[1] = half_width;
			e3[1] = half_width;
			*interpolation = (test_time / 9.0f - time) / (test_time / 9.0f);
	       	}
		else if (time - 0.0f <= 2.0f * test_time / 9.0f)
		{
			s1[1] = 0.0f;
			s2[1] = 0.0f;
			s3[1] = 0.0f;
			e1[1] = 0.0f;
			e2[1] = 0.0f;
			e3[1] = 0.0f;
			
			*interpolation = (2.0f * test_time / 9.0f - time) / (test_time / 9.0f);
		}
		else
		{
			s1[1] = -half_width;
			s2[1] = -half_width;
			s3[1] = -half_width;
			e1[1] = -half_width;
			e2[1] = -half_width;
			e3[1] = -half_width;

			*interpolation = (3.0f * test_time / 9.0f - time) / (test_time / 9.0f);
		}

	}
	/* Y-Test */
	else if (time <= 2.0f * test_time / 3.0f)
	{
		vec3_set(s1, half_width,   0.5f + half_width, 0.0f); 
		vec3_set(s2, 0.0f,         0.5f + half_width, 0.0f);
		vec3_set(s3, -half_width,  0.5f + half_width, 0.0f);
		vec3_set(e1, half_width,  -0.5f - half_width, 0.0f);
		vec3_set(e2, 0.0f,        -0.5f - half_width, 0.0f);
		vec3_set(e3, -half_width, -0.5f - half_width, 0.0f);

		if (time - test_time / 3.0f <= test_time / 9.0f) 
		{
			s1[2] = -half_width;
			s2[2] = -half_width;
			s3[2] = -half_width;
			e1[2] = -half_width;
			e2[2] = -half_width;
			e3[2] = -half_width;

			*interpolation = (4.0f * test_time / 9.0f - time) / (test_time / 9.0f);
	       	}
		else if (time - test_time / 3.0f <= 2.0f * test_time / 9.0f)
		{
			s1[2] = 0.0f;
			s2[2] = 0.0f;
			s3[2] = 0.0f;
			e1[2] = 0.0f;
			e2[2] = 0.0f;
			e3[2] = 0.0f;
			
			*interpolation = (5.0f * test_time / 9.0f - time) / (test_time / 9.0f);
		}
		else
		{
			s1[2] = half_width;
			s2[2] = half_width;
			s3[2] = half_width;
			e1[2] = half_width;
			e2[2] = half_width;
			e3[2] = half_width;
			
			*interpolation = (6.0f * test_time / 9.0f - time) / (test_time / 9.0f);
		}

	}
	/* Z-Test */
	else
	{
		vec3_set(s1, half_width,  0.0f, -0.5f - half_width); 
		vec3_set(s2, 0.0f,        0.0f, -0.5f - half_width);
		vec3_set(s3, -half_width, 0.0f, -0.5f - half_width);
		vec3_set(e1, half_width,  0.0f,  0.5f + half_width);
		vec3_set(e2, 0.0f,        0.0f,  0.5f + half_width);
		vec3_set(e3, -half_width, 0.0f,  0.5f + half_width);

		if (time - 2.0f * test_time / 3.0f <= test_time / 9.0f) 
		{
			s1[1] = half_width;
			s2[1] = half_width;
			s3[1] = half_width;
			e1[1] = half_width;
			e2[1] = half_width;
			e3[1] = half_width;

			*interpolation = (7.0f * test_time / 9.0f - time) / (test_time / 9.0f);
	       	}
		else if (time -  2.0f * test_time / 3.0f <= 2.0f * test_time / 9.0f)
		{
			s1[1] = 0.0f;
			s2[1] = 0.0f;
			s3[1] = 0.0f;
			e1[1] = 0.0f;
			e2[1] = 0.0f;
			e3[1] = 0.0f;
			*interpolation = (8.0f * test_time / 9.0f - time) / (test_time / 9.0f);
		}
		else
		{
			s1[1] = -half_width;
			s2[1] = -half_width;
			s3[1] = -half_width;
			e1[1] = -half_width;
			e2[1] = -half_width;
			e3[1] = -half_width;

			*interpolation = (9.0f * test_time / 9.0f - time) / (test_time / 9.0f);
		}
	}
}

void convex_volume_intersection_simulation(struct simulation *sim, const f64 delta)
{
	static i32 on_first_entry = 1;
	static struct arena mem_persistent;
	if (on_first_entry)
	{
		--on_first_entry;
		mem_persistent = arena_alloc(16*1024*1024);
		convex_volume_intersection_simulation_setup(sim, &mem_persistent);
	}

	sim->time += delta;
	const f64 test_time = 27.0;
	if (sim->time >= test_time) { sim->time -= test_time; }

	vec3 s1, s2, s3, e1, e2, e3; /* start and end points for small bodies */
	f32 interpolation; /* relative small body positions between start and end points */
	convex_volume_intersection_determine_positions(&interpolation, s1, s2, s3, e1, e2, e3, sim->time, test_time);

	vec3_interpolate(sim->pipeline.bodies[CVI_SMALL1_INDEX].position, s1, e1, interpolation);
	vec3_interpolate(sim->pipeline.bodies[CVI_SMALL2_INDEX].position, s2, e2, interpolation);
	vec3_interpolate(sim->pipeline.bodies[CVI_SMALL3_INDEX].position, s3, e3, interpolation);

	sim->phy_out.collisions = rbp_simulate(sim->mem_frame, &sim->pipeline, delta);
	//TODO: need to set out aswell
	//sim->phy_out.closest_point_pairs = rbp_push_closest_points_between_bodies(sim->mem_frame, &sim->pipeline, &sim->point_pairs_count);
}

static void gravity_simulation_setup(struct simulation *sim, struct arena *mem_persistent)
{
	/* (1) Setup rng */
	mersenne_twister_init(sim->seed);

	sim->mem_persistent = mem_persistent;
	sim->mem_frame = arena_push(mem_persistent, NULL, sizeof(struct arena));
	*sim->mem_frame = arena_alloc(1024*1024);
	sim->mem_tmp.arenas = arena_push(sim->mem_persistent, NULL, 5 * sizeof(struct arena));
	sim->mem_tmp.arenas[0] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[1] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[2] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[3] = arena_alloc(64*1024);
	sim->mem_tmp.arenas[4] = arena_alloc(64*1024);
	sim->mem_tmp.arena_count = 5;

	/* (2) Setup rigid bodies */
	/* TODO: rewrite pipeline to use non-contiguous shit...? */
	sim->pipeline = rbp_new(sim->mem_persistent, G_BODIES);
	sim->entities = entities_init_default(sim->mem_persistent, G_BODIES);
	sim->visuals = visuals_init_default(sim->mem_persistent, G_BODIES);

	vec3 pos_1, pos_2, pos_3, pos_4;
	vec3_set(pos_1, 0.0f, 0.0f, 0.0f);
	vec3_set(pos_2, 1.0f, 3.0f, 1.0f);
	vec3_set(pos_3, 0.0f, 3.0f, 0.0f);
	vec3_set(pos_4, -1.0f, 3.0f, -1.0f);

	/* (3) Setup entity, rigid_body and visuals  */
	entity_construct_random(sim->mem_persistent, sim, G_LARGE_INDEX,  1.5f, 1.5f, 70, 70, &sim->mem_tmp, pos_1);
	entity_construct_random(sim->mem_persistent, sim, G_SMALL1_INDEX, 0.5f, 0.5f, 70, 70, &sim->mem_tmp, pos_2);
	entity_construct_random(sim->mem_persistent, sim, G_SMALL2_INDEX, 0.5f, 0.5f, 70, 70, &sim->mem_tmp, pos_3);
	entity_construct_random(sim->mem_persistent, sim, G_SMALL3_INDEX, 0.5f, 0.5f, 70, 70, &sim->mem_tmp, pos_4);

	vec3 box[8] =
	{
		{ -10.0f, -10.0f, -10.0f },
		{ -10.0f, -10.0f,  10.0f },
		{ -10.0f,  -5.0f, -10.0f },
		{ -10.0f,  -5.0f,  10.0f },
		{ 10.0f,  -10.0f, -10.0f },
		{ 10.0f,  -10.0f,  10.0f },
		{ 10.0f,   -5.0f, -10.0f },
		{ 10.0f,   -5.0f,  10.0f },
	};

	struct rigid_body floor;
	struct tri_mesh mesh = convex_hull_construct(
			sim->mem_persistent,
		        sim->mem_tmp.arenas + 0,
		       	sim->mem_tmp.arenas + 1,
		       	sim->mem_tmp.arenas + 2, 
			sim->mem_tmp.arenas + 3, 
			sim->mem_tmp.arenas + 4,
		       	box,
		       	8,
		       	100.0f * FLT_EPSILON);

	const f32 density = 1.0f;
	statics_setup(&floor, sim->mem_persistent, &mesh, density);
	floor.margin = 1.0f;
	rbp_add(&sim->pipeline, G_FLOOR_INDEX, &floor, 0);

	++sim->entity_count;
	sim->entities[G_FLOOR_INDEX].id = G_FLOOR_INDEX;
	sim->entities[G_FLOOR_INDEX].active = 1;
	vec4_set(sim->visuals[G_FLOOR_INDEX].color, gen_rand_f(), gen_rand_f(), gen_rand_f(), 1.0f);
}

void gravity_simulation(struct simulation *sim, const f64 delta)
{
	static i32 on_first_entry = 1;
	static struct arena mem_persistent;
	if (on_first_entry)
	{
		--on_first_entry;
		mem_persistent = arena_alloc(1024*1024);
		gravity_simulation_setup(sim, &mem_persistent);
	}

	sim->time += delta;
 	sim->phy_out = rbp_simulate_frame(sim->mem_frame, &sim->pipeline, delta);
}
