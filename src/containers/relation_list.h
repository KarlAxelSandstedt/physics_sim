#ifndef __RELATION_LIST_H__
#define __RELATION_LIST_H__

#include "mg_common.h"
#include "mg_mempool.h"

/* two-way relation. next == -1 means this is the last relation in the chain. If
 * the entry is in the free chain, next is the next free entry, == -1 means no memory left. 
 */
struct relation_unit {
	i32 related_to; /* if head, index to outside, otherwise related unit */
	i32 next;
};

/**
 * static relationship graph. any relationships added by a node must be added when the node is added.
 * A node already in the graph may not create new relationships, and may only gain new relations when
 * a new node is added to the graph.
 */
struct relation_list {
	struct arena *mem;
	struct relation_unit *r;
	i32 free_chain_len;
	i32 free_chain;
	i32 num_relations;
};

/* initialize given number of units with no relations */
struct relation_list relation_list_init(struct arena *mem, const i32 num_relations);
void relation_list_free(struct relation_list *list);
/* Add empty relation_unit to relation list */
i32 relation_list_add_relation_unit_empty(struct relation_list *list, const i32 outside_index);
/* Assumes added relation is not already in relationship with the unit and that unit is in list */
void relation_list_add_to_relation_unit(struct relation_list *list, const i32 unit, const i32 relation);
/* add unit with outside index and the given relations */
i32 relation_list_add_relation_unit(struct relation_list *list, const i32 outside_index, const i32 *relations, const i32 num_relations);
/* Assumes unit exists in list */
void relation_list_remove_relation_unit(struct relation_list *list, const i32 unit);
/* Assumes unit exists in list and is related to relation */
void relation_list_internal_remove_from_relation_unit(struct relation_list *list, const i32 unit, const i32 relation);
/* Returns 1 if u1, u2 related, 0 otherwise. Does not check if u1 or u2 is within bounds or in free chain */
i32 relation_list_is_related(const struct relation_list *list, const i32 u1, const i32 u2);
/* Returns number of units in both u1 and u2's relations. NOTE that duplicates may occur. the union is pushed onto mem */
/* copy relation to unit, does not check if relations already exist */
void relation_list_copy_relations(struct relation_list *list, const i32 copy_to, const i32 copy_from);
i32 relation_list_push_union(struct arena *mem, const struct relation_list *list, const i32 u1, const i32 u2);

#endif
