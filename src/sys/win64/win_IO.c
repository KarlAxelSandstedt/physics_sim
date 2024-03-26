#include <stdio.h>
#include "win_public.h"
#include "win_local.h"

struct mg_buffer win_file_raw_dump(struct arena *mem, const char *pathname)
{
	fprintf(stderr, "%s - Not implemented", __func__);
	assert(0);
}

u64 win_file_read(struct mg_buffer *buf, const char *pathname)
{
	fprintf(stderr, "%s - Not implemented", __func__);
	assert(0);
}

struct mg_buffer win_file_init_buf(struct arena *mem, const char *pathname)
{
	fprintf(stderr, "%s - Not implemented", __func__);
	assert(0);
}
