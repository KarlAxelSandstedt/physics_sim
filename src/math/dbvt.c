#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbvt.h"

static i32 dbvt_internal_alloc_node(struct dbvt *tree, const i32 id, const struct AABB *box)
{
	if (tree->next != DBVT_NO_NODE)
	{
		const i32 index = tree->next;
		tree->next = tree->nodes[index].id;
		tree->nodes[index].id = id;
		tree->nodes[index].parent = DBVT_NO_NODE;
		tree->nodes[index].left = DBVT_NO_NODE;
		tree->nodes[index].right = DBVT_NO_NODE;
		memcpy(&tree->nodes[index].box, box, sizeof(struct AABB));
		return index;
	}
	else
	{
		printf("OUT-OF-MEMORY in dbvt_internal_alloc_node");
		return DBVT_NO_NODE;
	}
}

static i32 dbvt_internal_free_node(struct dbvt *tree, const i32 index)
{
	assert(index >= 0 && index < tree->len && "DBVT: free index out of bounds");

	const i32 id = tree->nodes[index].id;
	tree->nodes[index].id = tree->next;
	tree->next = index;

	return id;
}

struct dbvt dbvt_alloc(struct arena *mem, const i32 len)
{
	struct dbvt tree =
	{
		.len = len,
		.proxy_count = 0,
		.root = DBVT_NO_NODE,
		.next = 0,
	};

	if (mem)
	{
		tree.nodes = (struct dbvt_node *) arena_push(mem, NULL, len * sizeof(struct dbvt_node));
		tree.cost_queue = min_queue_new(mem, COST_QUEUE_MAX);
	}
	else
	{
		tree.nodes = malloc(len * sizeof(struct dbvt_node));
		tree.cost_queue = min_queue_new(NULL, COST_QUEUE_MAX);
	}

	for (i32 i = 0; i < len-1; ++i)
	{
		tree.nodes[i].id = i+1;
	}
	tree.nodes[len-1].id = DBVT_NO_NODE;

	return tree;
}

static f32 cost_SVT(const struct AABB *box)
{
	return box->hw[0]*box->hw[1]*box->hw[2];
}

static f32 cost_SAT(const struct AABB *box)
{
	return box->hw[0]*(box->hw[1] + box->hw[2]) + box->hw[1]*box->hw[2];
}

static void dbvt_internal_balance_node(struct dbvt *tree, const i32 node)
{
	/* (1) find best rotation */
	i32 left = tree->nodes[node].left;
	i32 right = tree->nodes[node].right;
	struct AABB box_union;
	f32 cost_rotation, cost_original, cost_best = FLT_MAX;
			
	i32 upper_rotation; /* child to rotate */
	i32 best_rotation = DBVT_NO_NODE; /* best grandchild to rotate */
	if (tree->nodes[left].left != DBVT_NO_NODE)
	{
		AABB_union(&box_union, &tree->nodes[tree->nodes[left].left].box, &tree->nodes[right].box);
		cost_original = cost_SAT(&tree->nodes[left].box);	
		cost_rotation = cost_SAT(&box_union);
		if (cost_rotation < cost_original)
		{
			upper_rotation = right;
			best_rotation = tree->nodes[left].right;
			cost_best = cost_rotation;
		}

		AABB_union(&box_union, &tree->nodes[tree->nodes[left].right].box, &tree->nodes[right].box);
		cost_rotation = cost_SAT(&box_union);
		if (cost_rotation < cost_best && cost_rotation < cost_original)
		{
			upper_rotation = right;
			best_rotation = tree->nodes[left].left;
			cost_best = cost_rotation;
		}
	}

	if (tree->nodes[right].left != DBVT_NO_NODE)
	{
		AABB_union(&box_union, &tree->nodes[tree->nodes[right].left].box, &tree->nodes[left].box);
		cost_original = cost_SAT(&tree->nodes[right].box);	
		cost_rotation = cost_SAT(&box_union);
		if (cost_rotation < cost_best && cost_rotation < cost_original)
		{
			upper_rotation = left;
			best_rotation = tree->nodes[right].right;
			cost_best = cost_rotation;
		}

		AABB_union(&box_union, &tree->nodes[tree->nodes[right].right].box, &tree->nodes[left].box);
		cost_rotation = cost_SAT(&box_union);
		if (cost_rotation < cost_best && cost_rotation < cost_original)
		{
			upper_rotation = left;
			best_rotation = tree->nodes[right].left;
			cost_best = cost_rotation;
		}
	}

	/* (2) apply rotation */
	if (best_rotation != DBVT_NO_NODE)
	{
		tree->nodes[best_rotation].parent = node;
		if (upper_rotation == left)
		{
			tree->nodes[upper_rotation].parent = right;
			tree->nodes[node].left = best_rotation;
			if (best_rotation == tree->nodes[right].left)
			{
				AABB_union(&tree->nodes[right].box, 
						&tree->nodes[tree->nodes[right].right].box,
					       	&tree->nodes[upper_rotation].box);
				tree->nodes[right].left = upper_rotation;
			}
			else
			{
				AABB_union(&tree->nodes[right].box, 
						&tree->nodes[tree->nodes[right].left].box,
					       	&tree->nodes[upper_rotation].box);
				tree->nodes[right].right = upper_rotation;
			}
			left = best_rotation;
		}
		else
		{
			tree->nodes[upper_rotation].parent = left;
			tree->nodes[node].right = best_rotation;
			if (best_rotation == tree->nodes[left].left)
			{
				AABB_union(&tree->nodes[left].box, 
						&tree->nodes[tree->nodes[left].right].box,
					       	&tree->nodes[upper_rotation].box);
				tree->nodes[left].left = upper_rotation;
			}
			else
			{
				AABB_union(&tree->nodes[left].box, 
						&tree->nodes[tree->nodes[left].left].box,
					       	&tree->nodes[upper_rotation].box);
				tree->nodes[left].right = upper_rotation;
			}
			right = best_rotation;
		}
	}

	/* (3) refit node's box */
	AABB_union(&tree->nodes[node].box, &tree->nodes[left].box, &tree->nodes[right].box);
}

i32 dbvt_insert(struct dbvt *tree, const i32 id, const struct AABB *box)
{
	tree->proxy_count += 1;
	const i32 index = dbvt_internal_alloc_node(tree, id, box);
	if (tree->root == DBVT_NO_NODE)
	{
		tree->root = index;
	}
	else
	{
		/**
		 * (1) Find best sibling using the minimum surface area hueristic + branch and bound algorithm.
		 * The idea is that every node in the hierarchy is a potential sibling to the new node, and we find
		 * the best suitable one by continuously delve deeper into the hierarchy as long as the some 
		 * new potential node gives a better cost than previous ones. We keep track of the best score and 
		 * the node achieving it. When no node achieves a better score, we are done and set the best scoring
		 * one as the sibling.
		 */
		i32 best_index = tree->root;
		f32 best_cost = FLT_MAX;
		f32 node_cost = 0.0f; 
	
		tree->cost_index[min_queue_insert(tree->cost_queue, node_cost)] = tree->root;
		assert(tree->cost_queue->elements[0].priority == 0.0f);

		i32 node;
		f32 inherited_cost, cost;
		struct AABB box_union;
	
		while(tree->cost_queue->num_elements > 0)
		{
			/* (i) Get cost of node */
			inherited_cost = tree->cost_queue->elements[0].priority; 
			node = tree->cost_index[min_queue_extract_min(tree->cost_queue)];
			AABB_union(&box_union, &tree->nodes[index].box, &tree->nodes[node].box);
			/* Inherited area cost + expanded node area cost */
			cost = inherited_cost + cost_SAT(&box_union);

			if (cost < best_cost)
			{
				best_cost = cost;
				best_index = node;
			}

			/**
			 * The current difference in area produced by the node's path + the new box's area
			 * is a lower bound on the node's descendants' cost. If the lower bound is not less
			 * than the best cost, we can prune the children's trees. Otherwise, we must still
			 * consider them as viable siblings. Their priorities become the increase in cost 
			 * to node's path when adding the new box (the inherited cost).
			 */
			cost -= cost_SAT(&tree->nodes[node].box);

			if (tree->nodes[node].left != DBVT_NO_NODE && cost + cost_SAT(&tree->nodes[index].box) < best_cost)
			{
				assert(tree->cost_queue->num_elements < COST_QUEUE_MAX-1);

				i32 j = min_queue_insert(tree->cost_queue, cost);
				tree->cost_index[j] = tree->nodes[node].left;

				j = min_queue_insert(tree->cost_queue, cost);
				tree->cost_index[j] = tree->nodes[node].right;
			}
		}

		/* (2) Setup a new parent node for the new node and its sibling */
		const i32 parent = dbvt_internal_alloc_node(tree, DBVT_NO_NODE, box);
		/* TODO: cleanup below */
		if (tree->nodes[best_index].parent != DBVT_NO_NODE)
		{
			if (tree->nodes[tree->nodes[best_index].parent].left == best_index)
			{
				tree->nodes[tree->nodes[best_index].parent].left = parent;
			}
			else
			{
				tree->nodes[tree->nodes[best_index].parent].right = parent;
			}
		}

		tree->nodes[parent].parent = tree->nodes[best_index].parent;
		tree->nodes[parent].left = best_index;
		tree->nodes[parent].right = index;
		tree->nodes[best_index].parent = parent;
		tree->nodes[index].parent = parent;
		AABB_union(&tree->nodes[parent].box, &tree->nodes[index].box, &tree->nodes[best_index].box);
		if (best_index == tree->root)
		{
			tree->root = parent;
		} 


		//printf("parent: %i,%i,%i,%i\n", tree->nodes[parent].parent, tree->nodes[parent].id, tree->nodes[parent].left, tree->nodes[parent].right);

		node = tree->nodes[parent].parent;
		/* (3) Traverse from grandparent of leaf, refitting and rotating node up to the root */
		while (node != DBVT_NO_NODE)
		{
			dbvt_internal_balance_node(tree, node);
			node = tree->nodes[node].parent;
		}
	}

	//dbvt_validate(tree);

	return index;
}

void dbvt_remove(struct dbvt *tree, const i32 index)
{
	tree->proxy_count -= 1;

	assert(tree->nodes[index].left  == DBVT_NO_NODE);
	assert(tree->nodes[index].right == DBVT_NO_NODE);

	i32 parent = tree->nodes[index].parent;
	if (parent == DBVT_NO_NODE)
	{
		tree->root = DBVT_NO_NODE;
		dbvt_internal_free_node(tree, index);
	}
	else
	{
		i32 sibling;
		if (tree->nodes[parent].left == index)
		{
			sibling = tree->nodes[parent].right;
		}
		else
		{
			sibling = tree->nodes[parent].left;
		}

		const i32 grand_parent = tree->nodes[parent].parent;
		tree->nodes[sibling].parent = grand_parent;

		dbvt_internal_free_node(tree, parent);
		dbvt_internal_free_node(tree, index);

		/* set new root */
		if (grand_parent == DBVT_NO_NODE)
		{
			tree->root = sibling;
		}
		else
		{
			if (tree->nodes[grand_parent].left == parent)
			{
				tree->nodes[grand_parent].left = sibling;
			}
			else
			{
				tree->nodes[grand_parent].right = sibling;
			}

			AABB_union(&tree->nodes[grand_parent].box, 
					&tree->nodes[tree->nodes[grand_parent].left].box,
					&tree->nodes[tree->nodes[grand_parent].right].box);
			parent = tree->nodes[grand_parent].parent;
			while (parent != DBVT_NO_NODE)
			{
				dbvt_internal_balance_node(tree, parent);
				parent = tree->nodes[parent].parent;
			}
		}
	}

	//dbvt_validate(tree);
}

i32 dbvt_internal_descend_a(const struct dbvt_node *a, const struct dbvt_node *b)
{
	return (b->left == DBVT_NO_NODE || (a->left != DBVT_NO_NODE && cost_SAT(&b->box) < cost_SAT(&a->box))) ? 1 : 0;
}

i32 dbvt_internal_push_subtree_overlap_pairs(struct arena *mem, const struct dbvt *tree, i32 subA, i32 subB, i32 stack[2*COST_QUEUE_MAX])
{
	assert(subA != DBVT_NO_NODE && subB != DBVT_NO_NODE);

	i32 overlap_count = 0;
	i32 overlap[2];
	i32 q = -1;

	while (1)
	{
		if (AABB_test(&tree->nodes[subA].box, &tree->nodes[subB].box))
		{
			if (tree->nodes[subA].left == DBVT_NO_NODE && tree->nodes[subB].left == DBVT_NO_NODE)
			{
				overlap_count += 1;
				overlap[0] = tree->nodes[subA].id;	
				overlap[1] = tree->nodes[subB].id;	
				arena_push_packed(mem, overlap, sizeof(overlap));
			}
			else
			{
				/* if a is larger than b, descend into a first  */
				if (dbvt_internal_descend_a(tree->nodes + subA, tree->nodes + subB))
				{
					stack[++q] = tree->nodes[subA].left;
					stack[++q] = subB;
					subA = tree->nodes[subA].right;
				}
				else
				{
					stack[++q] = tree->nodes[subB].left;
					stack[++q] = subA;
					subB = tree->nodes[subB].right;
				}

				assert(q < 2*COST_QUEUE_MAX);
				continue;
			}
		}

		if (q != -1)
		{
			subA = stack[q--];
			subB = stack[q--];
		}
		else
		{
			break;
		}
	}

	return overlap_count;
}

i32 dbvt_push_overlap_pairs(struct arena *mem, struct dbvt *tree)
{
	if (tree->proxy_count < 2) { return 0; }

	i32 overlap_count = 0;
	i32 a = tree->nodes[tree->root].left;
	i32 b = tree->nodes[tree->root].right;
	i32 q = -1;
	i32 stack1[2*COST_QUEUE_MAX];
	i32 stack2[2*COST_QUEUE_MAX];

	while (1)
	{
		overlap_count += dbvt_internal_push_subtree_overlap_pairs(mem, tree, a, b, stack2);

		if (tree->nodes[a].left != DBVT_NO_NODE)
		{
			stack1[++q] = tree->nodes[a].left;
			stack1[++q] = tree->nodes[a].right;	
			assert(q < 2*COST_QUEUE_MAX);
		}

		if (tree->nodes[b].left != DBVT_NO_NODE)
		{
			 a = tree->nodes[b].left;	
			 b = tree->nodes[b].right;	
			 continue;
		}

		if (q != -1)
		{
			a = stack1[q--];
			b = stack1[q--];
		}
		else
		{
			break;
		}
	}

	return overlap_count;
}

void dbvt_validate(struct dbvt *tree)
{
	i32 i = tree->root;
	i32 q = DBVT_NO_NODE;

	i32 node_count = 0;
	while (i != DBVT_NO_NODE)
	{
		node_count++;
		const i32 parent = tree->nodes[i].parent;
		if (parent != DBVT_NO_NODE)
		{
			const i32 parent_left = tree->nodes[parent].left;
			const i32 parent_right = tree->nodes[parent].right;
			assert(parent_left != parent_right);
			assert(parent_left == i || parent_right == i);
		}
	
		assert((tree->nodes[i].left == DBVT_NO_NODE && tree->nodes[i].right == DBVT_NO_NODE) 
				|| (tree->nodes[i].left != DBVT_NO_NODE && tree->nodes[i].right != DBVT_NO_NODE));

		if (tree->nodes[i].left != DBVT_NO_NODE)
		{
			tree->cost_index[++q] = tree->nodes[i].right;
			i = tree->nodes[i].left;
			assert(q < COST_QUEUE_MAX);
		}
		else if (q != DBVT_NO_NODE)
		{
			i = tree->cost_index[q--];
			assert(i != DBVT_NO_NODE);
		}
		else
		{
			i = DBVT_NO_NODE;
		}
	}

	assert(node_count == 2*tree->proxy_count - 1);
}

void dbvt_push_lines(struct drawbuffer *buf, struct dbvt *tree, const vec4 color)
{
	i32 i = tree->root;
	i32 q = DBVT_NO_NODE;

	while (i != DBVT_NO_NODE)
	{
		AABB_push_lines(buf, &tree->nodes[i].box, color);
		if (tree->nodes[i].left != DBVT_NO_NODE)
		{
			tree->cost_index[++q] = tree->nodes[i].right;
			assert(q < COST_QUEUE_MAX);
			i = tree->nodes[i].left;
		}
		else if (q != DBVT_NO_NODE)
		{
			i = tree->cost_index[q--];
		}
		else
		{
			i = DBVT_NO_NODE;
		}
	}
}
