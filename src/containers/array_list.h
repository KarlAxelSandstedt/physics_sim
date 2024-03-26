#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

#include "mg_common.h"
#include "mg_mempool.h"

struct array_list
{
	u64 length;	/* array length */
	u64 max_count;	/* max count used over the object's lifetime */
	u64 count;	/* current count of occupied slots */
	u64 data_size;
	u64 slot_size;	/* slot size, minimum size 8 so we can store free list chain of pointers */
	u8 *slot;
	void **free_chain;
};

struct array_list *array_list_new(struct arena *mem, const u64 length, const u64 data_size);
void array_list_free(struct array_list *list);
void *array_list_reserve(struct array_list *list); 	/* reserve slot = returned address, or NULL if no space */
void *array_list_add(struct array_list *list, void *data);	/* return slot and fill slot with data */
void array_list_remove(struct array_list *list, void *addr);	/* add address to free chain */
void array_list_remove_index(struct array_list *list, const u64 index); /* add slot[index] to free chain  */
void *array_list_address(struct array_list *list, const u64 index);	/* get address at slot[index] */

/* Generational array list, keeps track of the generation of the array list elements */
struct gen_array_list
{
	void **free_chain;
	u64 data_size;
	u64 slot_size;	/* slot size, minimum size 8 so we can store free list chain of pointers */

	u8 *slot;
	u32 *generation;

	u32 length;	/* array length */
	u32 max_count;	/* max count used over the object's lifetime */
	u32 count;	/* current count of occupied slots */
};

struct gen_array_list *gen_array_list_new(struct arena *mem, const u64 length, const u64 data_size);
void gen_array_list_free(struct gen_array_list *list);
void *gen_array_list_reserve(u64 *gen_index, struct gen_array_list *list); 	/* reserve slot = returned address, or NULL if no space */
void *gen_array_list_add(u64 *gen_index, struct gen_array_list *list, void *data);	/* return slot and fill slot with data */
void gen_array_list_remove(struct gen_array_list *list, void *addr);	/* add address to free chain */
void gen_array_list_remove_index(struct gen_array_list *list, const u64 gen_index); /* add slot[index] to free chain  */
void *gen_array_list_generation_address(struct gen_array_list *list, const u64 gen_index);	/* get address at slot given that the slot is of the correct generation */
void *gen_array_list_address(struct gen_array_list *list, const u64 gen_index);	/* get address at slot[index] */



#endif
