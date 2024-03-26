#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xmmintrin.h>
#include <emmintrin.h>

#include "mg_common.h"
#include "system_common.h"
#include "mg_mempool.h"

void	mg_memcpy_aligned(uint8_t *dst, const uint8_t *src, const size_t mem_size);	/* Assumes BOTH dst and src being memory aligned. mem_size WILL get padded if not multiple of MEMORY_ALIGNMENT */

struct mg_mempool* g_mempool;

struct mg_mempool* mg_mempool_new(const u64 granularity_64K, const u64 granularity_16M)
{
	assert(granularity_64K > 0);

	struct mg_mempool* pool = malloc(sizeof(struct mg_mempool));
	pool->granularity_64K = granularity_64K;
	pool->num_blocks_64K = granularity_64K;
	pool->in_use_block_64K = 0;
	mg_mempool_internal_allocate_block_64K(pool);

	pool->granularity_16M = granularity_16M;
	pool->num_blocks_16M = granularity_16M;
	pool->in_use_block_16M = 0;
	mg_mempool_internal_allocate_block_16M(pool);

	return pool;
}

void mg_mempool_free_resources(struct mg_mempool* pool)
{
	assert(pool->in_use_block_16M == 0 && pool->in_use_block_64K == 0 && "Blocks still in use by systems at mg_mempool_free_resources!\n");

	struct block_64K* block64K = pool->pool_64K;
	while (block64K) {
		struct block_64K* next = block64K->header.next;
		free(block64K);
		block64K = next;
	}

	struct block_16M* block16M = pool->pool_16M;
	while (block16M) {
		struct block_16M* next = block16M->header.next;
		free(block16M);
		block16M = next;
	}

	free(pool);
}

void mg_mempool_free_64K(struct mg_mempool* pool, struct block_64K* block)
{
	u64 freed_blocks = 1;
	struct block_64K* last = block;
	for (; last->header.next; last = last->header.next, ++freed_blocks);
	last->header.next = pool->pool_64K;
	pool->pool_64K = block;
	pool->in_use_block_64K -= freed_blocks;
}


void mg_mempool_free_16M(struct mg_mempool* pool, struct block_16M* block)
{
	u64 freed_blocks = 1;
	struct block_16M* last = block;
	for (; last->header.next; last = last->header.next, ++freed_blocks);
	last->header.next = pool->pool_16M;
	pool->pool_16M = block;
	pool->in_use_block_16M -= freed_blocks;
}

struct block_64K* mg_mempool_borrow_block_64K(struct mg_mempool* pool)
{
	if (pool->num_blocks_64K == 0) { mg_mempool_internal_allocate_block_64K(pool); }

	pool->in_use_block_64K += 1;
	pool->num_blocks_64K -= 1;
	struct block_64K* borrowed = pool->pool_64K;
	pool->pool_64K = pool->pool_64K->header.next;
	borrowed->header.next = NULL;
	return borrowed;
}

struct block_16M* mg_mempool_borrow_block_16M(struct mg_mempool* pool)
{
	if (pool->num_blocks_16M == 0) { mg_mempool_internal_allocate_block_16M(pool); }

	pool->in_use_block_16M += 1;
	pool->num_blocks_16M -= 1;
	struct block_16M* borrowed = pool->pool_16M;
	pool->pool_16M = pool->pool_16M->header.next;
	borrowed->header.next = NULL;
	return borrowed;
}

void mg_mempool_internal_allocate_block_64K(struct mg_mempool* pool)
{
	struct block_64K* head;
	ALIGN_ALLOC(&head, sizeof(struct block_64K), MEMORY_ALIGNMENT);

	assert(((uintptr_t)head) % MEMORY_ALIGNMENT == 0);
	assert(((uintptr_t)&head->mem) % MEMORY_ALIGNMENT == 0);
	assert(((uintptr_t)&head->header) % MEMORY_ALIGNMENT == 0);

	struct block_64K* prev_block = head;
	for (u64 block = 1; block < pool->granularity_64K; ++block) {
		struct block_64K* new_block;

		ALIGN_ALLOC(&new_block, sizeof(struct mg_mempool), MEMORY_ALIGNMENT);

		assert(((uintptr_t)new_block) % MEMORY_ALIGNMENT == 0);
		assert(((uintptr_t)&new_block->mem) % MEMORY_ALIGNMENT == 0);
		assert(((uintptr_t)&new_block->header) % MEMORY_ALIGNMENT == 0);

		prev_block->header.next = new_block;
		prev_block = new_block;
	}
	prev_block->header.next = pool->pool_64K;
	pool->pool_64K = head;
}

void mg_mempool_internal_allocate_block_16M(struct mg_mempool* pool)
{
	struct block_16M* head;
	ALIGN_ALLOC(&head, sizeof(struct block_16M), MEMORY_ALIGNMENT);
	head->header.next = NULL;

	assert(((uintptr_t)head) % MEMORY_ALIGNMENT == 0);
	assert(((uintptr_t)&head->mem) % MEMORY_ALIGNMENT == 0);
	assert(((uintptr_t)&head->header) % MEMORY_ALIGNMENT == 0);

	struct block_16M* prev_block = head;
	for (u64 block = 1; block < pool->granularity_16M; ++block) {
		struct block_16M* new_block;

		ALIGN_ALLOC(&new_block, sizeof(struct block_16M), MEMORY_ALIGNMENT);

		assert(((uintptr_t)new_block) % MEMORY_ALIGNMENT == 0);
		assert(((uintptr_t)&new_block->mem) % MEMORY_ALIGNMENT == 0);
		assert(((uintptr_t)&new_block->header) % MEMORY_ALIGNMENT == 0);

		prev_block->header.next = new_block;
		prev_block = new_block;
	}
	prev_block->header.next = pool->pool_16M;
	pool->pool_16M = head;
}

struct arena	arena_block_64K(struct block_64K* block, const size_t start_offset, const size_t mem_size)
{
	assert(start_offset % MEMORY_ALIGNMENT == 0 && mem_size % MEMORY_ALIGNMENT == 0 && start_offset + mem_size <= BLOCK_SIZE_64K);

	struct arena stack = { .stack_ptr = &block->mem[start_offset], .mem_left = mem_size, .mem_size = mem_size};
	return stack;

}

struct arena arena_block_16M(struct block_16M* block, const size_t start_offset, const size_t mem_size)
{
	assert(start_offset % MEMORY_ALIGNMENT == 0 && mem_size % MEMORY_ALIGNMENT == 0 && start_offset + mem_size <= BLOCK_SIZE_16M);

	struct arena stack = { .stack_ptr = &block->mem[start_offset], .mem_left = mem_size, .mem_size = mem_size};
	return stack;
}

void *arena_push(struct arena *stack, const void* data, const size_t mem_size)
{
	assert(mem_size > 0);

	size_t mem_to_push = mem_size % MEMORY_ALIGNMENT;
	if (mem_to_push) { mem_to_push = MEMORY_ALIGNMENT - mem_to_push; }
	mem_to_push += mem_size;
	assert(mem_to_push % MEMORY_ALIGNMENT == 0);

	u8* alloc_addr = NULL;
	if (stack->mem_left >= mem_to_push) {
		if (data) {
			memcpy(stack->stack_ptr, data, mem_size);
		}
		alloc_addr = stack->stack_ptr;
		stack->mem_left -= mem_to_push;
		stack->stack_ptr += mem_to_push;
	} else {
		assert(0 && "OUT OF MEMORY, HANDLE THIS!");
	}
	
	return alloc_addr;
}
void *arena_push_packed(struct arena *stack, const void* data, const size_t mem_size)
{
	assert(mem_size > 0);

	u8* alloc_addr = NULL;
	if (stack->mem_left >= mem_size) {
		if (data) {
			memcpy(stack->stack_ptr, data, mem_size);
		}
		alloc_addr = stack->stack_ptr;
		stack->mem_left -= mem_size;
		stack->stack_ptr += mem_size;
	} else {
		assert(0 && "OUT OF MEMORY, HANDLE THIS!");
	}
	
	return alloc_addr;
}

void *arena_push_aligned(struct arena *stack, const void* aligned_data, const size_t mem_size)
{
	assert(mem_size > 0 && (uintptr_t) aligned_data % MEMORY_ALIGNMENT == 0);

	size_t mem_to_push = mem_size % MEMORY_ALIGNMENT;
	if (mem_to_push) { mem_to_push = MEMORY_ALIGNMENT - mem_to_push; }
	mem_to_push += mem_size;
	assert(mem_to_push % MEMORY_ALIGNMENT == 0);

	u8* alloc_addr = NULL;
	if (stack->mem_left >= mem_size) {
		mg_memcpy_aligned(stack->stack_ptr, aligned_data, mem_to_push);
		alloc_addr = stack->stack_ptr;
		stack->mem_left -= mem_to_push;
		stack->stack_ptr += mem_to_push;
	} else {
		assert(0 && "OUT OF MEMORY, HANDLE THIS!");
	}

	return alloc_addr;
}

void arena_pop(struct arena *stack, const size_t mem_to_pop)
{
	assert(stack->mem_size - stack->mem_left >= mem_to_pop);
	size_t extra = mem_to_pop % MEMORY_ALIGNMENT;

	if (extra) { extra = MEMORY_ALIGNMENT - extra; }
	assert((mem_to_pop + extra) % MEMORY_ALIGNMENT == 0);

	stack->stack_ptr -= mem_to_pop + extra;
	stack->mem_left += mem_to_pop + extra;
}

void arena_pop_packed(struct arena *stack, const size_t mem_to_pop)
{
	assert(stack->mem_size - stack->mem_left >= mem_to_pop);

	stack->stack_ptr -= mem_to_pop;
	stack->mem_left += mem_to_pop;
}

void arena_flush(struct arena* stack)
{
	if (stack)
	{
		stack->stack_ptr -= stack->mem_size - stack->mem_left;
		stack->mem_left = stack->mem_size;
	}
}

void mg_memcpy_aligned(uint8_t *dst, const uint8_t *src, const size_t mem_size)
{
	assert((uintptr_t) dst % MEMORY_ALIGNMENT == 0 && (uintptr_t) src % MEMORY_ALIGNMENT == 0 && mem_size > 0);

#ifdef __SSE_EXT__
	const uint64_t num_blocks16B = mem_size / 16;
	__m128i block;
	__m128i* src128 = (__m128i*) src;
	__m128i* dst128 = (__m128i*) dst;
	for (uint64_t i = 0; i < num_blocks16B; ++i) {
		block = _mm_load_si128(src128 + i);
		_mm_store_si128(dst128 + i, block);
	}
#else
	memcpy(dst, src, mem_size);
#endif
}


struct arena arena_alloc(const u64 mem_size)
{
	struct arena ar =
	{
		.mem_size = mem_size,
		.mem_left = mem_size,
		.stack_ptr = malloc(mem_size),
	};

	return ar;
}

void arena_free(struct arena *ar)
{
	ar->stack_ptr -= ar->mem_size - ar->mem_left;
	free(ar->stack_ptr);

	ar->stack_ptr = NULL;
	ar->mem_size = 0;
	ar->mem_left = 0;
}

void arena_align16(struct arena *mem)
{
	u64 pad = 16 - (u64) mem->stack_ptr % 16;
	if (pad != 16)
	{
		if (pad <= mem->mem_left)
		{
			mem->mem_left -= pad;
			mem->stack_ptr += pad;
		}
		else
		{
			assert(0 && "Out of memory - handle this!\n");
		}
	}

	assert((u64) mem->stack_ptr % 16 == 0);
}
