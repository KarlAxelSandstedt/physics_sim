#ifndef __MG_PROFILER_H__
#define __MG_PROFILER_H__

#include "mg_common.h"
#ifdef MG_PROFILE
#include <stdint.h>
#include <stdio.h>
#include "mg_string.h"
#include "mg_timer.h"
#include "mg_mempool.h"
#include "mg_buffer.h"
#include "hash_index.h"
#include "dynamic_array.h"

/************************** Profiling **************************/

#define MAX_PROFILES (1 << 12)

struct profile
{
	char *label;
	u64 time;		
	u64 time_including_children;
	u64 bytes_processed;
	u64 enter_count;
	u32 id;
	i32 parent;
};

struct profile_block
{
	u64 previous_time_including_children;
	u64 time_start;
	u64 time_in_child_blocks;
	u64 bytes_processed;
	i32 index;
};

extern struct hash_index *g_profile_hash;
extern struct d_array *g_profile_array;
extern struct profile_block *g_profile_blocks;

/* Should not be used standalone, use macros instead */ 
i32 profile_stack_peek(void);
void profile_stack_push(const i32 index, const u64 time_start, const u64 bytes_to_process);
void profile_stack_pop(const u64 bytes_processed);
void profile_table_print(FILE *stream);
i32 profile_new(const char *label);
void profile_init(void);

/* TODO: Function collapse on jumps... */
/* TODO: NOT THREAD-SAFE! */

#define PROFILE_INIT						\
{								\
	profile_init();						\
}								

#define PROFILE_FUNCTION PROFILE_BLOCK(__func__, 0)
#define PROFILE_BANDWIDTH(label, bytes_to_process) PROFILE_BLOCK(label, bytes_to_process)

#define PROFILE_BLOCK(label, bytes_to_process)					\
{										\
	static i32 __profile_id = -1;						\
	if (__profile_id == -1)							\
	{									\
		__profile_id = profile_new(label);				\
	}									\
	profile_stack_push(__profile_id, time_rdtsc(), bytes_to_process);	\
}

#define PROFILE_END	PROFILE_BANDWIDTH_END(0)				
#define PROFILE_BANDWIDTH_END(bytes_processed)					\
{										\
	profile_stack_pop(bytes_processed);					\
}										

#define PROFILE_PRINT(stream) profile_table_print(stream);

/********************************** Repetition Testing ************************************/

enum tester_state
{
	RT_UNINITIALIZED = 0,
	RT_TESTING,
	RT_COMPLETED,
	RT_ERROR,
};

struct repetition_tester
{
	u64 bytes_to_process;
	u64 try_for_time;
	u64 cpu_freq;
	u64 time_at_start;

	u64 test_count;
	u64 total_time;
	u64 max_iteration_time;
	u64 min_iteration_time;
	
	u64 page_faults_in_current_test;
	u64 page_faults_min_time;
	u64 page_faults_max_time; /* page fault count on max count */
	u64 page_faults;

	u64 branch_misses_in_current_test;
	u64 branch_misses_min_time;
	u64 branch_misses_max_time;
	u64 branch_misses;

	u64 frontend_stalled_cycles_in_current_test;
	u64 frontend_stalled_cycles_min_time;
	u64 frontend_stalled_cycles_max_time;
	u64 frontend_stalled_cycles;

	u64 backend_stalled_cycles_in_current_test;
	u64 backend_stalled_cycles_min_time;
	u64 backend_stalled_cycles_max_time;
	u64 backend_stalled_cycles;

	u64 cycles_in_current_test;
	u64 cycles_min_time;
	u64 cycles_max_time;
	u64 cycles;

	u64 time;
	u64 bytes;
	u64 time_in_current_test;
	u64 bytes_in_current_test;
	enum tester_state state;
	u32 enter_count;
	u32 exit_count;
	u32 print : 1;
#ifdef __linux__

#define NUM_EVENTS 5
#define PAGE_FAULT_SAMPLING_PERIOD 1		/* counter period for new update */
#define BRANCH_MISSES_SAMPLING_PERIOD 1000
	u64 event_count;
	u64 pf_id; /* page faults, event group leader */
	u64 bm_id; /* branch_misses */
	u64 fnt_id; /* frontend stall cycles */
	u64 bck_id; /* backend stall cycles */
	u64 cyc_id;  /* non-cpu-frequency-scaled cycles */
	i32 pf_fd; 
	i32 bm_fd;  
	i32 fnt_fd;
	i32 bck_fd;
	i32 cyc_fd;
#endif
};

i32 rt_is_testing(struct repetition_tester *tester);
void rt_wave(struct repetition_tester *tester, const u64 bytes_to_process, const u64 cpu_freq, const u64 try_for_time, const u32 print);
void rt_begin_time(struct repetition_tester *tester);
void rt_end_time(struct repetition_tester *tester);
void rt_print_statistics(const struct repetition_tester *tester, FILE *file);

#else
#define PROFILE_INIT
#define PROFILE_FUNCTION
#define PROFILE_BANDWIDTH(label, bytes)
#define PROFILE_BLOCK(label, bytes)
#define PROFILE_END
#define PROFILE_BANDWIDTH_END(bytes_processed)					
#define PROFILE_PRINT(stream)
#endif

#endif
