#ifndef __SIMULATION_PUBLIC_H__
#define __SIMULATION_PUBLIC_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "system_common.h"
#include "system_public.h"
#include "widget.h"
#include "rigid_body_pipeline.h"

/*
 * Struct for the simulation state. The state should be easily set once, taking some
 * initial inputs:
 * 	- seed,
 * 	- simulation_function(sim, delta, seed)
 *
 * The seed is simply the initial seed from which the simulation can start its PRNG.
 * The simulation function should, using the simulation rules which it defines, act
 * upon any entities in the simulation state. On the first call, it should initialize
 * any simulation variables (Such as its pseudo-random number generator). This will
 * make it easy from the rendering system.
 *
 * TODO(Important!):
 * Creating a new simulation should be as simple as creating the simulation function,
 * and defining the simulation local behaviour and data. Everything else MUST be sim-
 * -ulation independent, i.e, the only change needed to run two different simulations
 * should be
 *
 * struct simulation sim = { .simulation_method = SIMULATION_1 };
 *  
 * and
 *
 * struct simulation sim = { .simulation_method = SIMULATION_2 };
 *
 * TODO(Important!):
 * In some simulations, we may want to specify some behaviour from specific entities,
 * such as follow these lines, or do this, or do that. For this, we would benefit
 * from expanding the callback stuff from before, and make it easier to use, therefore
 * we should put some effort in here. The goal is the make writing simulations as easy
 * as possible!
 */

struct entity
{
	u64 id; 
	i32 active;
};

struct visual
{
	vec4 color;
};

struct simulation
{
	u64 entity_count;
	struct entity *entities;
	struct visual *visuals;
	struct rbp pipeline;

	f32 speed_scale;

	struct arena *mem_persistent;	/* full simulation lifetime */
	struct arena *mem_frame;	/* frame lifetime */
	struct arena_collection mem_tmp;

	f64 time;

	void (*simulation_method)(struct simulation *, const f64);
	u64 seed;
	i32 running;

	/* TODO: Put somewhere else, in some output_of_simulation struct */
	/* TODO: For this data, write a sim_frame_reset function so to not accidentaly reuse old data */
	struct physics_output phy_out;
};

/****************************** sim_init ******************************/

struct simulation *sim_init(struct arena *mem, void (*simulation_method)(struct simulation *, const f64), const u64 seed);
void sim_cleanup(struct simulation *sim);


/****************************** sim_main ******************************/

void sim_system_events(struct simulation *sim, struct ui_state *ui, struct graphics_context *gtx);
void sim_main(struct simulation *sim, const f64 delta);

#define CVI_BODIES 4
#define CVI_LARGE_INDEX 0
#define CVI_SMALL1_INDEX 1
#define CVI_SMALL2_INDEX 2
#define CVI_SMALL3_INDEX 3
void convex_volume_intersection_simulation(struct simulation *sim, const f64 delta);

#define G_BODIES 5
#define G_LARGE_INDEX 0
#define G_SMALL1_INDEX 1
#define G_SMALL2_INDEX 2
#define G_SMALL3_INDEX 3
#define G_FLOOR_INDEX 4
void gravity_simulation(struct simulation *sim, const f64 delta);

/****************************** sim_event ******************************/

/* process system events that affect the simulation, ui or graphics context */
void sim_process_system_events(struct simulation *sim, struct ui_state *ui, struct graphics_context *gtx); 

/****************************** sim_event ******************************/

/****************************** sim_entity ******************************/

void entity_push_convex_hull(struct drawbuffer *buf, struct simulation *sim, const u64 index);


#endif
