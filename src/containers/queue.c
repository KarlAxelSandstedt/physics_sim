#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "queue.h"

/**
 * parent_index() - get queue index of parent.
 *
 * queue_index - index of child
 *
 * RETURNS: the index of the parent. If the function is called on the element first in the queue (index 0),
 * 	then an error value of -1 is returned: 0/2 - ((0 AND 1) XOR 1) = 0 - (0 XOR 1) = -1.
 */
static i32 parent_index(const i32 queue_index)
{
	return queue_index / 2 - ((queue_index & 0x1) ^ 0x1);
}

/**
 * left_index() - get queue index of left child 
 *
 * queue_index - index of parent 
 */
static i32 left_index(const i32 queue_index)
{
	return (queue_index << 1) | 0x1;
}

/**
 * right_index() - get queue index of right child 
 *
 * queue_index - index of parent 
 */
static i32 right_index(const i32 queue_index)
{
	return (queue_index + 1) << 1;
}

/**
 * min_queue_change_elements() - Change two elements in the element array, and update their corresponding 
 * 	data' queue_index
 *
 * queue - The queue
 * i1 - queue index of element 1
 * i2 - queue index of element 2
 */
static void min_queue_change_elements(struct min_queue * const queue, const i32 i1, const i32 i2)
{
	/* Update data queue indices */
	queue->queue_indices[queue->elements[i1].object_index] = i2;
	queue->queue_indices[queue->elements[i2].object_index] = i1;

	/* Update priorities */
	const f32 tmp_priority = queue->elements[i1].priority;
	queue->elements[i1].priority = queue->elements[i2].priority;
	queue->elements[i2].priority = tmp_priority;

	/* update queue's data indices */
	const i32 tmp_index = queue->elements[i1].object_index;
	queue->elements[i1].object_index = queue->elements[i2].object_index;
	queue->elements[i2].object_index = tmp_index;
}

/**
 * min_queue_heapify_up() - Keep the queue coherent after a decrease of the queue_index's priority has been made.
 * 	A Decrease of the priority may destroy the coherency of the queue, as the parent should always be infront
 * 	of the children in the queue. decreasing the priority of a child may thus result in the child having a
 * 	lower priority than it's parent, so they have to be i32erchanged.
 *
 * queue - The queue
 * queue_index - The index of the queue element who just had it's priority decreased.
 */
static void min_queue_heapify_up(struct min_queue * const queue, i32 queue_index)
{
	i32 parent = parent_index(queue_index);

	/* Continue until parent's priority is smaller than child or the root has been reached */
	while (parent != -1 && queue->elements[queue_index].priority < queue->elements[parent].priority) {
		min_queue_change_elements(queue, queue_index, parent);
		queue_index = parent;
		parent = parent_index(queue_index); 
	}
}

static void recursion_done(struct min_queue * const queue, const i32 queue_index, const i32 small_priority_index);
static void recursive_call(struct min_queue * const queue, const i32 queue_index, const i32 small_priority_index);

void (*func[2])(struct min_queue * const, const i32, const i32) = { &recursion_done, &recursive_call };

/**
 * min_queue_heapify_down() - Keeps the queue coherent when the element at queue_index has had 
 * 	it's priority increased. Then the element may have a higher priority than it's children, breaking
 * 	the min-heap property.
 *
 * queue - The queue
 * queue_index - Index of the element whose priority has been increased
 */
static void min_queue_heapify_down(struct min_queue * const queue, const i32 queue_index)
{
	const i32 left = left_index(queue_index);
	const i32 right = right_index(queue_index);
	i32 smallest_priority_index = queue_index;

	if (left < queue->num_elements && queue->elements[left].priority < queue->elements[smallest_priority_index].priority)
		smallest_priority_index = left;
	
	if (right < queue->num_elements && queue->elements[right].priority < queue->elements[smallest_priority_index].priority)
		smallest_priority_index = right;
	
	/* Child had smaller priority */
	/* assumes i32 == 4B */
	func[ ((u32) queue_index - smallest_priority_index) >> 31 ](queue, queue_index, smallest_priority_index);
}

static void recursion_done(struct min_queue * const queue, const i32 queue_index, const i32 small_priority_index)
{
	return;
}

static void recursive_call(struct min_queue * const queue, const i32 queue_index, const i32 smallest_priority_index)
{
		min_queue_change_elements(queue, queue_index, smallest_priority_index);
		/* Now child may break the min-heap property */
		min_queue_heapify_down(queue, smallest_priority_index);
}

struct min_queue *min_queue_new(struct arena *arena, const i32 num_objects)
{
	assert(num_objects > 0);

	struct min_queue *queue = NULL;

	if (arena)
	{
		queue = (struct min_queue *) arena_push(arena, NULL, sizeof(struct min_queue));
		queue->queue_indices = (i32 *) arena_push(arena, NULL, num_objects * sizeof(i32));
		queue->elements = (struct queue_element *) arena_push(arena, NULL, num_objects * sizeof(struct queue_element));
	}
	else
	{
		queue = malloc(sizeof(struct min_queue));
		queue->queue_indices = malloc(num_objects * sizeof(i32));
		queue->elements = malloc(num_objects * sizeof(struct queue_element));
	}

	queue->num_objects = num_objects;
	queue->num_elements = 0;
		
	for (i32 i = 0; i < num_objects; ++i) {
		queue->queue_indices[i] = i;
		queue->elements[i].object_index = i;
		queue->elements[i].priority = FLT_MAX;
	}

	return queue;
}

void min_queue_free(struct min_queue * const queue)
{
	free(queue->queue_indices);
	free(queue->elements);
	free(queue);
}

i32 min_queue_extract_min(struct min_queue * const queue)
{
	assert(queue->num_elements > 0 && "Queue should have elements to extract\n");
	queue->num_elements -= 1;
	const i32 object_index = queue->elements[0].object_index;
	queue->elements[0].priority = FLT_MAX;

	/* Keep the array of elements compact */
	min_queue_change_elements(queue, 0, queue->num_elements);
	/* Check coherence of the queue from the start */
	min_queue_heapify_down(queue, 0);

	//queue->queue_indices[queue->num_elements] = queue->num_elements;
	//queue->elements[queue->num_elements].object_index = queue->num_elements;

	return object_index;
}

i32 min_queue_insert(struct min_queue * const queue, const f32 priority)
{
	const i32 queue_index = queue->num_elements;
	//assert(priority >= 0.0f && "Priorities should be non-negative");
	assert(queue_index < queue->num_objects && "Queue index should be withing queue bounds");

	queue->num_elements += 1;	
	const i32 object_index = queue->elements[queue_index].object_index;
	queue->elements[queue_index].priority = priority;
	min_queue_heapify_up(queue, queue_index);

	return object_index;
}

void min_queue_decrease_priority(struct min_queue * const queue, const i32 queue_index, const f32 priority)
{
	//assert(priority >= 0.0f && "Priorities should be non-negative");
	assert(queue_index < queue->num_elements && "Queue index should be withing queue bounds");

	if (priority < queue->elements[queue_index].priority) {
		queue->elements[queue_index].priority = priority;
		min_queue_heapify_up(queue, queue_index);
	}
}

struct lifo_queue* lifo_queue_new(const i32 num_objects, const i32 max_queue_size)
{
	//assert(num_objects > 0);
	assert(max_queue_size > 0);

	struct lifo_queue* queue = malloc(sizeof(struct lifo_queue));

	queue->num_objects = num_objects;
	queue->num_elements = 0;
	queue->max_queue_size = max_queue_size;
	queue->queue_indices = malloc(num_objects * sizeof(i32));
	queue->elements = malloc(max_queue_size * sizeof(i32));

	for (i32 i = 0; i < num_objects; ++i) {
		queue->queue_indices[i] = -1;
	}

	return queue;
}

void lifo_queue_free(struct lifo_queue* const queue)
{
	free(queue->queue_indices);
	free(queue->elements);
	free(queue);
}

void lifo_queue_add(struct lifo_queue* const queue, const i32 object_index)
{
	assert(0 <= object_index && object_index < queue->num_objects);

	if (queue->queue_indices[object_index] == -1 && queue->num_elements < queue->max_queue_size) {
		queue->elements[queue->num_elements] = object_index;
		queue->queue_indices[object_index] = queue->num_elements;
		queue->num_elements += 1;
	}
}

i32 lifo_queue_extract(struct lifo_queue* const queue)
{
	if (queue->num_elements) {
		queue->num_elements -= 1;
		const i32 object_index = queue->elements[queue->num_elements];
		queue->queue_indices[object_index] = -1;
		return object_index;
	} else {
		return -1;
	}
}

static void min_heap_change_elements(struct min_heap * const heap, const i32 i1, const i32 i2)
{
	/* Update priorities */
	const f32 tmp_priority = heap->elements[i1].priority;
	heap->elements[i1].priority = heap->elements[i2].priority;
	heap->elements[i2].priority = tmp_priority;

	/* update heap's data indices */
	const u64 tmp_id = heap->elements[i1].id;
	heap->elements[i1].id = heap->elements[i2].id;
	heap->elements[i2].id = tmp_id;
}


static void min_heap_heapify_up(struct min_heap * const heap, i32 heap_index)
{
	i32 parent = parent_index(heap_index);

	/* Continue until parent's priority is smaller than child or the root has been reached */
	while (parent != -1 && heap->elements[heap_index].priority < heap->elements[parent].priority) {
		min_heap_change_elements(heap, heap_index, parent);
		heap_index = parent;
		parent = parent_index(heap_index); 
	}
}

static void heap_recursion_done(struct min_heap * const heap, const i32 heap_index, const i32 small_priority_index);
static void heap_recursive_call(struct min_heap * const heap, const i32 heap_index, const i32 small_priority_index);

void (*heap_func[2])(struct min_heap * const, const i32, const i32) = { &heap_recursion_done, &heap_recursive_call };

/**
 * min_heap_heapify_down() - Keeps the heap coherent when the element at heap_index has had 
 * 	it's priority increased. Then the element may have a higher priority than it's children, breaking
 * 	the min-heap property.
 *
 * heap - The heap
 * heap_index - Index of the element whose priority has been increased
 */
static void min_heap_heapify_down(struct min_heap * const heap, const i32 heap_index)
{
	const i32 left = left_index(heap_index);
	const i32 right = right_index(heap_index);
	i32 smallest_priority_index = heap_index;

	if (left < heap->count && heap->elements[left].priority < heap->elements[smallest_priority_index].priority)
		smallest_priority_index = left;
	
	if (right < heap->count && heap->elements[right].priority < heap->elements[smallest_priority_index].priority)
		smallest_priority_index = right;
	
	/* Child had smaller priority */
	heap_func[ ((u32) heap_index - smallest_priority_index) >> 31 ](heap, heap_index, smallest_priority_index);
}

static void heap_recursion_done(struct min_heap * const heap, const i32 heap_index, const i32 small_priority_index)
{
	return;
}

static void heap_recursive_call(struct min_heap * const heap, const i32 heap_index, const i32 smallest_priority_index)
{
		min_heap_change_elements(heap, heap_index, smallest_priority_index);
		min_heap_heapify_down(heap, smallest_priority_index);
}

struct min_heap *min_heap_new(struct arena *mem, const i32 max_count)
{
	assert(max_count > 0);

	struct min_heap *heap = NULL;
	if (mem)
	{
		heap = arena_push(mem, NULL, sizeof(struct min_heap));
		heap->elements = arena_push(mem, NULL, max_count * sizeof(struct heap_slot));
	}
	else
	{
		heap = malloc(sizeof(struct min_heap));
		heap->elements = malloc(max_count * sizeof(struct heap_slot));
	}

	heap->max_count = max_count;
	heap->count = 0;

	return heap;
}

void min_heap_free(struct min_heap *heap)
{
	free(heap->elements);
	free(heap);
}

void min_heap_push(struct min_heap *heap, const f32 priority, const u64 id)
{
	const i32 heap_index = heap->count;
	assert(heap_index < heap->max_count && "Heap index should be withing heap bounds");

	heap->count += 1;	
	heap->elements[heap_index].priority = priority;
	heap->elements[heap_index].id = id;
	min_heap_heapify_up(heap, heap_index);
}

u64 min_heap_pop(struct min_heap *heap)
{
	assert(heap->count > 0 && "Heap should have elements to extract\n");
	heap->count -= 1;

	const u64 id = heap->elements[0].id;
	min_heap_change_elements(heap, 0, heap->count);
	min_heap_heapify_down(heap, 0);

	return id;
}

u64 min_heap_peek(struct min_heap *heap)
{
	assert(heap->count > 0 && "Heap should have elements to extract\n");
	const u64 id = heap->elements[0].id;

	return id;
}

void min_heap_print(FILE *log, struct min_heap *heap)
{
	fprintf(log, "min heap %p: ", heap);
	fprintf(log, "{ ");
	for (i32 i = 0; i < heap->count; ++i)
	{
		fprintf(log, "%f, ", heap->elements[i].priority);
	}
	fprintf(log, "}\n");
}
