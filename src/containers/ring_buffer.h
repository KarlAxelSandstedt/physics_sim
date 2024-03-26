#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "mg_common.h"

struct ring_buffer;

/* Allocated virtual memory wrapped ring buffer using mem_hint as a minimum memsize.
 * The final size depends on the page size of the underlying system.  returns valid 
 * ptr on SUCCESS, NULL on FAILURE.*/
struct ring_buffer *ring_buffer_alloc(const u64 mem_hint);	

void ring_buffer_free(struct ring_buffer *r_buf);			

/* return address to unused memory in buffer with len size, offset ring buffer. returns buffer on SUCCESS, empty buffer on FAILURE. */
struct mg_buffer ring_buffer_push(struct ring_buffer *r_buf, const u64 len);
void ring_buffer_pop(struct ring_buffer *r_buf, const u64 len); /* release len bytes in ring_buffer in fifo order. */

#endif
