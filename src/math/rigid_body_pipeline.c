#include <stdlib.h>
#include <string.h>
#include "rigid_body_pipeline.h"

#define UNIFORM_SIZE 256
#define GRAVITY_CONSTANT_DEFAULT 9.80665f

struct rbp rbp_new(struct arena *mem, const i32 size)
{
	struct rbp pipeline =
	{
		.size = size,
		.count = 0,
		.gravity = { 0.0f, -GRAVITY_CONSTANT_DEFAULT, 0.0f },
	};

	if (mem)
	{
		pipeline.bodies = arena_push(mem, NULL, size * sizeof(struct rigid_body));	
		pipeline.dynamic_tree = dbvt_alloc(mem, 2*size);
	}
	else
	{
		pipeline.bodies = malloc(size * sizeof(struct rigid_body));	
		pipeline.dynamic_tree = dbvt_alloc(mem, 2*size);
	}

	for (i32 i = 0; i < size; ++i)
	{
		pipeline.bodies[i].active = 0;			
	}

	return pipeline;
}

void rbp_add(struct rbp *pipeline, const i32 index, struct rigid_body *body, u32 dynamic)
{
	assert(index >= 0 && index < pipeline->size);
	assert(pipeline->bodies[index].active == 0);

	memcpy(pipeline->bodies + index, body, sizeof(struct rigid_body));
	pipeline->bodies[index].active = 1;
	pipeline->bodies[index].dynamic = dynamic;
	pipeline->count += 1;

	struct AABB proxy;
	rigid_body_proxy(&proxy, &pipeline->bodies[index]);
	pipeline->bodies[index].proxy = dbvt_insert(&pipeline->dynamic_tree, index, &proxy);
}

void rbp_remove(struct rbp *pipeline, const i32 index)
{
	assert(index >= 0 && index < pipeline->size);
	assert(pipeline->bodies[index].active == 1);

	pipeline->bodies[index].active = 0;
	pipeline->count -= 1;
}

void rbp_push_dbvt(struct drawbuffer *buf, struct rbp *pipeline, const vec4 color)
{
	dbvt_push_lines(buf, &pipeline->dynamic_tree, color);
}

void rbp_push_proxies(struct drawbuffer *buf, const struct rbp *pipeline, const vec4 color)
{
	for (i32 i = 0; i < pipeline->size; ++i)
	{
		if (pipeline->bodies[i].active)
		{
			AABB_push_lines(buf, &pipeline->dynamic_tree.nodes[pipeline->bodies[i].proxy].box, color);
		}
	}
}

void rbp_push_convex_hulls(const struct rbp *pipeline, struct drawbuffer *buf, const vec4 color, struct arena *mem_1, struct arena *mem_2, struct arena *mem_3, struct arena *mem_4, struct arena *mem_5, i32 i_count[], i32 i_offset[])
{
	const i32 len = (pipeline->size % UNIFORM_SIZE) ? pipeline->size / UNIFORM_SIZE + 1 : pipeline->size / UNIFORM_SIZE;
	for (i32 i = 0; i < len; ++i)
	{
		i_count[i] = 0;
		i_offset[i] = 0;
	}

	const void *first = buf->i_buf.stack_ptr;

	for (i32 i = 0; i < pipeline->size; ++i)
	{
		const void *before = buf->i_buf.stack_ptr;
		if (pipeline->bodies[i].active)
		{
			convex_hull_cs_step_draw(mem_1, mem_2, mem_3, mem_4, mem_5, pipeline->bodies[i].mesh.v, pipeline->bodies[i].mesh.v_count, 0.0001f, pipeline->bodies[i].mesh.v_count, 7583458, buf, color, 0);
		}
		const void *after = buf->i_buf.stack_ptr;
		i_count[i / UNIFORM_SIZE] += ((u64) after - (u64) before) / sizeof(i32);
		if (i % UNIFORM_SIZE == 0)
		{
			i_offset[i / UNIFORM_SIZE] = ((u64) before - (u64) first);
		}
	}
}

void rbp_push_positions(void *buf, const struct rbp *pipeline)
{

}

static void internal_update_bodies(struct rbp *pipeline, const f32 delta)
{
	vec3 translation;
	struct AABB world_AABB;
	for (i32 i = 0; i < pipeline->size; ++i)
	{
		if (pipeline->bodies[i].active)
		{
			vec3_scale(translation, pipeline->bodies[i].velocity, delta);
			vec3_translate(pipeline->bodies[i].position, translation);
			vec3_add(world_AABB.center, pipeline->bodies[i].local_box.center, pipeline->bodies[i].position);
			vec3_copy(world_AABB.hw, pipeline->bodies[i].local_box.hw);
			const struct AABB *proxy = &pipeline->dynamic_tree.nodes[pipeline->bodies[i].proxy].box;
			if (!AABB_contains(proxy, &world_AABB))
			{
				world_AABB.hw[0] += pipeline->bodies[i].margin;
				world_AABB.hw[1] += pipeline->bodies[i].margin;
				world_AABB.hw[2] += pipeline->bodies[i].margin;
				dbvt_remove(&pipeline->dynamic_tree, pipeline->bodies[i].proxy);
				pipeline->bodies[i].proxy = dbvt_insert(&pipeline->dynamic_tree, i, &world_AABB);
			}
		}
	}

}

static i32 internal_push_proxy_overlaps(struct arena *mem_frame, struct rbp *pipeline)
{
	return dbvt_push_overlap_pairs(mem_frame, &pipeline->dynamic_tree);
}

static i32 *internal_push_collisions(struct arena *mem_frame, struct rbp *pipeline, i32 *overlaps, const i32 overlap_count)
{
	i32 *collisions = arena_push_packed(mem_frame, NULL, sizeof(i32)*pipeline->size);
	for (i32 i = 0; i < pipeline->size; ++i) { collisions[i] = 0; }
	i32 num_collisions = 0;

	struct arena record = *mem_frame;

	struct rigid_body *b1, *b2;
	for (i32 i = 0; i < overlap_count; ++i)
	{
		b1 = pipeline->bodies + overlaps[2*i];
		b2 = pipeline->bodies + overlaps[2*i+1];

		struct contact_manifold c_m;
		//if (GJK_test(b1->position, b1->v, b1->v_count, b2->position, b2->v, b2->v_count, 0.0, 100.0f*FLT_EPSILON))
		if (GJK_EPA(mem_frame, &c_m, b1->position, b1->mesh.v, b1->mesh.v_count, b2->position, b2->mesh.v, b2->mesh.v_count, 0.001f, 100.0f*FLT_EPSILON))
		{
			num_collisions += 1;
			collisions[overlaps[2*i]] = 1;
			collisions[overlaps[2*i+1]] = 1;
		}
	}

	*mem_frame = record;
	return collisions;
}

vec3ptr rbp_push_closest_points_between_bodies(struct arena *mem_frame, struct rbp *pipeline, u32 *pair_count)
{
	*pair_count = 0;
	struct rigid_body *b1, *b2;
	vec3ptr point_pairs = arena_push(mem_frame, NULL, 2*sizeof(vec3)*pipeline->size*(pipeline->size-1)/2);

	for (i32 i = 0; i < pipeline->size; ++i)
	{
		b1 = pipeline->bodies + i;
		for (i32 j = i+1; j < pipeline->size; ++j)
		{
			b2 = pipeline->bodies + j;
			struct contact_manifold c_m;
			if (GJK_EPA(mem_frame, &c_m, b1->position, b1->mesh.v, b1->mesh.v_count, b2->position, b2->mesh.v, b2->mesh.v_count, 0.001f, 100.0f*FLT_EPSILON))
			//if (GJK_distance(point_pairs[2*(*pair_count)], point_pairs[2*(*pair_count) + 1],
			//			b1->position, b1->v, b1->v_count, b2->position, b2->v, b2->v_count, 0.001f, 100.0f*FLT_EPSILON) > 0.0f)
			{
				printf("PENETRATION_DEPTH: %f\n", c_m.penetration_depth);
				vec3 pen_dir;
				vec3_sub(pen_dir, c_m.p_1, c_m.p_2);
				vec3_translate(b2->position, pen_dir);
				vec3_copy(point_pairs[2*(*pair_count)], c_m.p_1);
				vec3_copy(point_pairs[2*(*pair_count) + 1], c_m.p_2);
				*pair_count += 1;
			}
		}
	}

	return point_pairs;
}

i32 *rbp_simulate(struct arena *mem_frame, struct rbp *pipeline, const f32 delta)
{
	internal_update_bodies(pipeline, delta);

	i32 *overlaps = (i32 *) mem_frame->stack_ptr;
	i32 overlap_pairs_count = internal_push_proxy_overlaps(mem_frame, pipeline);
	i32 *collisions = internal_push_collisions(mem_frame, pipeline, overlaps, overlap_pairs_count);

	return collisions;
}

void rbp_construct_random(struct arena *mem, struct rbp *pipeline, const u64 index, const f32 min_radius, const f32 max_radius, const u32 min_v_count, const u32 max_v_count, struct arena_collection *mem_tmp, const vec3 pos)
{
	assert(mem_tmp->arena_count >= 5);

	struct rigid_body body;
	const f32 radius = gen_continuous_uniform_f(min_radius, max_radius);
	const f32 t1 = gen_rand_f();
	const f32 t2 = gen_rand_f();

	struct arena record = mem_tmp->arenas[4];
	u32 v_count = (u32) gen_continuous_uniform_f(min_v_count, (f32) max_v_count + 0.99f); 
	vec3ptr v = arena_push_packed(&mem_tmp->arenas[4], NULL, v_count * sizeof(vec3));

	vec3_set(body.velocity, 0.0f, 0.0f, 0.0f);
	//vec3_set(body.velocity, 
	//	gen_continuous_uniform_f(-1.0f, 1.0f),
	//	gen_continuous_uniform_f(-1.0f, 1.0f),
	//	gen_continuous_uniform_f(-1.0f, 1.0f));
	//vec3_mul_constant(body.velocity, 1.0f / vec3_length(body.velocity));
	vec3_set(body.position, 0.0f, 0.0f, 0.0f);
	for (u32 j = 0; j < v_count; ++j)
	{
		const f32 u1 = gen_continuous_uniform_f(0.0f, 1.0f);
		const f32 u2 = gen_continuous_uniform_f(0.0f, 1.0f);
		const f32 phi = acosf(2*u1 - 1.0f) - MM_PI_F / 2.0f;
		const f32 lambda = 2*MM_PI_F*u2;
		vec3_set(v[j]
				,pos[0] + radius*cosf(phi)*cosf(lambda)
				,pos[1] + radius*cosf(phi)*sinf(lambda)
				,pos[2] + radius*sinf(phi));
	}

	/* TODO:
	 * (O) generate hull
	 * (O) generate statics
	 * () generate dynamics 
	 */
	{
		const f32 density = 1.0;
		struct tri_mesh mesh = convex_hull_construct(mem, mem_tmp->arenas + 0, mem_tmp->arenas + 1, mem_tmp->arenas + 2, mem_tmp->arenas + 3, mem_tmp->arenas + 4, v, v_count, 100.0f * FLT_EPSILON);
		statics_setup(&body, mem, &mesh, density);
	}

	mem_tmp->arenas[4] = record;

	body.margin = 1.00f;
	rbp_add(pipeline, index, &body, 1);
}

static void rbp_internal_integrate(struct arena *mem_frame, struct rbp *pipeline, const f32 delta)
{
	vec3 velocity, acceleration, force, torque, tmp;
	struct AABB world_AABB;

	for (i32 i = 0; i < pipeline->count; ++i)
	{
		struct rigid_body *b = pipeline->bodies + i;
		if (b->active && b->dynamic)
		{
			assert(b->mass > 0.0f);

			vec3_scale(velocity, b->linear_momentum, 1.0f/b->mass);
			vec3_translate_scaled(b->position, velocity, delta);

			vec3_add(world_AABB.center, b->local_box.center, pipeline->bodies[i].position);
			vec3_copy(world_AABB.hw, b->local_box.hw);
			const struct AABB *proxy = &pipeline->dynamic_tree.nodes[b->proxy].box;
			if (!AABB_contains(proxy, &world_AABB))
			{
				world_AABB.hw[0] += pipeline->bodies[i].margin;
				world_AABB.hw[1] += pipeline->bodies[i].margin;
				world_AABB.hw[2] += pipeline->bodies[i].margin;
				dbvt_remove(&pipeline->dynamic_tree, pipeline->bodies[i].proxy);
				pipeline->bodies[i].proxy = dbvt_insert(&pipeline->dynamic_tree, i, &world_AABB);
			}

			/*L_new = L_old + Force*delta */
			vec3_scale(force, pipeline->gravity, b->mass);
			vec3_translate_scaled(b->linear_momentum, force, delta);
		}
	}
}

struct physics_output physics_output_cleared(void)
{
	struct physics_output phy_out = { 0 };
	return phy_out;
}

struct physics_output rbp_simulate_frame(struct arena *mem_frame, struct rbp *pipeline, const f32 delta)
{
	/*
	 * (1) Apply forces 
	 * (2) get derivatives
	 * (3) integrate
	 * (4) collision step
	 * (5) constraint step
	 */
	struct physics_output phy_out = { 0 };
	
	/* Apply forces, get derivatives and integrate */
	/* super simpled euler integration */
	rbp_internal_integrate(mem_frame, pipeline, delta);

	i32 *overlaps = (i32 *) mem_frame->stack_ptr;
	i32 overlap_pairs_count = internal_push_proxy_overlaps(mem_frame, pipeline);
	phy_out.collisions = internal_push_collisions(mem_frame, pipeline, overlaps, overlap_pairs_count);

	return phy_out;
}
