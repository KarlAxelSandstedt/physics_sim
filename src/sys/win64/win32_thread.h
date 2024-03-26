#ifndef __WIN32_THREAD_H__
#define __WIN32_THREAD_H__

#define THREAD_MAX		32
#define THREAD_STACKSIZE	65536

#include <stdint.h>
#include <windows.h>

struct win32_thread {
	int32_t id;
	HANDLE handle;
};

struct win32_thread win32_thread_new(const size_t stack_size, uint32_t (*thread_routine)(void *), void *data);
win32_thread_return(const uint32_t exitcode);
win32_thread_wait_for_single();
//win32_thread_wait_for_multiple();
//win32_barrier();
//win32_mutex();

#endif
