#ifndef __RIGID_BODY_PIPELINE_H__
#define __RIGID_BODY_PIPELINE_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "dbvt.h"
#include "rigid_body.h"

struct physics_output
{
	i32 *collisions;
	vec3ptr closest_point_pairs;
	u32 point_pairs_count;
};
/*
 * Rigid Body Pipeline
 */
struct rbp
{
	i32 size;
	i32 count;
	struct rigid_body *bodies;
	struct dbvt dynamic_tree;

	vec3 gravity;	/* gravity constant */
};

struct 	rbp rbp_new(struct arena *mem, const i32 size);
void rbp_add(struct rbp *pipeline, const i32 index, struct rigid_body *body, u32 dynamic);
void 	rbp_remove(struct rbp *pipeline, const i32 index);
void 	rbp_construct_random(struct arena *mem, struct rbp *pipeline, const u64 index, const f32 min_radius, const f32 max_radius, const u32 min_v_count, const u32 max_v_count, struct arena_collection *mem_tmp, const vec3 pos);

void	rbp_push_dbvt(struct drawbuffer *buf, struct rbp *pipeline, const vec4 color);
void	rbp_push_proxies(struct drawbuffer *buf, const struct rbp *pipeline, const vec4 color);
void 	rbp_push_convex_hulls(const struct rbp *pipeline, struct drawbuffer *buf, const vec4 color, struct arena *mem_1, struct arena *mem_2, struct arena *mem_3, struct arena *mem_4, struct arena *mem_5, i32 i_count[], i32 i_offset[]);
void	rbp_push_positions(void *buf, const struct rbp *pipeline);
vec3ptr rbp_push_closest_points_between_bodies(struct arena *mem_frame, struct rbp *pipeline, u32 *pair_count);

i32 *	rbp_simulate(struct arena *mem_frame, struct rbp *pipeline, const f32 delta);


struct physics_output physics_output_cleared(void);
struct physics_output rbp_simulate_frame(struct arena *mem_frame, struct rbp *pipeline, const f32 delta);

#endif
