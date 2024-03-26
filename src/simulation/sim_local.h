#ifndef __SIMULATION_LOCAL_H__
#define __SIMULATION_LOCAL_H__

#include "sim_public.h"

/****************************** sim_init ******************************/

struct entity *entities_init_default(struct arena *mem, const u64 count);
struct visual *visuals_init_default(struct arena *mem, const u64 count);

/****************************** sim_main ******************************/

/****************************** sim_event ******************************/

/****************************** sim_entity ******************************/

void entity_construct_random_at_origin(struct arena *mem, struct simulation *sim, const u64 index, const f32 min_radius, const f32 max_radius, const u32 min_v_count, const u32 max_v_count, struct arena_collection *mem_tmp);
void entity_construct_random(struct arena *mem, struct simulation *sim, const u64 index, const f32 min_radius, const f32 max_radius, const u32 min_v_count, const u32 max_v_count, struct arena_collection *mem_tmp, const vec3 position);

#endif
