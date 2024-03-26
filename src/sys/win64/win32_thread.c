#include "win32_thread.h"

struct win32_thread win32_thread_new(const size_t stack_size, uint32_t (*thread_routine)(void *), void *data)
{
	struct win32_thread thread;
	thread.handle = CreateThread(0, stack_size, thread_routine, data, 0, &thread.id);
	return thread;
}

win32_thread_return(const uint32_t exitcode)
{
	ExitThread(exitcode);
}
