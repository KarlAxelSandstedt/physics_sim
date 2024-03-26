#include "timer.h"

struct precision_timer g_precision_timer;

#ifdef _WIN64
#include <windows.h>
#include <intrin.h>

void precision_timer_init(void)
{
	LARGE_INTEGER ret_val;
	QueryPerformanceCounter(&ret_val);
	g_precision_timer.time_start = ret_val.QuadPart;
	QueryPerformanceFrequency(&ret_val);
	g_precision_timer.ticks_per_second = ret_val.QuadPart;

	u64 start = __rdtsc();
	const u64 ms = 100;
	const u64 goal = g_precision_timer.time_start + g_precision_timer.ticks_per_second / (1000/ms);
	for (;;)
	{
		QueryPerformanceCounter(&ret_val);
		if (goal <= ret_val.QuadPart)
		{
			break;
		}
	}
	u64 end = __rdtsc();
	g_precision_timer.rdtsc_freq = (1000/ms) * (end-start);
}

i64 precision_timer_get_time(void)
{
	LARGE_INTEGER ret_val;
	QueryPerformanceCounter(&ret_val);
	return ret_val.QuadPart - g_precision_timer.time_start;
}

#elif __LINUX__
#include <sys/time.h>
#include <x86intrin.h>

void precision_timer_init(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	g_precision_timer.time_start = tv.tv_sec * 1000000 + tv.tv_usec;
	g_precision_timer.ticks_per_second = 1000000;	/* microseconds */

	struct timeval tv_end;
	gettimeofday(&tv_end, NULL);
	u64 start = __rdtsc();
	const u64 ms = 100;
	const u64 goal = (tv_end.tv_sec) * 1000000 + tv_end.tv_usec + 1000000 / (1000/ms);
	while ((u64) (tv_end.tv_sec * 1000000 + tv_end.tv_usec) < goal)
	{
		gettimeofday(&tv_end, NULL);
	}
	u64 end = __rdtsc();
	g_precision_timer.rdtsc_freq = (1000/ms) * (end - start);
}

int64_t precision_timer_get_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	const int64_t time = tv.tv_sec * 1000000 + tv.tv_usec;
	return time - g_precision_timer.time_start;
}

#endif

i64 precision_timer_get_ticks_per_second(void)
{
	return g_precision_timer.ticks_per_second;
}

mg_string precision_timer_get_time_string(char *buf, const size_t buf_size)
{
	const double time = (double) precision_timer_get_time() / g_precision_timer.ticks_per_second;
	return mg_string_from_double(time, 6, buf, buf_size);
}

f64 precision_timer_get_time_seconds(void)
{
	return (double) precision_timer_get_time() / g_precision_timer.ticks_per_second;
}

u64 time_rdtsc(void)
{
	return __rdtsc();
}

f64 time_rdtsc_in_seconds(void)
{
	return (f64) __rdtsc() / g_precision_timer.rdtsc_freq;
}

f64 time_seconds_from_rdtsc(u64 ticks)
{
	return (f64) ticks / g_precision_timer.rdtsc_freq;
}

u64 freq_rdtsc(void)
{
	return g_precision_timer.rdtsc_freq;
}
