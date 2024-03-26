#ifndef __HIERARCHY_H__
#define __HIERARCHY_H__

struct hierarchy {
	struct hierarchy* parent;
	struct hierarchy* sibling;
	struct hierarchy* child;
	void* owner;
};

struct hierarchy* hierarchy_new(void);									/* Create a new hierarchy with zeroed out data */
void hierarchy_free(struct hierarchy* hierarchy);			/* Release all memory used and pointer to from the hierarchy */
void hierarchy_set_owner(struct hierarchy* const hierarchy, void *owner);											
void hierarchy_set_parent(struct hierarchy* const parent, struct hierarchy* const child); /* child become first child of parent */
void hierarchy_remove_from_parent(struct hierarchy* const child);									/* child becomes fatherless, previous sibling get sibling as sibling */
void hierarchy_make_sibling_after(struct hierarchy* const sibling, struct hierarchy* const hierarchy);	/* make hierarchy the sibling of sibling */
struct hierarchy* hierarchy_get_previous_sibling(const struct hierarchy* const hierarchy);				/* Get the previous sibling of hierarchy (NULL if none) */
void* hierarchy_get_owner(const struct hierarchy* const hierarchy);
void* hierarchy_get_parent_owner(const struct hierarchy* const child);
void* hierarchy_get_sibling_owner(const struct hierarchy* const hierarchy);

#endif

