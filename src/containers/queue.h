#ifndef __min_queue__
#define __min_queue__

#include <assert.h>

#include "mg_common.h"
#include "mg_mempool.h"

/****************************************************************************/

/**
 * USAGE: Objects to be prioritised should be indexable. The User must himself write priortising routines between the objects
 *		as the queue only consider handles of objects and their priorities.
 */

/**
 * queue_element - Represents the strucutre of a priority queue's elements
 *
 * priority - The priority of the element
 * object_index - The elements corresponding vertex's index
 */
struct queue_element {
	f32 priority;
	int object_index;
};

/**
 * min_queue - A min prioritiy queue which prioritises objects with low priorities
 * 	This means that an element having a low priority will be served faster than an element with a higher
 * 	priority.
 *
 * HOW:
 * 	index = queue.push(priority, id) => queue.elements[index].{ priority, object_index }
 * 	object indices are only used to find storage for user-level indices.
 *
 * 	extract minimum (element[0]) => i = element[0] => user_id = queue->object_index[i]
 *
 * queue_indices - Array of corresponding queue indicies to object indices. queue_indices[i] = queue index of data at position i.
 * elements - Pointer to the elements of the queue, where each element corresponds to a object
 * num_objects - Number of objects considered (static)
 * num_elements - Number of vertices left inside the queue (dynamic)
 */
struct min_queue {
	int *queue_indices;
	struct queue_element *elements;
	int num_objects;
	int num_elements;
	int max_used;
};

/**
 * min_queue_new() - Allocate a new priority queue and set the priority of all objects to
 * 	the largest possible priority, i.e. last position in the queue.
 *
 * arena - Optional arena allocator
 * num_objects - Number of objects to be placed in the queue.
 */
struct min_queue *min_queue_new(struct arena *arena, const int num_objects);

/**
 * min_queue_free() - Free a queue and all it's resources. 
 *
 * queue - The queue to be freed
 */
void min_queue_free(struct min_queue * const queue);

/**
 * min_queue_extract_min() - Retrieve a pointer the the vertex with the lowest priority, and remove the
 * 	object from the queue, and keep the queue coherent.
 *
 * queue - the queue
 * 
 * Return: Object index of first element in queue.
 */
i32 min_queue_extract_min(struct min_queue * const queue);

/**
 * min_queue_decrease_priority() - Decrease the priority of the object corresponding to queue_index
 * 	if the priority is smaller than it's current priority. If changes are made in the queue, the queue
 * 	is updated and kept coherent. 
 *
 * queue - The queue
 * queue_index - index containing priority to be changed
 * priority - The new priority
 */ 
void min_queue_decrease_priority(struct min_queue * const queue, const int queue_index, const f32 priority);

/* append new element with given priority to queue, return its index into queue_indices */
i32 min_queue_insert(struct min_queue * const queue, const f32 priority);

/****************************************************************************/

struct heap_slot
{
	u64 id;
	f32 priority;
};

/**
 * Use min_heap if the priorities are constant throughout the queue (That is why min_queue has queue_indicies)
 */
struct min_heap
{
	struct heap_slot *elements;
	i32 count;
	i32 max_count;
};

struct min_heap *min_heap_new(struct arena *mem, const i32 max_count);
void min_heap_free(struct min_heap *heap);
void min_heap_print(FILE *log, struct min_heap *heap);
void min_heap_push(struct min_heap *heap, const f32 priority, const u64 id);
u64 min_heap_pop(struct min_heap *heap);
u64 min_heap_peek(struct min_heap *heap);

/****************************************************************************/

/**
 * lifo_queue - Last in First out queue. Hence the last object being added to the queue before a removal is also the item to be removed. 
 * 
 * queue_indices - Array of corresponding queue indicies to object indices. queue_indices[i] = queue index of object at position i.
 * elements - Pointer to the elements of the queue, where each integer element corresponds to a object index
 * num_objects - Number of objects considered (static)
 * num_elements - Number of vertices left inside the queue (dynamic)
 */
struct lifo_queue {
	int* queue_indices;
	int* elements;
	int num_objects;
	int num_elements;
	int max_queue_size;
};

struct lifo_queue* lifo_queue_new(const int num_objects, const int max_queue_size);		/* queue_index == -1 <=> object is not in queue. */
void lifo_queue_free(struct lifo_queue* const queue);
void lifo_queue_add(struct lifo_queue* const queue, const int object_index);			/* Add object to queue if object is not already in queue, and queue is not full */
int lifo_queue_extract(struct lifo_queue * const queue);								/* Extract the last object index in the queue, with -1 equating to the empty queue case */

#endif
