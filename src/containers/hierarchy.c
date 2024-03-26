#include <stdlib.h>
#include <assert.h>

#include "hierarchy.h"

struct hierarchy* hierarchy_new(void)
{
	struct hierarchy* hierarchy = malloc(sizeof(struct hierarchy));

	hierarchy->parent = NULL;
	hierarchy->sibling = NULL;
	hierarchy->child = NULL;
	hierarchy->owner = NULL;

	return hierarchy;
}

void hierarchy_free(struct hierarchy* hierarchy)
{
	assert(hierarchy);

	if (hierarchy->child) {
		hierarchy_free(hierarchy->child);
	}

	if (hierarchy->sibling) {
		hierarchy_free(hierarchy->sibling);
	}
	
	free(hierarchy->owner);
	free(hierarchy);
}

void hierarchy_set_owner(struct hierarchy* const hierarchy, void* owner)
{
	assert(hierarchy);

	hierarchy->owner = owner;
}

void hierarchy_set_parent(struct hierarchy* const parent, struct hierarchy* const child)
{
	assert(parent && child);

	hierarchy_remove_from_parent(child);
	child->parent = parent;
	child->sibling = parent->child;
	parent->child = child;
}

void hierarchy_remove_from_parent(struct hierarchy* const child)
{
	assert(child);
	if (!child->parent)
		return;

	struct hierarchy* const previous_sibling = hierarchy_get_previous_sibling(child);
	if (previous_sibling) {
		previous_sibling->sibling = child->sibling;
	}
	else {
		child->parent->child = child->sibling;
	}

	child->parent = NULL;
	child->sibling = NULL;
}

void hierarchy_make_sibling_after(struct hierarchy* const sibling, struct hierarchy* const hierarchy)
{
	assert(sibling && hierarchy);

	hierarchy_remove_from_parent(hierarchy);
	hierarchy->parent = sibling->parent;
	hierarchy->sibling = sibling->sibling;
	sibling->sibling = hierarchy;
}

struct hierarchy* hierarchy_get_previous_sibling(const struct hierarchy* const hierarchy)
{
	assert(hierarchy);

	if ( (!hierarchy->parent )|| (hierarchy->parent->child == hierarchy))
		return NULL;

	const struct hierarchy* child = hierarchy->parent->child;
	while (child->sibling != hierarchy) {
		child = child->sibling;
		assert(child);
	}

	return child;
}

void* hierarchy_get_owner(const struct hierarchy* const hierarchy)
{
	assert(hierarchy);

	return hierarchy->owner;
}

void* hierarchy_get_parent_owner(const struct hierarchy* const child)
{
	assert(child);

	if (child->parent) {
		return child->parent->owner;
	}
	else {
		return NULL;
	}
}

void* hierarchy_get_sibling_owner(const struct hierarchy* const hierarchy)
{
	assert(hierarchy);

	if (hierarchy->sibling) {
		return hierarchy->sibling->owner;
	}
	else {
		return NULL;
	}
}
