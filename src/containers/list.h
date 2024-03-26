#ifndef __LINKED_LIST__
#define __LINKED_LIST__

/**
 * struct list - A singly linked list.
 *
 * @first: The address of the first node.
 * @last: The address of the last node.
 */
struct list {
	struct node *first;
	struct node *last;
	size_t length;
};

struct node {
	struct node *next;
	void *x;
};


/**
 * list_new() - Create a list.
 * @arg1: The value to be stored in node 1.
 * @argN: The value to be stored in node N.
 * @argN+1: NULL to indicate no more inputs given.
 *
 * The list to be created will contain N nodes if N+1 inputs was used, and assumes that
 * no more parameters were given after NULL. The first parameter will correspond to the
 * first node, the second parameter corresponds to the second node, and so forth... If
 * null is the only input parameter, an empty list is created.
 *
 * Return: The new list. 
 *
 * Error: It is an unchecked runtime error to omit the casts for the second to N:th
 * 	  argument (unless the arguments are of type char * or void *) since list_new
 * 	  expects inputs of void *. If memory allocation failed, exit.
 */
struct list *list_new(void *x, ...);

/**
 * list_concat() - Concatenate lists.
 * @l: The list to be concatenated to.
 * @tail: The list to be concatenated.
 *
 * list_concat creates the concatenated list l->...->tail->... . If both l and tail
 * are both allocated structures, the list structure of tail will be freed. If any 
 * of l or tail is null, or tail is simply empty, do nothing.
 */
void list_concat(struct list *l, struct list *tail);

/**
 * list_push() - Push to a list.
 * @l: The list to push the new value to.
 * @x: The value.
 *
 * Prepends a value to a list that is not null.
 *
 * Error: It is an checked runtime error to push to a list that is null. If memory
 *	   allocation failed, exit. 
 */
void list_push(struct list *l, void *x);

/**
 * list_copy() - Copies a list.
 * @l: The list to be copied.
 *
 * Creates a new list with the same values as the given list.
 *
 * Return: The new list.
 *
 * Error: If memory allocation failed, exit.
 */
struct list *list_copy(struct list *l);

/**
 * list_reverse() - Reverse a list:
 * @l: The list to be reversed.
 *
 * Reverses a given list. If the list is null, or simply empty, do nothing.
 */
void list_reverse(struct list *l);

/**
 * list_length() - Return the length of the list.
 * @l: The list.
 *
 * Retrieve the length of a list that is not null. It is a checked runtime error
 * to get the length of a list that is null.
 *
 * Return: The length of the list.
 */
size_t list_length(struct list *l);

/**
 * list_pop() - Pop from a list.
 * @l: The list to be popped.
 * @x: Address to storage for popped node's value.
 *
 * Given a list, remove and free the first node and retrieve the value inside
 * of it. If the list happens to be empty or simply null, do nothing.
 */
void list_pop(struct list *l, void **x);

/**
 * list_append() - Append a value to the end of a list.
 * @l: The list to be appended to.
 * @x: The value to append.
 *
 * Appends a value to a non-null list. 
 *
 * Error: It is an checked runtime error to append to a list that is null. If
 * 	  memory allocation failed, exit.
 */
void list_append(struct list *l, void *x);

/**
 * list_free() - Free a list.
 * @l: The list to be freed.
 *
 * If the given list is not null, deallocate the list and all of its nodes and set
 * the list to null. Otherwise do nothing.
 */
void list_free(struct list **l);

/**
 * list_map() - Call a fuction on all the nodes of the list.
 * @l: The list.
 * @apply: The function to be applied.
 * @cl: The application specific pointer.
 *
 * Apply the function apply on all nodes in the list. For each node, apply is called
 * a with a pointer to the value field and cl. If the list is null or simply empty,
 * do nothing.
 *
 * Error: It is a checked runtime error to set apply as null. It is an unchecked
 * 	  runtime error for apply to change the list. 
 */
void list_map(struct list *l, void apply(void **x, void *cl), void *cl);

/**
 * list_to_array() - Create an array of values from a list.
 * @l: The list.
 * @end: The terminating value of the created array.
 *
 * Create an array of N+1 values, where index 0 corresponds to the list's first node's
 * value, and index N-1 corresponds the the list's last node's value. Index N is set
 * to the value end.
 *
 * Return: The array on the heap containing the list's values, terminated with end.
 *
 * If the list is null or simply empty, an array of one element that is set to end 
 * is returned. Since the array is allocated on the heap, it is up to the user to
 * free it later.
 *
 * Error: If memory allocation failed, exit.
 */
void **list_to_array(struct list *l, void *end);

#endif
