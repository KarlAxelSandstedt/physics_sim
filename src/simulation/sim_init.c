#include <string.h>

#include "sim_public.h"
#include "sim_local.h"

struct entity *entities_init_default(struct arena *mem, const u64 count)
{
	struct entity *entities = arena_push(mem, NULL, count * sizeof(struct entity));
	
	for (u64 i = 0; i < count; ++i)
	{
		entities[i].id = i;
	}

	return entities;
}

struct visual *visuals_init_default(struct arena *mem, const u64 count)
{
	struct visual *visuals = arena_push(mem, NULL, count * sizeof(struct visual));
	
	const vec4 default_color = { 0.0f, 0.3f, 0.8f, 1.0f };
	for (u64 i = 0; i < count; ++i)
	{
		vec4_copy(visuals[i].color, default_color);
	}

	return visuals;
}

struct simulation *sim_init(struct arena *mem, void (*simulation_method)(struct simulation *, const f64), const u64 seed)
{
	struct simulation *sim; 
	struct simulation cpy = 
	{
		.entity_count = 0,
		.running = 1,
		.speed_scale = 1.0f,
		.time = 0.0,
		.seed = seed,
		.simulation_method = simulation_method,
		.mem_persistent = NULL,
		.mem_frame = NULL,
		.mem_tmp = { 0 },
	};

	if (mem)
	{
		sim = arena_push(mem, &cpy, sizeof(cpy));
	}
	else
	{
		assert(0 && "Should enforce arena usage?");
		sim = malloc(sizeof(cpy));
		memcpy(sim, &cpy, sizeof(cpy));
	}

	return sim;
}

void sim_cleanup(struct simulation *sim)
{

}
