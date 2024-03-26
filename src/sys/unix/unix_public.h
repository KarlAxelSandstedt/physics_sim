#ifndef __UNIX_PUBLIC_H__
#define __UNIX_PUBLIC_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "mg_buffer.h"

void print_error(FILE *file, i32 status, char *err_file, i32 err_line);

struct mg_buffer unix_file_raw_dump(struct arena *mem, const char *pathname);
u64 unix_file_read(struct mg_buffer *buf, const char *pathname);
struct mg_buffer unix_file_init_buf(struct arena *mem, const char *pathname);

#endif
