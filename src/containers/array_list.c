#include <string.h>

#include "array_list.h"

struct array_list *array_list_new(struct arena *mem, const u64 length, const u64 data_size)
{
	struct array_list *list;
	struct arena record;

	u64 slot_size = data_size;
	if (data_size < sizeof(void *))
	{
		slot_size = sizeof(void *);		
	}

	if (mem)
	{
		record = *mem;
		list = arena_push(mem, NULL, sizeof(struct array_list));
		list->slot = arena_push(mem, NULL, length * slot_size);
	}
	else
	{
		list = malloc(sizeof(struct array_list));
		list->slot = malloc(length * slot_size);
	}

	if (list && list->slot)
	{
		list->length = length;
		list->max_count = 0;
		list->count = 0;
		list->data_size = data_size;
		list->slot_size = slot_size;
		list->free_chain = NULL;
	}
	else
	{
		if (mem)
		{
			*mem = record;
		}
		else
		{
			array_list_free(list);
		}
		
		list = NULL;
	}
	
	return list;
}

void array_list_free(struct array_list *list)
{
	free(list->slot);
	free(list);
}

void *array_list_reserve(struct array_list *list)
{
	void *addr = NULL;

	if (list->count < list->length)
	{
		if (list->free_chain)
		{
			addr = list->free_chain;
			list->free_chain = *((void **) addr);
		}
		else
		{
			addr = list->slot + list->slot_size * list->max_count;
			list->max_count += 1;
		}	
		list->count += 1;
	}

	return addr;
}

void *array_list_add(struct array_list *list, void *data)
{
	void *addr = array_list_reserve(list);

	if (addr)
	{
		memcpy(addr, data, list->data_size);
	}

	return addr;
}

void array_list_remove(struct array_list *list, void *addr)
{
	assert(addr && (void *) list->slot <= addr && addr < (void *) (list->slot + list->length * list->slot_size));
	assert((addr - (void *) list->slot) % list->slot_size == 0);
	
	*((void **) addr) = list->free_chain;
	list->free_chain = addr;
	list->count -= 1;
}

void array_list_remove_index(struct array_list *list, const u64 index)
{
	assert(index < list->length);
	void *addr = list->slot + index * list->slot_size;

	*((void **) addr) = list->free_chain;
	list->free_chain = addr;
	list->count -= 1;
}

void *array_list_address(struct array_list *list, const u64 index)
{
	assert(index < list->length);

	return list->slot + index * list->slot_size;
}

struct gen_array_list *gen_array_list_new(struct arena *mem, const u64 length, const u64 data_size)
{
	struct gen_array_list *list;
	struct arena record;

	u64 slot_size = data_size;
	if (data_size < sizeof(void *))
	{
		slot_size = sizeof(void *);		
	}

	if (mem)
	{
		record = *mem;
		list = arena_push(mem, NULL, sizeof(struct gen_array_list));
		list->slot = arena_push(mem, NULL, length * slot_size);
		list->generation = arena_push(mem, NULL, length * sizeof(u32));
	}
	else
	{
		list = malloc(sizeof(struct gen_array_list));
		list->slot = malloc(length * slot_size);
		list->generation = malloc(length * sizeof(u32));
	}

	if (list && list->slot)
	{
		list->length = length;
		list->max_count = 0;
		list->count = 0;
		list->data_size = data_size;
		list->slot_size = slot_size;
		list->free_chain = NULL;
	}
	else
	{
		if (mem)
		{
			*mem = record;
		}
		else
		{
			gen_array_list_free(list);
		}
		
		list = NULL;
	}
	
	return list;
}

void gen_array_list_free(struct gen_array_list *list)
{
	free(list->generation);
	free(list->slot);
	free(list);
}

void *gen_array_list_reserve(u64 *gen_index, struct gen_array_list *list)
{
	void *addr = NULL;

	if (list->count < list->length)
	{
		if (list->free_chain)
		{
			addr = list->free_chain;
			list->free_chain = *((void **) addr);
		}
		else
		{
			addr = list->slot + list->slot_size * list->max_count;
			list->generation[list->max_count] = 0;
			list->max_count += 1;
		}	
		const u64 index = ((u64) addr - (u64) list->slot) / list->slot_size;
		*gen_index = ((u64) list->generation[index] << 32) | index;
		list->count += 1;
	}

	return addr;
}

void *gen_array_list_add(u64 *gen_index, struct gen_array_list *list, void *data)
{
	void *addr = gen_array_list_reserve(gen_index, list);

	if (addr)
	{
		memcpy(addr, data, list->data_size);
	}

	return addr;
}

void gen_array_list_remove(struct gen_array_list *list, void *addr)
{
	assert(addr && (void *) list->slot <= addr && addr < (void *) (list->slot + list->length * list->slot_size));
	assert((addr - (void *) list->slot) % list->slot_size == 0);
	
	const u64 index = ((u64) addr - (u64) list->slot) / list->slot_size;
	list->generation[index] += 1;

	*((void **) addr) = list->free_chain;
	list->free_chain = addr;
	list->count -= 1;
}

void gen_array_list_remove_index(struct gen_array_list *list, const u64 gen_index)
{
	assert((0xffffffff & gen_index) < (u64) list->length);
	const u64 index = gen_index & 0xffffffff;
	void *addr = list->slot + index * list->slot_size;

	list->generation[index] += 1;

	*((void **) addr) = list->free_chain;
	list->free_chain = addr;
	list->count -= 1;
}

void *gen_array_list_generation_address(struct gen_array_list *list, const u64 gen_index)
{
	assert((0xffffffff & gen_index) < (u64) list->length);

	void *addr = NULL;
	const u64 generation = gen_index >> 32;
	const u64 index = 0xffffffff & gen_index;

	if (list->generation[index] == generation)
	{
		addr = list->slot + index * list->slot_size;
	}

	return addr;
}

void *gen_array_list_address(struct gen_array_list *list, const u64 gen_index)
{
	assert((0xffffffff & gen_index) < (u64) list->length);

	return list->slot + (gen_index & 0xffffffff) * list->slot_size;
}
