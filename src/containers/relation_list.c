#include "relation_list.h"

struct relation_list relation_list_init(struct arena *mem, const i32 num_relations)
{
	struct relation_list list =
	{
		.mem = mem,
		.free_chain = -1,
		.free_chain_len = 0,
		.r = (struct relation_unit *) mem->stack_ptr,
	};

	if (num_relations > 0)
	{
		list.num_relations = num_relations;
		arena_push_packed(mem, NULL, num_relations * sizeof(struct relation_unit));
		for (i32 i = 0; i < num_relations; ++i)
	       	{ 
			list.r[i].related_to = i;
			list.r[i].next = -1;
	       	}
	}
	else
	{
		list.num_relations = 0;
	}

	return list;
}

void relation_list_free(struct relation_list *list)
{
	arena_pop_packed(list->mem, list->num_relations * sizeof(struct relation_unit));
	assert((u8 *) list->r == list->mem->stack_ptr);
}

i32 relation_list_add_relation_unit_empty(struct relation_list *list, const i32 outside_index)
{
	i32 unit;
	if (list->free_chain_len > 0)
	{
		unit = list->free_chain;
		list->free_chain = list->r[unit].next;
		list->free_chain_len -= 1;
	}
	else
	{
		unit = list->num_relations;
		list->num_relations += 1;
		arena_push_packed(list->mem, NULL, sizeof(struct relation_unit));
	}

	list->r[unit].related_to = outside_index;
	list->r[unit].next = -1;

	return unit;
}

i32 relation_list_is_related(const struct relation_list *list, const i32 u1, const i32 u2)
{
	assert(u1 >= 0 && u2 >= 0 && u1 < list->num_relations && u2 < list->num_relations);
	
	for (i32 i = list->r[u1].next; i != -1; i = list->r[i].next)
	{
		if (list->r[i].related_to == u2) { return 1; }
	}

	return 0;
}

i32 relation_list_push_union(struct arena *mem, const struct relation_list *list, const i32 u1, const i32 u2)
{
	assert(0 <= u1 && u1 < list->num_relations);
	assert(0 <= u2 && u2 < list->num_relations);

	i32 union_len = 0;

	for (i32 i = list->r[u1].next; i != -1; i = list->r[i].next) 
	{ 
		arena_push_packed(mem, &list->r[i].related_to, sizeof(i32));
		union_len += 1;
	}

	for (i32 i = list->r[u2].next; i != -1; i = list->r[i].next) 
	{ 
		arena_push_packed(mem, &list->r[i].related_to, sizeof(i32));
		union_len += 1;
	}

	return union_len;
}

i32 relation_list_add_relation_unit(struct relation_list *list, const i32 outside_index, const i32 *relations, const i32 num_relations)
{
	const i32 unit = relation_list_add_relation_unit_empty(list, outside_index);

	if (num_relations > 0)
	{
		i32 added = 0;
		if (list->free_chain_len < num_relations)
		{
			added = num_relations - list->free_chain_len;
			arena_push_packed(list->mem, NULL, added * sizeof(struct relation_unit));
		}
	
		i32 chain = list->free_chain;
		if (chain == -1) { chain = list->num_relations; }
		list->r[unit].next = chain;

		i32 i = 0;
		i32 prev = list->free_chain;
		for (; i < list->free_chain_len; ++i)
		{
			list->r[list->free_chain].related_to = relations[i];
			prev = list->free_chain;
			list->free_chain = list->r[prev].next;
		}
	
		for (i32 j = 0; j < added; ++j)
		{
			list->r[prev].next = list->num_relations + j;
			prev = list->num_relations + j;
			list->r[list->num_relations + j].related_to = relations[i];
			i += 1;
		}
	
		assert(list->free_chain == -1 || list->free_chain_len > num_relations);
	
		list->r[prev].next = -1;
		list->free_chain_len -= num_relations - added;
		list->num_relations += added;
	
		for (i32 j = 0; j < num_relations; ++j)
		{
			assert(0 <= relations[j] && relations[j] < list->num_relations);
			relation_list_add_to_relation_unit(list, relations[j], unit);
		}
	}

	return unit;
}

void relation_list_copy_relations(struct relation_list *list, const i32 copy_to, const i32 copy_from)
{
	assert(0 <= copy_to && copy_to < list->num_relations);
	assert(0 <= copy_from && copy_from < list->num_relations);

	for (i32 unit = list->r[copy_from].next; unit != -1; unit = list->r[unit].next)
	{
		relation_list_add_to_relation_unit(list, copy_to, list->r[unit].related_to);
	}
}

void relation_list_add_to_relation_unit(struct relation_list *list, const i32 unit, const i32 relation)
{
	assert(0 <= unit && unit < list->num_relations);
	assert(0 <= relation && relation < list->num_relations);

	const i32 tmp = list->r[unit].next;
	if (list->free_chain_len > 0)
	{
		const i32 i = list->free_chain;
		list->r[unit].next = i;
		list->free_chain = list->r[i].next;
		list->r[i].next = tmp;
		list->r[i].related_to = relation;
		list->free_chain_len -= 1;
	}
	else
	{
		arena_push_packed(list->mem, NULL, sizeof(struct relation_unit));
		list->r[unit].next = list->num_relations;
		list->r[list->num_relations].next = tmp;
		list->r[list->num_relations].related_to = relation; 
		list->num_relations += 1;
	}
}

void relation_list_remove_relation_unit(struct relation_list *list, const i32 unit)
{
	assert(0 <= unit && unit < list->num_relations);
	if (list->r[unit].related_to == -1) { return; }
	list->r[unit].related_to = -1;

	i32 end = unit;
	i32 chain_len = 1;
	while (list->r[end].next != -1)
	{
		end = list->r[end].next;
		relation_list_internal_remove_from_relation_unit(list, list->r[end].related_to, unit);
		chain_len += 1;
	}

	list->r[end].next = list->free_chain;
	list->free_chain = unit;
	list->free_chain_len += chain_len;
}

void relation_list_internal_remove_from_relation_unit(struct relation_list *list, const i32 unit, const i32 relation)
{
	assert(0 <= unit && unit < list->num_relations);
	assert(0 <= relation && relation < list->num_relations);

	i32 prev = unit;
	for (i32 i = list->r[unit].next; i != -1; i = list->r[i].next)
	{
		if (list->r[i].related_to == relation)
		{
			const i32 tmp = list->r[i].next;
			list->r[i].related_to = -1;
			list->r[i].next = list->free_chain;
			list->free_chain = i;
			list->free_chain_len += 1;
			list->r[prev].next = tmp;
			return;
		}
		prev = i;
	}

	assert(0 && "tried to delete relation from unit that does not exist!");
}
