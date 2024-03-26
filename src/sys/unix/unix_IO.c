#define _LARGEFILE64_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "unix_public.h"
#include "unix_local.h"
#include "profiler.h"

void print_error(FILE *file, i32 status, char *err_file, i32 err_line)
{
	char buf[256];
	strerror_r(status, buf, sizeof(buf));
	fprintf(file, "Error %s:%d - %s\n", err_file, err_line, buf);
}

struct mg_buffer unix_file_init_buf(struct arena *mem, const char *pathname)
{
	struct mg_buffer buf;

	const i32 fd = open(pathname, O_RDONLY);
	if (fd == -1)
	{
		/* TODO: print error to file, setup macro to get FILE, LINE, print syscall */
		fprintf(stdout, "Error at %s:%d: couldn't open file %s\n", __FILE__, __LINE__, pathname);
		buf.size = 0;
		buf.data = NULL;
		close(fd);
		return buf;
	}

	buf.size = lseek64(fd, 0, SEEK_END);
	buf.data = arena_push(mem, NULL, buf.size);
	close(fd);

	return buf;
}

u64 unix_file_read(struct mg_buffer *buf, const char *pathname)
{
	u64 bytes_left;
	i64 bytes_read_in_call;
	i32 fd;

	fd = open(pathname, O_RDONLY);
	if (fd == -1)
	{
		return 0;
	}	

	bytes_left = buf->size;
	while (0 < bytes_left)
	{
		bytes_read_in_call = read(fd, buf->data + (buf->size - bytes_left), bytes_left);
		if (bytes_read_in_call == -1)
		{
			fprintf(stdout, "Error at %s:%d: couldn't read file %s into buffer\n", __FILE__, __LINE__, pathname);
			return buf->size - bytes_left;
		}
		bytes_left -= (u64) bytes_read_in_call;	
	}

	close(fd);
	return buf->size;
}

struct mg_buffer unix_file_raw_dump(struct arena *mem, const char *pathname)
{
	struct mg_buffer buf;

	const i32 fd = open(pathname, O_RDONLY);
	if (fd == -1)
	{
		/* TODO: print error to file, setup macro to get FILE, LINE, print syscall */
		fprintf(stdout, "Error at %s:%d: couldn't open file %s\n", __FILE__, __LINE__, pathname);
		buf.size = 0;
		buf.data = NULL;
		return buf;
	}

	buf.size = lseek64(fd, 0, SEEK_END);
	lseek64(fd, 0, SEEK_SET);
	
	if (mem)
	{
		buf.data = arena_push(mem, NULL, buf.size);
	}
	else
	{
		buf.data = malloc(buf.size);
	}
	
	u64 bytes_left = buf.size;
	i64 bytes_read_in_call;
	while (0 < bytes_left)
	{
		bytes_read_in_call = read(fd, buf.data + (buf.size - bytes_left), bytes_left);
		if (bytes_read_in_call == -1)
		{
			if (mem)
			{
				arena_pop(mem, buf.size);
			}
			else
			{
				free(buf.data);
			}
			fprintf(stdout, "Error at %s:%d: couldn't read file %s into buffer\n", __FILE__, __LINE__, pathname);
			buf.size = 0;
			buf.data = NULL;

			break;
		}
		bytes_left -= (u64) bytes_read_in_call;	
	}

	close(fd);

	return buf;
}
