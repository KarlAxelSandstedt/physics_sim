#ifndef __MG_TIMER_H__
#define __MG_TIMER_H__

#include "mg_common.h"
#include "system_common.h"
#include "mg_string.h"

struct precision_timer 
{
	i64 time_start;
	i64 ticks_per_second;	/* Platform Dependent tick granularity */

	u64 rdtsc_freq;
};

void 		precision_timer_init(void);		/* Initialise Global Application Timer */
i64	 	precision_timer_get_time(void);	/* Get time since initialisation */
i64	 	precision_timer_get_ticks_per_second(void);
mg_string 	precision_timer_get_time_string(char *buf, const size_t buf_size); /* Return string formatted as [ s.%%%%%% (microseconds) */
f64		precision_timer_get_time_seconds(void);
u64 		precision_timer_get_rdtsc_frequency(void);
u64 		freq_rdtsc(void);
u64		time_rdtsc(void);
f64		time_rdtsc_in_seconds(void);
f64 		time_seconds_from_rdtsc(u64 ticks);

#endif
