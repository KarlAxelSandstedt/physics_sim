#include "system_public.h"
#include "system_local.h"

#if __OS__ == __LINUX__
#include "unix_public.h"
#elif __OS__ == __WIN64__
#include "win_public.h"
#endif

struct mg_buffer file_raw_dump(struct arena *mem, const char *pathname)
{
	struct mg_buffer buf;

#if __OS__ == __LINUX__
	buf = unix_file_raw_dump(mem, pathname);
#elif __OS__ == __WIN64__
	buf = win_file_raw_dump(mem, pathname);
#endif

	return buf;
}

u64 file_read(struct mg_buffer *buf, const char *pathname)
{
#if __OS__ == __LINUX__
	return unix_file_read(buf, pathname);
#elif __OS__ == __WIN64__
	return win_file_read(buf, pathname);
#endif
}

struct mg_buffer file_init_buf(struct arena *mem, const char *pathname)
{
	struct mg_buffer buf;

#if __OS__ == __LINUX__
	buf = unix_file_init_buf(mem, pathname);
#elif __OS__ == __WIN64__
	buf = win_file_init_buf(mem, pathname);
#endif

	return buf;
}
