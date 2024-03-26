#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>

#include "list.h"

#define ALLOC_FAILED "Error: failed memory allocation in list module."

static void mem_alloc_error()
{
		fprintf(stderr, "%s\n", ALLOC_FAILED);
		exit(EXIT_FAILURE);
}
		
/**
 * node_create() - Create a node.
 * @parent: The node that should point the new node.
 * @x: The value of the new node.
 *
 * Appends a node to the parent node. If the parent node is null, the new node will
 * not be appended to any existing node.
 *
 * Return: If success, the new node.
 *
 * Error: If memory allocation fails, exit.
 */
static struct node *node_create(struct node *parent, void *x)
{
	struct node *child = malloc(sizeof(struct node));
	if(!child)
		mem_alloc_error();

	child->next = NULL;
	child->x = x;

	if (parent)
		parent->next = child;

	return child;
}

struct list *list_new(void *x, ...)
{
	struct list *l = malloc(sizeof(struct list));
	if (!l)
		mem_alloc_error();

	l->length = 0;
	l->first = NULL;
	l->last = NULL;

	va_list ap;
	va_start(ap, x);

	while (x) {
		list_append(l, x);
		x = va_arg(ap, void *);
	}

	va_end(ap);

	return l;
}

void list_concat(struct list *l, struct list *tail)
{
	if (!l || !tail || tail->length == 0)
		return;

	if (l->length == 0) {
		l->first = tail->first;
		l->last = tail->last;
		l->length = tail->length;
	} else {
		l->last->next = tail->first;
		l->last = tail->last;
		l->length += tail->length;
	}

	tail->first = NULL;
	tail->last = NULL;
	tail->length = 0;
}

void list_push(struct list *l, void *x)
{
	assert(l);	

	struct node *n = node_create(NULL, x);
	if (!n)
		mem_alloc_error();

	if (l->length > 0) {
		n->next = l->first;
		l->first = n;
	} else {
		l->first = n;
		l->last = l->first;
	}

	l->length += 1;
}

struct list *list_copy(struct list *l)
{
	if (!l)
		return NULL;

	struct list *cpy = list_new(NULL);
	if (!cpy)
		mem_alloc_error();

	if (l->length == 0)
		return cpy;

	struct node *tmp = l->first;

	for (; tmp; tmp = tmp->next)
		list_append(cpy, tmp->x);

	return cpy;
}

void list_reverse(struct list *l)
{
	if (!l || l->length <= 1)
		return;

	struct node *old_chain = l->first->next;
	struct node *old_head = NULL;

	l->last = l->first;
	l->last->next = NULL;
	
	while (old_chain) {
		old_head = l->first;
		l->first = old_chain;
		old_chain = old_chain->next;
		l->first->next = old_head;
	}
}

size_t list_length(struct list *l)
{
	assert(l);

	return l->length;
}

void list_pop(struct list *l, void **x)
{
	if (!l || l->length == 0)
		return;

	struct node *tmp = l->first;
	*x = l->first->x;

	if (l->length > 1) {
		l->first = l->first->next;
	} else {
		l->first = NULL;
		l->last = NULL;
	}

	l->length -= 1;

	free(tmp);
}

void list_append(struct list *l, void *x)
{
	assert(l);

	if (l->length > 0) {
		l->last = node_create(l->last, x);
	} else {
		l->first = node_create(NULL, x);
		l->last = l->first;
	}

	if (l->last)
		l->length += 1;
	else
		mem_alloc_error();
} 

void list_free(struct list **l)
{
	if (*l == NULL)
		return;

	struct node *n = (*l)->first;
	struct node *tmp;

	while (n) {
		tmp = n->next;
		free(n);
		n = tmp;
	}

	free(*l);
	*l = NULL;
}

void list_map(struct list *l, void apply(void **x, void *cl), void *cl)
{
	assert(apply);

	if (!l || l->length == 0)
		return;

	struct node *n = l->first;
	for (; n; n = n->next)
		apply(&(n->x), cl);
}

void **list_to_array(struct list *l, void *end)
{
	void **vals = NULL;

	if (!l) {
		if ((vals = malloc(sizeof(void *))) == NULL)
			mem_alloc_error();

		*vals = end;
	} else {
		if ((vals = malloc(sizeof(void *) * (l->length + 1))) == NULL)
			mem_alloc_error();
		
		struct node *n = l->first;
		for (size_t i = 0; i < l->length; ++i) {
			vals[i] = n->x;
			n = n->next;
		}
	
		vals[l->length] = end;
	}

	return vals;
}
