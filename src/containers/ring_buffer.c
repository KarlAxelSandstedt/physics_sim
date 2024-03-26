#include "ring_buffer.h"
#include "mg_buffer.h"

#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include "mg_string.h"

/**
 * virtual memory wrapped ring buffer.
 */
struct ring_buffer
{
	u64 mem_total;
	u64 mem_left;
	u64 offset;	/* offset to write from base pointer buf */
	void *buf;	
};

const char shm_suffix[6] = "_Rbuf";

struct ring_buffer *ring_buffer_alloc(const u64 mem_hint)
{
	if (mem_hint == 0)
	{
		return NULL;
	}

	static u64 id = 0;
	char shm_str[39]; /* posix IPC shared memory id */
	shm_str[0] = '\\';
	mg_string shm_id = mg_string_from_uint(id++, shm_str + 1, 32);	

	struct ring_buffer *r_buf = malloc(sizeof(struct ring_buffer));
	memcpy(shm_str + 1 + shm_id.len, shm_suffix, sizeof(shm_suffix));

	/* (1) open shared fd with 0 mem */
	i32 shm_fd = shm_open(shm_str, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (shm_fd == -1)
	{
		fprintf(stderr, "Error %s:%d - %s\n", __FILE__, __LINE__, strerror(errno));
		free(r_buf);
		return NULL;
	}

	/* (2) we got the shared fd, so me way close the shared object again. All of this can apparently be done
	 * using ordinary open(), but that has implications for how the OS handles things, so we go for the
	 * "simpler and faster" using IPC. */
	shm_unlink(shm_str);

	const u64 page_size = (u64) getpagesize();
	const u64 mod = mem_hint % page_size;
	if (mod == 0)
	{
		r_buf->mem_total = mem_hint;
	}
	else
	{
		r_buf->mem_total = mem_hint + page_size - mod;
	}
	r_buf->mem_left = r_buf->mem_total;
	r_buf->offset = 0;

	/* (3) allocate memory to shared fd */
	if (ftruncate(shm_fd, r_buf->mem_total) == -1)
	{
		fprintf(stderr, "Error %s:%d - %s\n", __FILE__, __LINE__, strerror(errno));
		close(shm_fd);
		free(r_buf);
		return NULL;
	}

	void *alias;
	/* (4) map contiguous virtual memory to fd 2x times */
	for (;;)
	{
		/* Get any virtual memory the kernel sees fit */
		if ((alias = mmap(NULL, r_buf->mem_total,
					       	PROT_READ | PROT_WRITE,
					       	MAP_SHARED,
					       	shm_fd, 0)) == MAP_FAILED)
		{
			fprintf(stderr, "Error %s:%d - %s\n", __FILE__, __LINE__, strerror(errno));
			close(shm_fd);
			free(r_buf);
			return NULL;
		}

		/* try to allocate consecutive memory, if it fails, redo loop */
		if ((r_buf->buf = mmap(alias - r_buf->mem_total, r_buf->mem_total,
				       	PROT_READ | PROT_WRITE,
				       	MAP_FIXED | MAP_SHARED,
				       	shm_fd, 0)) != alias - r_buf->mem_total)
		{
			fprintf(stderr, "WARNING %s:%d - %s\n", __FILE__, __LINE__, strerror(errno));
			if (munmap(alias, r_buf->mem_total) == -1)
			{
				fprintf(stderr, "Error %s:%d - %s\n", __FILE__, __LINE__, strerror(errno));
				close(shm_fd);
				free(r_buf);
				return NULL;
			}
			continue;
		}

		break;
	}	
	/* (5) close fd, not needed anymore, we have the memory virtually mapped now */
	close(shm_fd);

	assert(r_buf->buf <= alias && (u64) alias - (u64) r_buf->buf == r_buf->mem_total 
			&& "alias virtual memory shoul be directly after buffer memory");

	return r_buf;
}

void ring_buffer_free(struct ring_buffer *r_buf)
{
	if (munmap(r_buf->buf, 2*r_buf->mem_total) == -1)
	{
		fprintf(stderr, "Error %s:%d - %s\n", __FILE__, __LINE__, strerror(errno));
	}
	free(r_buf);
}

struct mg_buffer ring_buffer_push(struct ring_buffer *r_buf, const u64 len)
{
	struct mg_buffer buf;
	if (len == 0 || r_buf->mem_left < len)
	{
		buf.data = NULL;
		buf.size = 0;
	}
	else
	{
		buf.data = r_buf->buf + r_buf->offset;
		buf.size = len;
		r_buf->mem_left -= len;
		r_buf->offset = (r_buf->offset + len) % r_buf->mem_total;
	}

	return buf;
}

void ring_buffer_pop(struct ring_buffer *r_buf, const u64 len)
{
	if (len <= r_buf->mem_total - r_buf->mem_left)
	{
		r_buf->mem_left += len;
	}
	else
	{
		r_buf->mem_left = r_buf->mem_total;
	}
}

#endif

