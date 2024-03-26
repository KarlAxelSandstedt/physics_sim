#ifndef __MG_ARENA_H__
#define	__MG_ARENA_H__

#include <stdlib.h>
#include <stdint.h>

#include "mg_common.h"
#include "system_common.h"

#define DEFAULT_POOL_64K_GRANULARITY 64
#define DEFAULT_POOL_16M_GRANULARITY 1

#define BLOCK_SIZE_64K  65536
#define BLOCK_SIZE_16M  16777216

#define MEMORY_ALIGNMENT 16

/*
 * struct mg_mempool: A Memory Pool that contains lists of memory aligned (MEMORY_ALIGNMENT) blocks, which can be requested through the api.
 * 
 * 
 * 
 * 
 */

struct block_header {
	void* next;
};

struct block_64K {
	u8 mem[BLOCK_SIZE_64K];
	struct block_header header;
};

struct block_16M {
	u8 mem[BLOCK_SIZE_16M];
	struct block_header header;
};

struct mg_mempool {
	u64 num_blocks_64K;
	u64 num_blocks_16M;
	u64 in_use_block_64K;
	u64 in_use_block_16M;
	u64 granularity_64K;
	u64 granularity_16M;
	struct block_64K *pool_64K;
	struct block_16M* pool_16M;
};

extern struct mg_mempool* g_mempool;

struct mg_mempool*	mg_mempool_new(const u64 granularity_64K, const u64 granularity_16M);
void			mg_mempool_free_resources(struct mg_mempool *pool);
void			mg_mempool_free_64K(struct mg_mempool* pool, struct block_64K* block); /* release all blocks in a chain back to the pool */
void			mg_mempool_free_16M(struct mg_mempool* pool, struct block_16M* block); /* release all blocks in a chain back to the pool */
struct block_64K*	mg_mempool_borrow_block_64K(struct mg_mempool* pool);
struct block_16M*	mg_mempool_borrow_block_16M(struct mg_mempool* pool);

void			mg_mempool_internal_allocate_block_64K(struct mg_mempool* pool);
void			mg_mempool_internal_allocate_block_16M(struct mg_mempool* pool);



/**
 * struct mg_memstack: Stack (interface) to mempool blocks, no allocations!
 * 
 * Assumptions: stack_ptr should be MEMORY_ALIGNMENT aligned, with the given memsize be a multiple of MEMORY_ALIGNMENT. This ensures that future vector instrinsics will work.
 * 
 * 
 * 
 */
struct arena {
	u8* stack_ptr;
	u64 mem_size;
	u64 mem_left;
};

struct arena_collection
{
	struct arena *arenas;
	u64 arena_count;
};

struct arena	arena_alloc(const u64 mem_size);
struct arena	arena_block_64K(struct block_64K* block, const u64 start_offset, const u64 mem_size);
struct arena	arena_block_16M(struct block_16M* block, const u64 start_offset, const u64 mem_size);
void	arena_align16(struct arena *mem); /* push pad bytes until 16 byte alignment */ 
void *	arena_push(struct arena *stack, const void* data, const u64 mem_size);						/* If data == NULL, only allocation occurs. Returns ptr to allocated address. (NULL == failure)  */
void *	arena_push_aligned(struct arena *stack, const void* aligned_data, const u64 mem_size);		/* Returns ptr to allocated address, asserts data to be block aligned. (NULL == failure)  */
void *	arena_push_packed(struct arena *stack, const void* data, const u64 mem_size); /* Pack buffer, no alignment checks */
void		arena_pop(struct arena *stack, const u64 mem_to_pop);
void 		arena_pop_packed(struct arena *stack, const u64 mem_to_pop);
void		arena_flush(struct arena* stack);	/* flush contents, reset stack to start of stack */
void		arena_free(struct arena *ar);

#endif
