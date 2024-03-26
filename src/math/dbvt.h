#ifndef __DYNAMIC_BOUNDING_VOLUME_TREE_H__
#define __DYNAMIC_BOUNDING_VOLUME_TREE_H__

#include "mg_common.h"
#include "geometry.h"
#include "queue.h"
#include "float.h"

/**
 * Basic steps for a simple dynamic bounding volume hierarchy:
 *
 * INCREMENTAL ADD
 * 	(1) Alloc leaf node
 * 	(2) Find the best sibling for the new volume
 * 	(3) Add parent to sibling and new node
 * 	(4) Rebase the tree, in order to balance it at keep the performance good
 *
 * INCREMENTAL CHECK OVERLAP
 * 	- several possibilities, we should as a first step try descendLargestVolume.
 * 	  If one goes through a whole tree at a time (depth first) one may get into really bad
 * 	  situations as comparing super small object vs relatively large ones, and check overlaps
 * 	  all the time.
 *
 * INCREMENTAL REMOVE
 * 	(1) Remove leaf
 * 	(2) Set sibling as parent leaf
 *
 * Some useful stuff for debugging purposes and performance monitoring would be:
 * 	- draw line box around AABB volumes
 * 	- average number of overlaps every frame
 * 	- number of nodes
 * 	- deepest leaves
 *
 * Some general optimisations
 * 	- enlarged AABBs somehow (less reinserts)
 * 	- do not recompute cost of child in balance if...
 * 	- clever strategy for how to place parant vs child nodes (left always comes after parent...)
 * 	  We want to make the cache more coherent in traversing the tree
 * 	- remove min_queue queue_indices, aren't they unnecessary?
 * 	- Another strategy for cache coherency while traversing would be double layer nodes (parent, child, child) 
 * 	- Global caching of collisions (Hash table of previous primitive collisions)
 * 	- Local caching (Every single object has a cache of collisions)
 * 	- SIMD AABB operations
 */

#define DBVT_NO_NODE -1
#define COST_QUEUE_MAX 124

struct dbvt_node {
	struct AABB box;
	i32 id;		/* id == -1 <=> end of free chain */
	i32 parent;
	i32 left;
	i32 right;
};

struct dbvt
{
	struct min_queue *cost_queue;
	struct dbvt_node *nodes;	
	i32 cost_index[COST_QUEUE_MAX];
	i32 proxy_count; 
	i32 root;
	i32 next;
	i32 len;
};

/* If mem == NULL, standard malloc is used */
struct 	dbvt dbvt_alloc(struct arena *mem, const i32 len);
/* id is an integer identifier from the outside, return index of added value */
i32 	dbvt_insert(struct dbvt *tree, const i32 id, const struct AABB *box);
/* remove leaf corresponding to index from tree */
void 	dbvt_remove(struct dbvt *tree, const i32 index);
/* push overlap indices onto mem->stack_ptr; returns number of collisions. -1 == out of memory */
i32 	dbvt_push_overlap_pairs(struct arena *mem, struct dbvt *tree);
/* validate tree construction */
void	dbvt_validate(struct dbvt *tree);
/* push heirarchy node box lines into draw buffer */
void	dbvt_push_lines(struct drawbuffer *buf, struct dbvt *tree, const vec4 color);
/* TODO: Calculate tree cost */
f32	dbvt_cost(struct dbvt *tree);
/* TODO: Calculate tree maximal depth */
i32 	dbvt_depth(struct dbvt *tree);
/* TODO: Calculate memory size  */
u64 	dbvt_memory_usage(struct dbvt *tree);

#endif
