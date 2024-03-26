#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <pthread.h>

#define UNIFORM_SIZE 256

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <unistd.h>

#include "mg_mempool.h"
#include "timer.h"
#include "profiler.h"
#include "mg_string.h"
#include "mg_font.h"
#include "unix_public.h"
#include "mgl_primitives.h"
#include "geometry.h"
#include "dbvt.h"
#include "rigid_body_pipeline.h"
#include "system_public.h"
#include "widget.h"
#include "sim_public.h"
#include "r_public.h"

/* (1) setup a simulator, which should enable us to easily switch simulation scenarios, and create the first collision simulation, with 1 big, 3 small geometric shapes. */

/**
 * ---- Second pass: system abstraction layer (Windows) ----
 * (O) Open window
 * (O) initiate opengl
 * () Handle window events according to abstraction
 * (O) Handle keyboard input
 * () Handle mouse input
 *
 * ---- Third pass (Initial physics engine):  ----
 * (O) GJK accurate version
 * (O) EPA 
 * (O) Rigid body static details 
 * 	(Convex hull, faces, edges, verts)
 * 	(Mass, center of mass, Local Intertia Tensor)
 * () Rigid body dynamic details 
 * 	(position, Linear momentum, rotation (quaternion), Angular momentum)
 * () Mirtich's Algorithm for mass, center of mass and Inertia Tensor.
 * () simulation bone structure 
 * 	() integrate timestep
 * 	() check collisions
 * 	() backtrack collisions
 * 	() generate collision forces
 * 	() redo timestep integration for remaining interval, go to check collisions
 *
 * ---- Forth pass: (debugging of the physics engine)  ----
 * () infinite precision FP-accuracy library, can we make this into a cool assert lib?
 * () Goldberg accuracy
 * () Logging / Replay collision Library
 * () Collision testing Library
 *
 * ---- Fifth pass: gl configuration, Cool visuals ----
 * () gl_config - setup vao, vbo, ebo state, so we can see what is bound where, resources usage....
 * () gl_config blend shit (are we blending?, how are we blending?)
 * () Handle depth testing, CCW/CW shit, set gl_state properly
 *
 * ---- Sixth pass: (OS memory allocation abstraction (no CRT, memory debugging/usage...)) ----
 * () Setup threading support layer
 * () - TODO 
 * 
 * ---- Sixth pass: Simulation layer - Setup two simulations, 10000 convex shapes and 3 small shapes vs 1 large shape ----
 * () - TODO
 */
int main(int argc, char *argv[])
{
	/* init global memory (full program lifetime memory) */
	const u64 memsize = 1024*1024*1024;
	struct arena global_arena = arena_alloc(memsize);

	/* inititate system resources, threads, timers, ... */
	system_resources_init(&global_arena);

	/* init graphics_context */
	char *title = "MossyPhysics";
	const vec2u32 win_position = { 200, 200 };
	const vec2u32 win_size = { 1280, 720 };
	const u32 border_size = 0;
	struct graphics_context *gc = graphics_context_init(&global_arena, title, win_position, win_size, border_size);
	//cursor_grab(&gc->win);

	gl_config_log(&gc->gl, stdout);
	
	/* init ui context */
	struct ui_state *ui = ui_context_init(&global_arena, 128);

	/* init simulation context */
	//struct simulation *sim = sim_init(&global_arena, &convex_volume_intersection_simulation, 5192858L);
	struct simulation *sim = sim_init(&global_arena, &gravity_simulation, 5192858L);

	struct render_state *re = r_init(&global_arena, gc);

	f64 old_time = time_rdtsc_in_seconds();
	f64 delta;	
	
	while (sim->running)
	{
		delta = time_rdtsc_in_seconds() - old_time;

		/* update stuff, such as unix XWarpPointer shit, and emit events, if necessary */
		gc_update(gc); 

		/* process any system IO events, window events, and check which subsystem they should be under */
	     	sim_process_system_events(sim, ui, gc);
	
	     	/* run simulation step */
	     	sim_main(sim, delta);
	     		
	     	/* simulation_draw -> ui_draw */
	     	r_main(re, gc, sim, ui, delta); 
		
		old_time += delta;
	}
	
	sim_cleanup(sim);
	graphics_context_destroy(gc);
	system_resources_cleanup();
	arena_free(&global_arena);
	
	return 0;
}
