#include <string.h>
#include <float.h>
#include "mmath.h"
#include "geometry.h"
#include "hash_index.h"
#include "relation_list.h"
#include "array_list.h"
#include "queue.h"

/**************************************************************/

i32 DCEL_half_edge_add(struct DCEL *dcel, struct arena *table_mem, const i32 origin, const i32 twin, const i32 face_ccw, const i32 next, const i32 prev)
{
	if (dcel->next_he == -1) 
	{ 
		dcel->next_he = dcel->num_he;
		arena_push_packed(table_mem, NULL, sizeof(struct DCEL_half_edge));
		dcel->he_table[dcel->next_he].next = -1;
		dcel->num_he += 1;
       	}

	const i32 tmp = dcel->he_table[dcel->next_he].next;
	dcel->he_table[dcel->next_he].he = dcel->next_he;
	dcel->he_table[dcel->next_he].origin = origin;
	dcel->he_table[dcel->next_he].twin = twin;
	dcel->he_table[dcel->next_he].face_ccw = face_ccw;
	dcel->he_table[dcel->next_he].next = next;
	dcel->he_table[dcel->next_he].prev = prev;
	dcel->next_he = tmp;

	return dcel->he_table[dcel->next_he].he;
}

i32 DCEL_half_edge_reserve(struct DCEL *dcel, struct arena *table_mem)
{
	if (dcel->next_he == -1) 
	{ 
		dcel->next_he = dcel->num_he;
		arena_push_packed(table_mem, NULL, sizeof(struct DCEL_half_edge));
		dcel->he_table[dcel->next_he].next = -1;
		dcel->num_he += 1;
	}

	const i32 he = dcel->next_he;
	dcel->next_he = dcel->he_table[he].next;

	return he;
}

void DCEL_half_edge_set(struct DCEL *dcel, const i32 he, const i32 origin, const i32 twin, const i32 face_ccw, const i32 next, const i32 prev)
{
	assert(he >= 0 && he < dcel->num_he);

	dcel->he_table[he].he = he;
	dcel->he_table[he].origin = origin;
	dcel->he_table[he].twin = twin;
	dcel->he_table[he].face_ccw = face_ccw;
	dcel->he_table[he].next = next;
	dcel->he_table[he].prev = prev;
}

i32 DCEL_face_add(struct DCEL *dcel, struct arena *face_mem, const i32 edge, const i32 unit)
{
	if (dcel->next_face == -1) 
	{ 
		dcel->next_face = dcel->num_faces;
		arena_push_packed(face_mem, NULL, sizeof(struct DCEL_face));
		dcel->num_faces += 1;
		dcel->faces[dcel->next_face].he_index = -1;
	}

	const i32 face = dcel->next_face;
	const i32 tmp = dcel->faces[face].he_index;
	dcel->faces[face].he_index = edge;
	dcel->faces[face].relation_unit = unit;
	dcel->next_face = tmp;

	return face;
}

void DCEL_half_edge_remove(struct DCEL *dcel, const i32 he)
{
	assert(he >= 0 && he < dcel->num_he);

	const i32 tmp = dcel->next_he;
	dcel->next_he = he;
	dcel->he_table[he].next = tmp;
	dcel->he_table[he].face_ccw = -1;
}

void DCEL_face_remove(struct DCEL *dcel, const i32 face)
{
	assert(face >= 0 && face < dcel->num_faces);

	const i32 tmp = dcel->next_face;
	dcel->next_face = face;
	dcel->faces[face].he_index = tmp;
	dcel->faces[face].relation_unit = -1;
}

i32 tetrahedron_indices(i32 indices[4], const vec3ptr v, const i32 v_count, const f32 tol)
{
	vec3 a, b, c, d;

	/* Find two points not to close to each other */
	for (i32 i = 1; i <= v_count; ++i)
	{
		/* all points are distance <= tol from first point */
		if (i == v_count) { return 0; }

		vec3_sub(a, v[i], v[0]);
		const f32 len = vec3_length(a);
		if (len > tol)
		{
			vec3_mul_constant(a, 1.0f / len);
			indices[1] = i;
			break;
		}
	}	

	/* Find non-collinear point */
	for (i32 i = indices[1] + 1; i <= v_count; ++i)
	{
		/* all points are collinear */
		if (i == v_count) { return 0; }

		vec3_sub(b, v[i], v[0]);
		const f32 dot = vec3_dot(a, b);
		
		if (sqrtf(vec3_length(b)*vec3_length(b) - dot*dot) > tol)
		{
			indices[2] = i;
			break;
		}
	}

	/* Find non-coplanar point */
	for (i32 i = indices[2] + 1; i <= v_count; ++i)
	{
		/* all points are coplanar */
		if (i == v_count) { return 0; }

		/* plane normal */
		vec3_cross(c, a, b);
		vec3_mul_constant(c, 1.0f / vec3_length(c));

		vec3_sub(d, v[i], v[0]);
		if (fabs(vec3_dot(d, c)) > tol)
		{
			indices[3] = i;
			break;
		}
	}

	return 1;
}

void triangle_CCW_relative_to(vec3 BCA[3], const vec3 p)
{
	vec3 AB, AC, AP, N;
	vec3_sub(AB, BCA[0], BCA[2]);
	vec3_sub(AC, BCA[1], BCA[2]);
	vec3_sub(AP, p, BCA[2]);
	vec3_cross(N, AB, AC);
	if (vec3_dot(N, AP) <= 0.0f)
	{
		vec3_copy(AB, BCA[0]);
		vec3_copy(BCA[0], BCA[1]);
		vec3_copy(BCA[1], AB);
	}
}

void triangle_CCW_relative_to_origin(vec3 BCA[3])
{
	vec3 AB, AC, AO, N;
	vec3_sub(AB, BCA[0], BCA[2]);
	vec3_sub(AC, BCA[1], BCA[2]);
	vec3_scale(AO, BCA[2], -1.0f);
	vec3_cross(N, AB, AC);
	if (vec3_dot(N, AO) <= 0.0f)
	{
		vec3_copy(AB, BCA[0]);
		vec3_copy(BCA[0], BCA[1]);
		vec3_copy(BCA[1], AB);
	}
}

static struct DCEL convex_hull_internal_setup_tetrahedron_DCEL(struct arena *table_mem, struct arena *face_mem, const i32 init_i[4], vec3ptr v)
{
	struct DCEL dcel =
	{ 
		.next_he = -1,
		.next_face = -1,
		.num_he = 0,
		.num_faces = 0,
	};

	dcel.faces = (struct DCEL_face *) face_mem->stack_ptr;
	dcel.he_table = (struct DCEL_half_edge *) table_mem->stack_ptr;
	DCEL_ALLOC_EDGES(&dcel, table_mem, 12);
	
	const vec3 inside =
	{
		0.25f * (v[init_i[0]][0] + v[init_i[1]][0] + v[init_i[2]][0] + v[init_i[3]][0]) - v[0][0],
		0.25f * (v[init_i[0]][1] + v[init_i[1]][1] + v[init_i[2]][1] + v[init_i[3]][1]) - v[0][1],
		0.25f * (v[init_i[0]][2] + v[init_i[1]][2] + v[init_i[2]][2] + v[init_i[3]][2]) - v[0][2],
	};

	vec3 a, b, c, d, cr;
	vec3_sub(a, v[0], v[0]);
	vec3_sub(b, v[init_i[1]], v[0]);
	vec3_sub(c, v[init_i[2]], v[0]);
	vec3_sub(d, v[init_i[3]], v[0]);
	vec3_cross(cr, b, c);

	/* CCW == inside gives negative dot product for any polygon on a convex polyhedron */
	if (vec3_dot(cr, inside) < 0.0f)
	{
		/* a -> b -> c */	
		i32 face = DCEL_face_add(&dcel, face_mem, 0, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[0], 3, face, 1, 2);
		DCEL_half_edge_add(&dcel, table_mem, init_i[1], 6, face, 2, 0);
		DCEL_half_edge_add(&dcel, table_mem, init_i[2], 9, face, 0, 1);

		/* b -> a -> d */
		face = DCEL_face_add(&dcel, face_mem, 3, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[1], 0,  face, 4, 5);
		DCEL_half_edge_add(&dcel, table_mem, init_i[0], 11, face, 5, 3);
		DCEL_half_edge_add(&dcel, table_mem, init_i[3], 7,  face, 3, 4);

		/* c -> b -> d */
		face = DCEL_face_add(&dcel, face_mem, 6, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[2], 1, face, 7, 8);
		DCEL_half_edge_add(&dcel, table_mem, init_i[1], 5, face, 8, 6);
		DCEL_half_edge_add(&dcel, table_mem, init_i[3], 10, face, 6, 7);

		/* a -> c -> d */
		face = DCEL_face_add(&dcel, face_mem, 9, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[0], 2, face, 10, 11);
		DCEL_half_edge_add(&dcel, table_mem, init_i[2], 8, face, 11, 9);
		DCEL_half_edge_add(&dcel, table_mem, init_i[3], 4, face, 9, 10);
	}
	else
	{
		/* a -> c -> b */
		i32 face = DCEL_face_add(&dcel, face_mem, 0, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[0], 3, face, 1, 2);
		DCEL_half_edge_add(&dcel, table_mem, init_i[2], 6, face, 2, 0);
		DCEL_half_edge_add(&dcel, table_mem, init_i[1], 9, face, 0, 1);
	
		/* c -> a -> d */
		face = DCEL_face_add(&dcel, face_mem, 3, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[2], 0,  face, 4, 5);
		DCEL_half_edge_add(&dcel, table_mem, init_i[0], 11, face, 5, 3);
		DCEL_half_edge_add(&dcel, table_mem, init_i[3], 7,  face, 3, 4);

		/* b -> c -> d */
		face = DCEL_face_add(&dcel, face_mem, 6, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[1], 1, face, 7, 8);
		DCEL_half_edge_add(&dcel, table_mem, init_i[2], 5, face, 8, 6);
		DCEL_half_edge_add(&dcel, table_mem, init_i[3], 10, face, 6, 7);

		/* a -> b -> d */
		face = DCEL_face_add(&dcel, face_mem, 9, -1);
		DCEL_half_edge_add(&dcel, table_mem, init_i[0], 2, face, 10, 11);
		DCEL_half_edge_add(&dcel, table_mem, init_i[1], 8, face, 11, 9);
		DCEL_half_edge_add(&dcel, table_mem, init_i[3], 4, face, 9, 10);
	}

	return dcel;
}

static struct relation_list convex_hull_internal_tetrahedron_conflicts(struct DCEL *dcel, struct arena *conflict_mem, const i32 *permutation, struct arena *stack, const vec3ptr v, const i32 v_count, const f32 EPSILON)
{
	struct relation_list conflict_graph = relation_list_init(conflict_mem, v_count);
	for (i32 i = 0; i < v_count; ++i) { conflict_graph.r[i].related_to = i; }

	vec3 b, c, cr;

	for (i32 face = 0; face < 4; ++face)
	{
		const i32 a_i = dcel->he_table[3*face + 0].origin;
		const i32 b_i = dcel->he_table[3*face + 1].origin;
		const i32 c_i = dcel->he_table[3*face + 2].origin;

		/* a -> b -> c, CCW, cross points outwards  */
		vec3_sub(b, v[b_i], v[a_i]);
		vec3_sub(c, v[c_i], v[a_i]);
		vec3_cross(cr, b, c);
		vec3_mul_constant(cr, 1.0f / vec3_length(cr));

		i32 *conflicts = (i32 *) stack->stack_ptr;
		i32 num_conflicts = 0;
		for (i32 v_i = 4; v_i < v_count; ++v_i)
		{
			const i32 index = permutation[v_i];
			vec3_sub(b, v[index], v[a_i]);
			/* If point is "in front" of face, we have a conflict */
			if (vec3_dot(cr, b) > EPSILON)
			{
				arena_push_packed(stack, &v_i, sizeof(i32));
				num_conflicts += 1;
			}
		}

		if (num_conflicts)  
		{ 
			dcel->faces[face].relation_unit = relation_list_add_relation_unit(&conflict_graph, face, conflicts, num_conflicts); 
		}
		else
		{
			dcel->faces[face].relation_unit = relation_list_add_relation_unit_empty(&conflict_graph, face); 
		}
			
		assert(face == conflict_graph.r[dcel->faces[face].relation_unit].related_to);
		
		/* pop conflict indices */
		arena_pop_packed(stack, num_conflicts * sizeof(i32));
	}

	return conflict_graph;
}

static void convex_hull_internal_random_permutation(i32 *permutation, const i32 indices[4], const i32 num_vs)
{
	for (i32 i = 0; i < num_vs; ++i) { permutation[i] = i; }
	permutation[0] = indices[0];
	permutation[1] = indices[1];
	permutation[2] = indices[2];
	permutation[3] = indices[3];
	permutation[indices[0]] = 0;
	permutation[indices[1]] = 1;
	permutation[indices[2]] = 2;
	permutation[indices[3]] = 3;

	for (i32 i = 4; i < num_vs; ++i)
	{
		const i32 r = (i32) gen_continuous_uniform_f(i, ((f32) num_vs) - 0.0001f);
		const i32 tmp = permutation[i];
		permutation[i] = permutation[r];
		permutation[r] = tmp;
	}
}

/* returns new face relation_unit, MAKE SURE ADDED VERTICES ARRAY contain enough memory  */
void convex_hull_internal_add_possible_conflicts(const i32 *permutation, struct relation_list *conflict_graph, i32 *added_vertices, struct arena *mem, const i32 face_unit, const i32 num_possible_conflicts, const i32 *possible_conflicts, const vec3 origin, const vec3 normal, const vec3ptr v, const f32 EPSILON)
{
	vec3 p;
	i32 num_added = 0;
	for (i32 i = 0; i < num_possible_conflicts; ++i)
	{
		if (added_vertices[possible_conflicts[i]] == -1)
		{
			vec3_sub(p, v[permutation[possible_conflicts[i]]], origin);
			const f32 dot = vec3_dot(normal, p);

			if (dot > EPSILON)
			{
				num_added += 1;
				arena_push_packed(mem, &possible_conflicts[i], sizeof(i32));
				added_vertices[possible_conflicts[i]] = possible_conflicts[i];
				relation_list_add_to_relation_unit(conflict_graph, face_unit, possible_conflicts[i]);
				relation_list_add_to_relation_unit(conflict_graph, possible_conflicts[i], face_unit);
			}
		}
	}

	/* reset array */
	arena_pop_packed(mem, num_added * sizeof(i32));
	i32 *ptr = (i32 *) mem->stack_ptr;
	for (i32 i = 0; i < num_added; ++i)
	{
		added_vertices[ptr[i]] = -1;
	}
}

static i32 convex_hull_internal_push_conflict_faces(struct DCEL *dcel, const struct relation_list *conflict_graph, struct hash_index *horizon_map, struct arena *mem_1, struct arena *mem_2, const i32 v)
{
	i32 num_conflict_faces = 0;
	i32 *edges_to_remove = (i32 *) arena_push_packed(mem_2, NULL, sizeof(i32));
	*edges_to_remove = 0;

	/* Delete all faces that conflict with the vertex */
	for (i32 i = conflict_graph->r[v].next; i != -1; i = conflict_graph->r[i].next)
	{
		/* (5) Keep track of visibility horizion */
		const i32 face_unit = conflict_graph->r[i].related_to;
		const i32 conflict_face = conflict_graph->r[face_unit].related_to;
		const i32 start = dcel->faces[conflict_face].he_index;

		i32 edge = start;
		while (1)
		{
			/* add / remove edge (non removed edges will become horizon */
			const i32 key = hash_generate_key_int(dcel->he_table[edge].origin);
			const i32 twin = dcel->he_table[edge].twin;
			const i32 key_twin = hash_generate_key_int(dcel->he_table[twin].origin);
			i32 twin_in = 0;
			for (i32 index = hash_first(horizon_map, key_twin); index != -1; index = hash_next(horizon_map, index))
			{
				if (index == twin)
				{
					twin_in = 1;
					break;
				}
			}

			const i32 tmp = dcel->he_table[edge].next;
			if (twin_in) 
			{ 
				hash_remove(horizon_map, key_twin, twin); 
				*edges_to_remove += 2;
				arena_push_packed(mem_2, &twin, sizeof(i32));
				arena_push_packed(mem_2, &edge, sizeof(i32));
			}
			else 
			{ 
				hash_add(horizon_map, key, edge); 
			}

			edge = tmp;
			if (edge == start) 
			{ 
				break;
		       	}
		}

		num_conflict_faces += 1;
		arena_push_packed(mem_1, &conflict_face, sizeof(i32));
	}
	
	return num_conflict_faces;
}

static void convex_hull_internal_DCEL_add_coplanar(struct DCEL *dcel, const i32 horizon_edge_1, const i32 horizon_edge_2, const i32 last_edge, const i32 prev_edge)
{
	const i32 twin_1 = dcel->he_table[horizon_edge_1].twin;
	const i32 twin_2 = dcel->he_table[horizon_edge_2].twin;
	assert(dcel->he_table[twin_1].face_ccw == dcel->he_table[twin_2].face_ccw);

	/* connect new face edges and planar neighbor face edges  */
	dcel->he_table[last_edge].next = dcel->he_table[twin_1].next;
	dcel->he_table[prev_edge].prev = dcel->he_table[twin_2].prev;

	dcel->he_table[dcel->he_table[twin_1].next].prev = last_edge; 
	dcel->he_table[dcel->he_table[twin_2].prev].next = prev_edge;
}

//i32 convex_hull_cs(struct arena *table_mem, struct arena *face_mem, struct arena *conflict_mem, struct arena *mem_4, const f32 *vs, const i32 num_vs, const f32 EPSILON)
//{
//	if (num_vs < 4) { return 0; }	
//
//	/* (1) Get inital points for tetrahedron */
//	i32 init_i[4] = { 0 };
//	if (convex_hull_internal_initial_tetrahedron_indices(init_i, vs, num_vs, EPSILON) == 0) { return 0; }
//
//	/* (2) permutation - Random permutation of remaining points, do this at start so we can stack edges afterwards  */
//	const u64 permutation_size = sizeof(i32) * num_vs;
//	i32 *permutation = (i32 *) arena_push(table_mem, NULL, permutation_size);
//	convex_hull_internal_random_permutation(permutation, init_i, num_vs);
//	
//	/* (3) initiate DCEL from points */
//	struct DCEL dcel = convex_hull_internal_setup_tetrahedron_DCEL(table_mem, face_mem, init_i, vs);
//	
//	/* (4) setup conflict graph */
//	struct relation_list conflict_graph = convex_hull_internal_tetrahedron_conflicts(&dcel, conflict_mem, permutation, table_mem, vs, num_vs, EPSILON);
//	
//
//	/* Cleanup */
//	relation_list_free(&conflict_graph);
//	arena_pop_packed(table_mem, dcel.num_he * sizeof(struct DCEL_half_edge));
//	arena_pop_packed(face_mem, dcel.num_faces * sizeof(struct DCEL_face));
//	/* pop random packed permutation from stack */
//	arena_pop(table_mem, permutation_size);
//	return 1;
//}

#ifdef MGL_DEBUG
/**
 * POLYGON_MODE == 0 (triangles)
 * POLYGON_MODE == 1 (lines)
 */
void convex_hull_cs_step_draw_internal_push_data(const struct DCEL *dcel, struct drawbuffer *d_buf, const f32 *vs, const vec4 color, const i32 POLYGON_MODE)
{
	vec3 normal, a, b, c;
	vec3i32 indices;
	i32 m_i = d_buf->next_index;
	const i32 index = d_buf->index;
	d_buf->index += 1;
	for (i32 i = 0; i < dcel->num_faces; ++i)
	{
		const struct DCEL_face *f = &dcel->faces[i];
		/* It is a face on the convex hull */
		if (f->relation_unit != -1)
		{
			if (POLYGON_MODE == 0)
			{
				/* get normal */
				struct DCEL_half_edge *he = &dcel->he_table[f->he_index];
				vec3_copy(a, vs + 3*he->origin);
				he = &dcel->he_table[he->next];
				vec3_sub(b, vs + 3*he->origin, a);
				he = &dcel->he_table[he->next];
				vec3_sub(c, vs + 3*he->origin, a);
				vec3_cross(normal, b, c);
				vec3_mul_constant(normal, 1.0f / vec3_length(normal));

				/* Push first triangle */
				he = &dcel->he_table[f->he_index];
				vec3_copy(a, vs + 3*he->origin);
				he = &dcel->he_table[he->next];
				vec3_copy(b, vs + 3*he->origin);
				he = &dcel->he_table[he->next];
				vec3_copy(c, vs + 3*he->origin);

				arena_push_packed(&d_buf->v_buf, a, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
				arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
				arena_push_packed(&d_buf->v_buf, b, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
				arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
				arena_push_packed(&d_buf->v_buf, c, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
				arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
				vec3i32_set(indices, m_i + 0, m_i + 1, m_i + 2);
				arena_push_packed(&d_buf->i_buf, indices, sizeof(vec3i32));

				i32 offset = 3;

				/* If more edges in polygon, loop through */
				/* 0->2->3, 0->3->4, ... */
				for (i32 j = 2; he->next != f->he_index; ++j)
				{
					he = &dcel->he_table[he->next];
					vec3_copy(a, vs + 3*he->origin);
					arena_push_packed(&d_buf->v_buf, a, sizeof(vec3));
					arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
					arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
					arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
					vec3i32_set(indices, m_i + 0, m_i + j, m_i + j + 1);
					arena_push_packed(&d_buf->i_buf, indices, sizeof(vec3i32));
					offset += 1;
				}

				m_i += offset;
			}
			else if (POLYGON_MODE == 1)
			{
				struct DCEL_half_edge *he = &dcel->he_table[f->he_index];
				const i32 first = he->he;
				
				/* get normal */
				vec3_copy(a, vs + 3*he->origin);
				he = &dcel->he_table[he->next];
				vec3_sub(b, vs + 3*he->origin, a);
				he = &dcel->he_table[he->next];
				vec3_sub(c, vs + 3*he->origin, a);
				vec3_cross(normal, b, c);
				vec3_mul_constant(normal, 1.0f / vec3_length(normal));

				/* Push first line */
				he = &dcel->he_table[first];
				vec3_copy(a, vs + 3*he->origin);
				he = &dcel->he_table[he->next];
				vec3_copy(b, vs + 3*he->origin);
				arena_push_packed(&d_buf->v_buf, a, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
				arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
				arena_push_packed(&d_buf->v_buf, b, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
				arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
				arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
				i32 indices[] = { m_i, m_i + 1 };
				arena_push_packed(&d_buf->i_buf, indices, 2*sizeof(i32));
				m_i += 2;

				while (he->he != first)
				{
					vec3_copy(a, vs + 3*he->origin);
					he = &dcel->he_table[he->next];
					vec3_copy(b, vs + 3*he->origin);
					arena_push_packed(&d_buf->v_buf, a, sizeof(vec3));
					arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
					arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
					arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
					arena_push_packed(&d_buf->v_buf, b, sizeof(vec3));
					arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
					arena_push_packed(&d_buf->v_buf, normal, sizeof(vec3));
					arena_push_packed(&d_buf->v_buf, &index, sizeof(i32));
					indices[0] += 2;
					indices[1] += 2;
					arena_push_packed(&d_buf->i_buf, indices, 2*sizeof(i32));
					m_i += 2;
				}
			}
		}
	}	

	/* update drawbuffer max_used */
	d_buf->next_index = m_i;
}

struct tri_mesh tri_mesh_empty(void)
{
	struct tri_mesh mesh =
	{
		.v = NULL,
		.v_count = 0,
		.tri = NULL,
		.tri_count = 0,
	};

	return mesh;
}

struct tri_mesh convex_hull_construct(struct arena *mem, struct arena *table_mem, struct arena *face_mem, struct arena *conflict_mem, struct arena *mem_4, struct arena *mem_5, const vec3ptr v, const u32 v_count, const f32 EPSILON)
{
	if (v_count < 4) { return tri_mesh_empty(); }	

	/* (1) Get inital points for tetrahedron */
	i32 init_i[4] = { 0 };
	if (tetrahedron_indices(init_i, v, v_count, EPSILON) == 0) { return tri_mesh_empty(); }

	/* (2) permutation - Random permutation of remaining points */
	const u64 permutation_size = sizeof(i32) * v_count;
	i32 *permutation = (i32 *) arena_push(table_mem, NULL, permutation_size);
	convex_hull_internal_random_permutation(permutation, init_i, v_count);
	
	/* (3) initiate DCEL from points */
	struct DCEL dcel = convex_hull_internal_setup_tetrahedron_DCEL(table_mem, face_mem, init_i, v);


	/* (4) setup conflict graph */
	struct relation_list conflict_graph = convex_hull_internal_tetrahedron_conflicts(&dcel, conflict_mem, permutation, table_mem, v, v_count, EPSILON);

	/**
	 * vertex -> edge map. We iterate over all conflicting faces for a point, and for each edge
	 * in a face, add it to the map if it is not currentl in the map. If it is in the map, remove it.
	 * In the end, we will only have the horizon edges left. For degenerate coplanar faces for newly
	 * created faces in the point's iteration, we can check the horizon edges' twins.
	 */
	struct hash_index *horizon_map = hash_new(mem_4, power_of_two_ceil(v_count), 1024);

	/* iteratetively solve and add conflicts until no vertices left */
	const i32 n = v_count;

	for (i32 i = 4; i < n; ++i)
	{
		/* Some face is conflicting with point */
		if (conflict_graph.r[i].next != -1)
		{
			/* (5) Get horizon edges (unsorted) and push all conflicting faces to pointer, delete all DCEL edges not on the horizon  */
			i32 *conflict_faces = (i32 *) mem_5->stack_ptr;
			i32 *edges_to_remove = (i32 *) mem_4->stack_ptr;
			i32 num_conflict_faces = convex_hull_internal_push_conflict_faces(&dcel, &conflict_graph, horizon_map, mem_5, mem_4, i);
			
			/* (6) sort the edges (push to mem_4) */
			i32 start;
			for (i32 k = 0; (start = hash_first(horizon_map, k)) == -1; ++k);
			const i32 *horizon_edges = (i32 *) mem_4->stack_ptr;
			arena_push_packed(mem_4, &start, sizeof(i32));
			i32 edge = start;
		        i32 num_edges = 1;
			while (1)
			{
				const i32 next = dcel.he_table[edge].next;
				const i32 key = hash_generate_key_int(dcel.he_table[next].origin);
				/* all keys should have 0 or 1 values at this point */
				edge = hash_first(horizon_map, key);
				if (edge == start) { break; }
				arena_push_packed(mem_4, &edge, sizeof(i32));
				num_edges += 1;	
			}
			hash_clear(horizon_map);

			/**
			 * (7) Remove all conflicts with face and face itself from conflict graph but record any
			 * old vertex conflicts at boundary edge first. New faces will only conflict with points
			 * in the given unions. 
			 */
			i32 *union_lens = (i32 *) mem_4->stack_ptr;
			arena_push_packed(mem_4, NULL, num_edges * sizeof(i32));
			for (i32 z = 0; z < num_edges; ++z)
			{
				const i32 f1 = dcel.he_table[horizon_edges[z]].face_ccw;
				const i32 f2 = dcel.he_table[dcel.he_table[horizon_edges[z]].twin].face_ccw;
				union_lens[z] = relation_list_push_union(mem_4, &conflict_graph, dcel.faces[f1].relation_unit, dcel.faces[f2].relation_unit);
			}
		
			
			/* (8) Add new faces */
			/* (9) Fix coplanar degeneracies */ 
			/* (10) find conflicts to i<j<n points and add to graph  */
			i32 j = 0, k = 1, upper = num_edges - 1; 
			for (; 0 < upper && dcel.he_table[dcel.he_table[horizon_edges[upper]].twin].face_ccw == dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw; --upper);
			for (; k <= upper && dcel.he_table[dcel.he_table[horizon_edges[k]].twin].face_ccw == dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw; ++k);
			j = (upper + 1) % num_edges;

			i32 len_offset = 0;
			for (i32 t = 0; t < j; ++t)
			{
				len_offset += union_lens[t]; 
			}

			i32 horizon_edges_to_remove = 0;
			
			i32 unit, face, prev_edge = -1, last_edge = -1;

			i32 b_i = dcel.he_table[dcel.he_table[dcel.he_table[dcel.he_table[horizon_edges[j]].twin].next].next].origin;
			vec3 b, c, normal, origin, new;
			vec3_copy(origin, v[dcel.he_table[horizon_edges[j]].origin]);
			vec3_sub(b, v[b_i], origin);
			vec3_sub(c, v[dcel.he_table[horizon_edges[(j+1) % num_edges]].origin], origin);
			vec3_cross(normal, b, c);
			vec3_mul_constant(normal, 1.0f / vec3_length(normal));
			vec3_sub(new, v[permutation[i]], origin);
			/*coplanar if neighbor is on fat plane of new face */
			if (fabs(vec3_dot(new, normal)) < EPSILON)
			{
				//unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
				//face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
				//conflict_graph.r[unit].related_to = face;
				face = dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw;
				unit = dcel.faces[face].relation_unit;

				last_edge = DCEL_half_edge_reserve(&dcel, table_mem);
				prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

				DCEL_half_edge_set(&dcel, prev_edge,
					dcel.he_table[horizon_edges[k % num_edges]].origin,
					-1,
					face,
					last_edge,
					-1);

				DCEL_half_edge_set(&dcel, last_edge,
					permutation[i],
					-1,
					face,
					-1,
					prev_edge);

				dcel.faces[face].he_index = last_edge;

				convex_hull_internal_DCEL_add_coplanar(&dcel, horizon_edges[j], horizon_edges[k-1], last_edge, prev_edge);
			
				i32 edge = dcel.he_table[horizon_edges[j]].twin;
				while (edge != dcel.he_table[horizon_edges[k-1]].twin) 
				{
					horizon_edges_to_remove += 2;
					const i32 tmp = dcel.he_table[edge].prev;
					arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
					arena_push_packed(mem_5, &edge, sizeof(i32));
					edge = tmp;
				}
				horizon_edges_to_remove += 2;
				arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
				arena_push_packed(mem_5, &edge, sizeof(i32));

				len_offset = 0;
				for (j = 0; j < k; ++j) 
				{ 
					len_offset += union_lens[j]; 
				}
			}
			else
			{
				for (; j != k; ) 
				{ 
					unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
					face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
					conflict_graph.r[unit].related_to = face;

					const i32 tmp = prev_edge;
					const i32 last_edge_in_polygon = DCEL_half_edge_reserve(&dcel, table_mem);
					if (j == ((upper + 1) % num_edges))
					{
						last_edge = last_edge_in_polygon;
					}
					prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

					DCEL_half_edge_set(&dcel, prev_edge,
						dcel.he_table[horizon_edges[(j+1) % num_edges]].origin,
						-1,
						face,
						last_edge_in_polygon,
						horizon_edges[j]);

					DCEL_half_edge_set(&dcel, last_edge_in_polygon,
						permutation[i],
						tmp,
						face,
						horizon_edges[j],
						prev_edge);
					if (tmp != -1) { dcel.he_table[tmp].twin = last_edge_in_polygon; }
					
					vec3_copy(origin, v[dcel.he_table[horizon_edges[j]].origin]);
					vec3_sub(b, v[dcel.he_table[horizon_edges[(j+1) % num_edges]].origin], origin);
					vec3_sub(c, v[permutation[i]], origin);
					vec3_cross(normal, b, c);
					vec3_mul_constant(normal, 1.0f / vec3_length(normal));
					convex_hull_internal_add_possible_conflicts(permutation, &conflict_graph, hash_index_hash_ptr(horizon_map), mem_5, unit, union_lens[j], union_lens + num_edges + len_offset, origin, normal, v, EPSILON);
				
					dcel.he_table[horizon_edges[j]].prev = last_edge_in_polygon;
					dcel.he_table[horizon_edges[j]].next = prev_edge;
					dcel.he_table[horizon_edges[j]].face_ccw = face;

					len_offset += union_lens[j]; 
					j = (j+1) % num_edges;
					if (j == 0) { len_offset = 0; }
				}
			}

			for (; k < upper+1;)
			{
				for (k += 1; k < upper+1 && dcel.he_table[dcel.he_table[horizon_edges[k]].twin].face_ccw == dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw; k += 1);

				b_i = dcel.he_table[dcel.he_table[dcel.he_table[dcel.he_table[horizon_edges[j]].twin].next].next].origin;
				vec3_copy(origin, v[dcel.he_table[horizon_edges[j]].origin]);
				vec3_sub(b, v[b_i], origin);
				vec3_sub(c, v[dcel.he_table[horizon_edges[(j+1) % num_edges]].origin], origin);
				vec3_cross(normal, b, c);
				vec3_mul_constant(normal, 1.0f / vec3_length(normal));
				vec3_sub(new, v[permutation[i]], origin);
				/*coplanar if neighbor is on fat plane of new face */
				if (fabs(vec3_dot(new, normal)) < EPSILON)
				{
					//unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
					//face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
					//conflict_graph.r[unit].related_to = face;

					face = dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw;
					unit = dcel.faces[face].relation_unit;

					const i32 tmp = prev_edge;
					const i32 last_edge_in_polygon = DCEL_half_edge_reserve(&dcel, table_mem);
					if (j == ((upper + 1) % num_edges))
					{
						last_edge = last_edge_in_polygon;
					}

					prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

					DCEL_half_edge_set(&dcel, prev_edge,
						dcel.he_table[horizon_edges[k % num_edges]].origin,
						-1,
						face,
						last_edge_in_polygon,
						-1);

					DCEL_half_edge_set(&dcel, last_edge_in_polygon,
						permutation[i],
						tmp,
						face,
						-1,
						prev_edge);
					dcel.he_table[tmp].twin = last_edge_in_polygon;

					dcel.faces[face].he_index = last_edge_in_polygon;

					convex_hull_internal_DCEL_add_coplanar(&dcel, horizon_edges[j], horizon_edges[k-1], last_edge_in_polygon, prev_edge);

					i32 edge = dcel.he_table[horizon_edges[j]].twin;
					while (edge != dcel.he_table[horizon_edges[k-1]].twin) 
					{
						horizon_edges_to_remove += 2;
						const i32 tmp = dcel.he_table[edge].prev;
						arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
						arena_push_packed(mem_5, &edge, sizeof(i32));
						edge = tmp;
					}
					horizon_edges_to_remove += 2;
					arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
					arena_push_packed(mem_5, &edge, sizeof(i32));

					for (; j < k; ++j) 
					{ 
						len_offset += union_lens[j]; 
					}
				}
				else
				{
					for (; j < k; ++j) 
					{ 
						unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
						face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
						conflict_graph.r[unit].related_to = face;

						const i32 tmp = prev_edge;
						const i32 last_edge_in_polygon = DCEL_half_edge_reserve(&dcel, table_mem);
						prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

						DCEL_half_edge_set(&dcel, prev_edge,
							dcel.he_table[horizon_edges[(j+1) % num_edges]].origin,
							-1,
							face,
							last_edge_in_polygon,
							horizon_edges[j]);

						DCEL_half_edge_set(&dcel, last_edge_in_polygon,
							permutation[i],
							tmp,
							face,
							horizon_edges[j],
							prev_edge);
						dcel.he_table[tmp].twin = last_edge_in_polygon;

						vec3_copy(origin, v[dcel.he_table[horizon_edges[j]].origin]);
						vec3_sub(b, v[dcel.he_table[horizon_edges[(j+1) % num_edges]].origin], origin);
						vec3_sub(c, v[permutation[i]], origin);
						vec3_cross(normal, b, c);
						vec3_mul_constant(normal, 1.0f / vec3_length(normal));
						convex_hull_internal_add_possible_conflicts(permutation, &conflict_graph, hash_index_hash_ptr(horizon_map), mem_5, unit, union_lens[j], union_lens + num_edges + len_offset, origin, normal, v, EPSILON);

						dcel.he_table[horizon_edges[j]].prev = last_edge_in_polygon;
						dcel.he_table[horizon_edges[j]].next = prev_edge;
						dcel.he_table[horizon_edges[j]].face_ccw = face;

						len_offset += union_lens[j]; 
					}
				}
			}

			assert(last_edge != -1);
			dcel.he_table[last_edge].twin = prev_edge;
			dcel.he_table[prev_edge].twin = last_edge;
			
			arena_pop_packed(mem_5, horizon_edges_to_remove * sizeof(i32));
			i32 *ptr = (i32 *) mem_5->stack_ptr;
			for (i32 z = 0; z < horizon_edges_to_remove; ++z)
			{
				DCEL_half_edge_remove(&dcel, ptr[z]);
			}

			/* cleanup mem_5 */
			for (i32 z = 0; z < num_conflict_faces; z++)
			{
				/* only free once if repeated */
				if (dcel.faces[conflict_faces[z]].relation_unit != -1)
				{
					relation_list_remove_relation_unit(&conflict_graph, dcel.faces[conflict_faces[z]].relation_unit);
					DCEL_face_remove(&dcel, conflict_faces[z]);
				}
			}
			arena_pop_packed(mem_5, num_conflict_faces * sizeof(i32));

			/* cleanup mem_4 */
			for (i32 z = 0; z < num_edges; ++z)
			{
				arena_pop_packed(mem_4, union_lens[z] * sizeof(i32));
			}
			for (i32 z = 0; z < edges_to_remove[0]; ++z)
			{
				DCEL_half_edge_remove(&dcel, edges_to_remove[z+1]);
			}
			arena_pop_packed(mem_4, (edges_to_remove[0]+1) * sizeof(i32));
			arena_pop_packed(mem_4, num_edges * sizeof(i32));
			arena_pop_packed(mem_4, num_edges * sizeof(i32));
		}
		relation_list_remove_relation_unit(&conflict_graph, i);
	}

	struct tri_mesh mesh =
	{
		.v = arena_push(mem, v, v_count * sizeof(vec3)),
		.v_count = v_count,
		.tri_count = 0,
	};

	mesh.tri = (vec3u32ptr) mem->stack_ptr;
	for (i32 i = 0; i < dcel.num_faces; ++i)
	{
		const struct DCEL_face *f = &dcel.faces[i];
		/* It is a face on the convex mesh */
		if (f->relation_unit != -1)
		{
			struct DCEL_half_edge *he = &dcel.he_table[f->he_index];

			vec3u32 tri;
			tri[0] = he->origin;
			he = &dcel.he_table[he->next];
			tri[1] = he->origin;
			he = &dcel.he_table[he->next];
			tri[2] = he->origin;

			arena_push_packed(mem, tri, sizeof(vec3u32));
			mesh.tri_count += 1;

			/* 0->2->3, 0->3->4, ... */
			while (he->next != f->he_index)
			{
				he = &dcel.he_table[he->next];
				tri[1] = tri[2];
				tri[2] = he->origin;
				arena_push_packed(mem, tri, sizeof(vec3u32));
				mesh.tri_count += 1;
			}
		}
	}	
	
	/* Cleanup */
	hash_free(horizon_map);
	relation_list_free(&conflict_graph);
	arena_pop_packed(table_mem, dcel.num_he * sizeof(struct DCEL_half_edge));
	arena_pop_packed(face_mem, dcel.num_faces * sizeof(struct DCEL_face));
	arena_pop(table_mem, permutation_size);

	return mesh;
}

i32 convex_hull_cs_step_draw(struct arena *table_mem, struct arena *face_mem, struct arena *conflict_mem, struct arena *mem_4, struct arena *mem_5, const f32 *vs, const i32 num_vs, const f32 EPSILON, const i32 num_steps, const u32 seed, struct drawbuffer *d_buf, const vec4 color, const i32 polygon_mode)
{
	mersenne_twister_init(seed);
	if (num_vs < 4) { return 0; }	

	/* (1) Get inital points for tetrahedron */
	i32 init_i[4] = { 0 };
	if (tetrahedron_indices(init_i, (vec3ptr) vs, num_vs, EPSILON) == 0) { return 0; }

	/* (2) permutation - Random permutation of remaining points */
	const u64 permutation_size = sizeof(i32) * num_vs;
	i32 *permutation = (i32 *) arena_push(table_mem, NULL, permutation_size);
	convex_hull_internal_random_permutation(permutation, init_i, num_vs);
	
	/* (3) initiate DCEL from points */
	struct DCEL dcel = convex_hull_internal_setup_tetrahedron_DCEL(table_mem, face_mem, init_i, (vec3ptr) vs);


	/* (4) setup conflict graph */
	struct relation_list conflict_graph = convex_hull_internal_tetrahedron_conflicts(&dcel, conflict_mem, permutation, table_mem, (vec3ptr) vs, num_vs, EPSILON);

	/**
	 * vertex -> edge map. We iterate over all conflicting faces for a point, and for each edge
	 * in a face, add it to the map if it is not currentl in the map. If it is in the map, remove it.
	 * In the end, we will only have the horizon edges left. For degenerate coplanar faces for newly
	 * created faces in the point's iteration, we can check the horizon edges' twins.
	 */
	struct hash_index *horizon_map = hash_new(mem_4, power_of_two_ceil(num_vs), 1024);

	/* iteratetively solve and add conflicts until no vertices left */
	const i32 n = (4 + num_steps < num_vs) ?  4 + num_steps : num_vs;

	for (i32 i = 4; i < n; ++i)
	{
		/* Some face is conflicting with point */
		if (conflict_graph.r[i].next != -1)
		{
			/* (5) Get horizon edges (unsorted) and push all conflicting faces to pointer, delete all DCEL edges not on the horizon  */
			i32 *conflict_faces = (i32 *) mem_5->stack_ptr;
			i32 *edges_to_remove = (i32 *) mem_4->stack_ptr;
			i32 num_conflict_faces = convex_hull_internal_push_conflict_faces(&dcel, &conflict_graph, horizon_map, mem_5, mem_4, i);
			
			/* (6) sort the edges (push to mem_4) */
			i32 start;
			for (i32 k = 0; (start = hash_first(horizon_map, k)) == -1; ++k);
			const i32 *horizon_edges = (i32 *) mem_4->stack_ptr;
			arena_push_packed(mem_4, &start, sizeof(i32));
			i32 edge = start;
		        i32 num_edges = 1;
			while (1)
			{
				const i32 next = dcel.he_table[edge].next;
				const i32 key = hash_generate_key_int(dcel.he_table[next].origin);
				/* all keys should have 0 or 1 values at this point */
				edge = hash_first(horizon_map, key);
				if (edge == start) { break; }
				arena_push_packed(mem_4, &edge, sizeof(i32));
				num_edges += 1;	
			}
			hash_clear(horizon_map);

			/**
			 * (7) Remove all conflicts with face and face itself from conflict graph but record any
			 * old vertex conflicts at boundary edge first. New faces will only conflict with points
			 * in the given unions. 
			 */
			i32 *union_lens = (i32 *) mem_4->stack_ptr;
			arena_push_packed(mem_4, NULL, num_edges * sizeof(i32));
			for (i32 z = 0; z < num_edges; ++z)
			{
				const i32 f1 = dcel.he_table[horizon_edges[z]].face_ccw;
				const i32 f2 = dcel.he_table[dcel.he_table[horizon_edges[z]].twin].face_ccw;
				union_lens[z] = relation_list_push_union(mem_4, &conflict_graph, dcel.faces[f1].relation_unit, dcel.faces[f2].relation_unit);
			}
		
			
			/* (8) Add new faces */
			/* (9) Fix coplanar degeneracies */ 
			/* (10) find conflicts to i<j<n points and add to graph  */
			i32 j = 0, k = 1, upper = num_edges - 1; 
			for (; 0 < upper && dcel.he_table[dcel.he_table[horizon_edges[upper]].twin].face_ccw == dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw; --upper);
			for (; k <= upper && dcel.he_table[dcel.he_table[horizon_edges[k]].twin].face_ccw == dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw; ++k);
			j = (upper + 1) % num_edges;

			i32 len_offset = 0;
			for (i32 t = 0; t < j; ++t)
			{
				len_offset += union_lens[t]; 
			}

			i32 horizon_edges_to_remove = 0;
			
			i32 unit, face, prev_edge = -1, last_edge = -1;

			i32 b_i = dcel.he_table[dcel.he_table[dcel.he_table[dcel.he_table[horizon_edges[j]].twin].next].next].origin;
			vec3 b, c, normal, origin, new;
			vec3_copy(origin, vs + 3*dcel.he_table[horizon_edges[j]].origin);
			vec3_sub(b, vs + 3*b_i, origin);
			vec3_sub(c, vs + 3*dcel.he_table[horizon_edges[(j+1) % num_edges]].origin, origin);
			vec3_cross(normal, b, c);
			vec3_mul_constant(normal, 1.0f / vec3_length(normal));
			vec3_sub(new, vs + 3*permutation[i], origin);
			/*coplanar if neighbor is on fat plane of new face */
			if (fabs(vec3_dot(new, normal)) < EPSILON)
			{
				//unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
				//face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
				//conflict_graph.r[unit].related_to = face;
				face = dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw;
				unit = dcel.faces[face].relation_unit;

				last_edge = DCEL_half_edge_reserve(&dcel, table_mem);
				prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

				DCEL_half_edge_set(&dcel, prev_edge,
					dcel.he_table[horizon_edges[k % num_edges]].origin,
					-1,
					face,
					last_edge,
					-1);

				DCEL_half_edge_set(&dcel, last_edge,
					permutation[i],
					-1,
					face,
					-1,
					prev_edge);

				dcel.faces[face].he_index = last_edge;

				convex_hull_internal_DCEL_add_coplanar(&dcel, horizon_edges[j], horizon_edges[k-1], last_edge, prev_edge);
			
				i32 edge = dcel.he_table[horizon_edges[j]].twin;
				while (edge != dcel.he_table[horizon_edges[k-1]].twin) 
				{
					horizon_edges_to_remove += 2;
					const i32 tmp = dcel.he_table[edge].prev;
					arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
					arena_push_packed(mem_5, &edge, sizeof(i32));
					edge = tmp;
				}
				horizon_edges_to_remove += 2;
				arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
				arena_push_packed(mem_5, &edge, sizeof(i32));

				len_offset = 0;
				for (j = 0; j < k; ++j) 
				{ 
					len_offset += union_lens[j]; 
				}
			}
			else
			{
				for (; j != k; ) 
				{ 
					unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
					face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
					conflict_graph.r[unit].related_to = face;

					const i32 tmp = prev_edge;
					const i32 last_edge_in_polygon = DCEL_half_edge_reserve(&dcel, table_mem);
					if (j == ((upper + 1) % num_edges))
					{
						last_edge = last_edge_in_polygon;
					}
					prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

					DCEL_half_edge_set(&dcel, prev_edge,
						dcel.he_table[horizon_edges[(j+1) % num_edges]].origin,
						-1,
						face,
						last_edge_in_polygon,
						horizon_edges[j]);

					DCEL_half_edge_set(&dcel, last_edge_in_polygon,
						permutation[i],
						tmp,
						face,
						horizon_edges[j],
						prev_edge);
					if (tmp != -1) { dcel.he_table[tmp].twin = last_edge_in_polygon; }
					
					vec3_copy(origin, vs + 3*dcel.he_table[horizon_edges[j]].origin);
					vec3_sub(b, vs + 3*dcel.he_table[horizon_edges[(j+1) % num_edges]].origin, origin);
					vec3_sub(c, vs + 3*permutation[i], origin);
					vec3_cross(normal, b, c);
					vec3_mul_constant(normal, 1.0f / vec3_length(normal));
					convex_hull_internal_add_possible_conflicts(permutation, &conflict_graph, hash_index_hash_ptr(horizon_map), mem_5, unit, union_lens[j], union_lens + num_edges + len_offset, origin, normal, (vec3ptr) vs, EPSILON);
				
					dcel.he_table[horizon_edges[j]].prev = last_edge_in_polygon;
					dcel.he_table[horizon_edges[j]].next = prev_edge;
					dcel.he_table[horizon_edges[j]].face_ccw = face;

					len_offset += union_lens[j]; 
					j = (j+1) % num_edges;
					if (j == 0) { len_offset = 0; }
				}
			}

			for (; k < upper+1;)
			{
				for (k += 1; k < upper+1 && dcel.he_table[dcel.he_table[horizon_edges[k]].twin].face_ccw == dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw; k += 1);

				b_i = dcel.he_table[dcel.he_table[dcel.he_table[dcel.he_table[horizon_edges[j]].twin].next].next].origin;
				vec3_copy(origin, vs + 3*dcel.he_table[horizon_edges[j]].origin);
				vec3_sub(b, vs + 3*b_i, origin);
				vec3_sub(c, vs + 3*dcel.he_table[horizon_edges[(j+1) % num_edges]].origin, origin);
				vec3_cross(normal, b, c);
				vec3_mul_constant(normal, 1.0f / vec3_length(normal));
				vec3_sub(new, vs + 3*permutation[i], origin);
				/*coplanar if neighbor is on fat plane of new face */
				if (fabs(vec3_dot(new, normal)) < EPSILON)
				{
					//unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
					//face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
					//conflict_graph.r[unit].related_to = face;

					face = dcel.he_table[dcel.he_table[horizon_edges[j]].twin].face_ccw;
					unit = dcel.faces[face].relation_unit;

					const i32 tmp = prev_edge;
					const i32 last_edge_in_polygon = DCEL_half_edge_reserve(&dcel, table_mem);
					if (j == ((upper + 1) % num_edges))
					{
						last_edge = last_edge_in_polygon;
					}

					prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

					DCEL_half_edge_set(&dcel, prev_edge,
						dcel.he_table[horizon_edges[k % num_edges]].origin,
						-1,
						face,
						last_edge_in_polygon,
						-1);

					DCEL_half_edge_set(&dcel, last_edge_in_polygon,
						permutation[i],
						tmp,
						face,
						-1,
						prev_edge);
					dcel.he_table[tmp].twin = last_edge_in_polygon;

					dcel.faces[face].he_index = last_edge_in_polygon;

					convex_hull_internal_DCEL_add_coplanar(&dcel, horizon_edges[j], horizon_edges[k-1], last_edge_in_polygon, prev_edge);

					i32 edge = dcel.he_table[horizon_edges[j]].twin;
					while (edge != dcel.he_table[horizon_edges[k-1]].twin) 
					{
						horizon_edges_to_remove += 2;
						const i32 tmp = dcel.he_table[edge].prev;
						arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
						arena_push_packed(mem_5, &edge, sizeof(i32));
						edge = tmp;
					}
					horizon_edges_to_remove += 2;
					arena_push_packed(mem_5, &dcel.he_table[edge].twin, sizeof(i32));
					arena_push_packed(mem_5, &edge, sizeof(i32));

					for (; j < k; ++j) 
					{ 
						len_offset += union_lens[j]; 
					}
				}
				else
				{
					for (; j < k; ++j) 
					{ 
						unit = relation_list_add_relation_unit_empty(&conflict_graph, -1);
						face = DCEL_face_add(&dcel, face_mem, horizon_edges[j], unit);
						conflict_graph.r[unit].related_to = face;

						const i32 tmp = prev_edge;
						const i32 last_edge_in_polygon = DCEL_half_edge_reserve(&dcel, table_mem);
						prev_edge = DCEL_half_edge_reserve(&dcel, table_mem);

						DCEL_half_edge_set(&dcel, prev_edge,
							dcel.he_table[horizon_edges[(j+1) % num_edges]].origin,
							-1,
							face,
							last_edge_in_polygon,
							horizon_edges[j]);

						DCEL_half_edge_set(&dcel, last_edge_in_polygon,
							permutation[i],
							tmp,
							face,
							horizon_edges[j],
							prev_edge);
						dcel.he_table[tmp].twin = last_edge_in_polygon;

						vec3_copy(origin, vs + 3*dcel.he_table[horizon_edges[j]].origin);
						vec3_sub(b, vs + 3*dcel.he_table[horizon_edges[(j+1) % num_edges]].origin, origin);
						vec3_sub(c, vs + 3*permutation[i], origin);
						vec3_cross(normal, b, c);
						vec3_mul_constant(normal, 1.0f / vec3_length(normal));
						convex_hull_internal_add_possible_conflicts(permutation, &conflict_graph, hash_index_hash_ptr(horizon_map), mem_5, unit, union_lens[j], union_lens + num_edges + len_offset, origin, normal, (vec3ptr) vs, EPSILON);

						dcel.he_table[horizon_edges[j]].prev = last_edge_in_polygon;
						dcel.he_table[horizon_edges[j]].next = prev_edge;
						dcel.he_table[horizon_edges[j]].face_ccw = face;

						len_offset += union_lens[j]; 
					}
				}
			}

			assert(last_edge != -1);
			dcel.he_table[last_edge].twin = prev_edge;
			dcel.he_table[prev_edge].twin = last_edge;
			
			arena_pop_packed(mem_5, horizon_edges_to_remove * sizeof(i32));
			i32 *ptr = (i32 *) mem_5->stack_ptr;
			for (i32 z = 0; z < horizon_edges_to_remove; ++z)
			{
				DCEL_half_edge_remove(&dcel, ptr[z]);
			}

			/* cleanup mem_5 */
			for (i32 z = 0; z < num_conflict_faces; z++)
			{
				/* only free once if repeated */
				if (dcel.faces[conflict_faces[z]].relation_unit != -1)
				{
					relation_list_remove_relation_unit(&conflict_graph, dcel.faces[conflict_faces[z]].relation_unit);
					DCEL_face_remove(&dcel, conflict_faces[z]);
				}
			}
			arena_pop_packed(mem_5, num_conflict_faces * sizeof(i32));

			/* cleanup mem_4 */
			for (i32 z = 0; z < num_edges; ++z)
			{
				arena_pop_packed(mem_4, union_lens[z] * sizeof(i32));
			}
			for (i32 z = 0; z < edges_to_remove[0]; ++z)
			{
				DCEL_half_edge_remove(&dcel, edges_to_remove[z+1]);
			}
			arena_pop_packed(mem_4, (edges_to_remove[0]+1) * sizeof(i32));
			arena_pop_packed(mem_4, num_edges * sizeof(i32));
			arena_pop_packed(mem_4, num_edges * sizeof(i32));
		}
		relation_list_remove_relation_unit(&conflict_graph, i);
	}

	convex_hull_cs_step_draw_internal_push_data(&dcel, d_buf, vs, color, polygon_mode);

	/* Cleanup */
	hash_free(horizon_map);
	relation_list_free(&conflict_graph);
	arena_pop_packed(table_mem, dcel.num_he * sizeof(struct DCEL_half_edge));
	arena_pop_packed(face_mem, dcel.num_faces * sizeof(struct DCEL_face));
	arena_pop(table_mem, permutation_size);

	return 1;
}

#endif

void convex_centroid(vec3 centroid, vec3ptr vs, const u32 n)
{
	assert(n > 0);
	vec3_set(centroid, 0.0f, 0.0f, 0.0f);
	for (u32 i = 0; i < n; ++i)
	{
		vec3_translate(centroid, vs[i]);
	}
	vec3_mul_constant(centroid, 1.0f / n);
}

u32 convex_support(vec3 support, const vec3 dir, vec3ptr vs, const u32 n)
{
	f32 max = -FLT_MAX;
	u32 max_index = 0;
	for (u32 i = 0; i < n; ++i)
	{
		const f32 dot = vec3_dot(vs[i], dir);
		if (max < dot)
		{
			max_index = i;
			max = dot; 
		}
	}

	vec3_copy(support, vs[max_index]);
	return max_index;
}

f32 GJK_internal_tolerance(vec3ptr vs_1, const u32 n_1, vec3ptr vs_2, const u32 n_2, const f32 tol)
{
	vec3 c_1, c_2;
	convex_centroid(c_1, vs_1, n_1);
	convex_centroid(c_2, vs_2, n_2);

	/* Get max distance from centroids to convex surfaces */
	f32 l_sq_1 = 0.0f;
	for (u32 i = 0; i < n_1; ++i)
	{
		const f32 l_sq = vec3_distance_squared(c_1, vs_1[i]);
		if (l_sq_1 < l_sq)
		{
			l_sq_1 = l_sq;
		}
	}

	f32 l_sq_2 = 0.0f;
	for (u32 i = 0; i < n_2; ++i)
	{
		const f32 l_sq = vec3_distance_squared(c_2, vs_2[i]);
		if (l_sq_2 < l_sq)
		{
			l_sq_2 = l_sq;
		}
	}

	const f32 D = sqrtf(l_sq_1) + sqrtf(l_sq_2) + vec3_distance(c_1, c_2);
	return tol * D*D;
}

u64 convex_minkowski_difference_support(vec3 support, const vec3 dir, vec3ptr A, const u32 n_A, vec3ptr B, const u32 n_B)
{
	vec3 v_1, v_2, support_dir;
	const u32 i_1 = convex_support(v_1, dir, A, n_A);
	vec3_scale(support_dir, dir, -1.0f);
	const u32 i_2 = convex_support(v_2, support_dir, B, n_B);
	vec3_sub(support, v_1, v_2);

	return (u64) i_1 << 32 | (u64) i_2;
}

u64 convex_minkowski_difference_world_support(vec3 support, const vec3 dir, const vec3 pos_A, vec3ptr A, const u32 n_A, const vec3 pos_B, vec3ptr B, const u32 n_B)
{
	vec3 v_1, v_2, support_dir;
	const u32 i_1 = convex_support(v_1, dir, A, n_A);
	vec3_translate(v_1, pos_A);
	vec3_scale(support_dir, dir, -1.0f);
	const u32 i_2 = convex_support(v_2, support_dir, B, n_B);
	vec3_translate(v_2, pos_B);
	vec3_sub(support, v_1, v_2);

	return (u64) i_1 << 32 | (u64) i_2;
}

static struct gjk_simplex GJK_internal_simplex_init(void)
{
	struct gjk_simplex simplex = 
	{
		.id = {UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX},
		.dot = { -1.0f, -1.0f, -1.0f, -1.0f },
		.type = UINT32_MAX,
	};

	return simplex;
}

static u32 GJK_internal_johnsons_algorithm(struct gjk_simplex *simplex, vec3 c_v, vec4 lambda)
{
	vec3 a;

	if (simplex->type == 0)
	{
		vec3_copy(c_v, simplex->p[0]);
	}
	else if (simplex->type == 1)
	{
		vec3_sub(a, simplex->p[0], simplex->p[1]);
		const f32 delta_01_1 = vec3_dot(a, simplex->p[0]);

		if (delta_01_1 > 0.0f)
		{
			vec3_sub(a, simplex->p[1], simplex->p[0]);
			const f32 delta_01_0 = vec3_dot(a, simplex->p[1]);
			if (delta_01_0 > 0.0f)
			{
				const f32 delta = delta_01_0 + delta_01_1;
				lambda[0] = delta_01_0 / delta;
				lambda[1] = delta_01_1 / delta;
				vec3_set(c_v,
				       	(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[1])[0]),
				       	(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[1])[1]),
				       	(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[1])[2]));
			}
			else
			{
				simplex->type = 0;
				vec3_copy(c_v, simplex->p[1]);
				vec3_copy(simplex->p[0], simplex->p[1]);
			}
		}
		else
		{
			/* 
			 * numerical issues, new simplex should always contain newly added point
			 * of simplex, terminate next iteration. Let c_v stay the same as in the
			 * previous iteration.
			 */
			return 1;
		}
	}
	else if (simplex->type == 2)
	{
		vec3_sub(a, simplex->p[1], simplex->p[0]);
		const f32 delta_01_0 = vec3_dot(a, simplex->p[1]);
		vec3_sub(a, simplex->p[0], simplex->p[1]);
		const f32 delta_01_1 = vec3_dot(a, simplex->p[0]);
		vec3_sub(a, simplex->p[0], simplex->p[2]);
		const f32 delta_012_2 = delta_01_0 * vec3_dot(a, simplex->p[0]) + delta_01_1 * vec3_dot(a, simplex->p[1]);
		if (delta_012_2 > 0.0f)
		{
			vec3_sub(a, simplex->p[2], simplex->p[0]);
			const f32 delta_02_0 = vec3_dot(a, simplex->p[2]);
			vec3_sub(a, simplex->p[0], simplex->p[2]);
			const f32 delta_02_2 = vec3_dot(a, simplex->p[0]);
			vec3_sub(a, simplex->p[0], simplex->p[1]);
			const f32 delta_012_1 = delta_02_0 * vec3_dot(a, simplex->p[0]) + delta_02_2 * vec3_dot(a, simplex->p[2]);
			if (delta_012_1 > 0.0f)
			{
				vec3_sub(a, simplex->p[2], simplex->p[1]);
				const f32 delta_12_1 = vec3_dot(a, simplex->p[2]);
				vec3_sub(a, simplex->p[1], simplex->p[2]);
				const f32 delta_12_2 = vec3_dot(a, simplex->p[1]);
				vec3_sub(a, simplex->p[1], simplex->p[0]);
				const f32 delta_012_0 = delta_12_1 * vec3_dot(a, simplex->p[1]) + delta_12_2 * vec3_dot(a, simplex->p[2]);
				if (delta_012_0 > 0.0f)
				{
					const f32 delta = delta_012_0 + delta_012_1 + delta_012_2;
					lambda[0] = delta_012_0 / delta;
					lambda[1] = delta_012_1 / delta;
					lambda[2] = delta_012_2 / delta;
					vec3_set(c_v,
						(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[1])[0] + lambda[2]*(simplex->p[2])[0]),
						(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[1])[1] + lambda[2]*(simplex->p[2])[1]),
						(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[1])[2] + lambda[2]*(simplex->p[2])[2]));
				}
				else
				{
					if (delta_12_2 > 0.0f)
					{
						if (delta_12_1 > 0.0f)
						{
							const f32 delta = delta_12_1 + delta_12_2;
							lambda[0] = delta_12_1 / delta;
							lambda[1] = delta_12_2 / delta;
							vec3_set(c_v,
							       	(lambda[0]*(simplex->p[1])[0] + lambda[1]*(simplex->p[2])[0]),
							       	(lambda[0]*(simplex->p[1])[1] + lambda[1]*(simplex->p[2])[1]),
							       	(lambda[0]*(simplex->p[1])[2] + lambda[1]*(simplex->p[2])[2]));
							simplex->type = 1;
							vec3_copy(simplex->p[0], simplex->p[1]);
							vec3_copy(simplex->p[1], simplex->p[2]);
							simplex->id[0] = simplex->id[1];
							simplex->dot[0] = simplex->dot[1];
						}
						else
						{
							simplex->type = 0;
							vec3_copy(c_v, simplex->p[2]);
							vec3_copy(simplex->p[0], simplex->p[2]);
							simplex->id[1] = UINT32_MAX;
							simplex->dot[1] = -1.0f;
						}


					}
					else
					{
						return 1;
					}
				}

			}
			else
			{
				if (delta_02_2 > 0.0f)
				{
					if (delta_02_0 > 0.0f)
					{
						const f32 delta = delta_02_0 + delta_02_2;
						lambda[0] = delta_02_0 / delta;
						lambda[1] = delta_02_2 / delta;
						vec3_set(c_v,
						       	(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[2])[0]),
						       	(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[2])[1]),
						       	(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[2])[2]));
						simplex->type = 1;
						vec3_copy(simplex->p[1], simplex->p[2]);
					}
					else
					{
						simplex->type = 0;
						vec3_copy(c_v, simplex->p[2]);
						vec3_copy(simplex->p[0], simplex->p[2]);
						simplex->id[1] = UINT32_MAX;
						simplex->dot[1] = -1.0f;
					}
				}
			}
		}
		else
		{
			return 1;
		}
	}
	else
	{
		vec3_sub(a, simplex->p[1], simplex->p[0]);
		const f32 delta_01_0 = vec3_dot(a, simplex->p[1]);
		vec3_sub(a, simplex->p[0], simplex->p[1]);
		const f32 delta_01_1 = vec3_dot(a, simplex->p[0]);
		vec3_sub(a, simplex->p[0], simplex->p[2]);
		const f32 delta_012_2 = delta_01_0 * vec3_dot(a, simplex->p[0]) + delta_01_1 * vec3_dot(a, simplex->p[1]);

		vec3_sub(a, simplex->p[2], simplex->p[0]);
		const f32 delta_02_0 = vec3_dot(a, simplex->p[2]);
		vec3_sub(a, simplex->p[0], simplex->p[2]);
		const f32 delta_02_2 = vec3_dot(a, simplex->p[0]);
		vec3_sub(a, simplex->p[0], simplex->p[1]);
		const f32 delta_012_1 = delta_02_0 * vec3_dot(a, simplex->p[0]) + delta_02_2 * vec3_dot(a, simplex->p[2]);

		vec3_sub(a, simplex->p[2], simplex->p[1]);
		const f32 delta_12_1 = vec3_dot(a, simplex->p[2]);
		vec3_sub(a, simplex->p[1], simplex->p[2]);
		const f32 delta_12_2 = vec3_dot(a, simplex->p[1]);
		vec3_sub(a, simplex->p[1], simplex->p[0]);
		const f32 delta_012_0 = delta_12_1 * vec3_dot(a, simplex->p[1]) + delta_12_2 * vec3_dot(a, simplex->p[2]);

		vec3_sub(a, simplex->p[0], simplex->p[3]);
		const f32 delta_0123_3 = delta_012_0 * vec3_dot(a, simplex->p[0]) + delta_012_1 * vec3_dot(a, simplex->p[1]) + delta_012_2 * vec3_dot(a, simplex->p[2]);

		if (delta_0123_3 > 0.0f)
		{
			vec3_sub(a, simplex->p[0], simplex->p[3]);
			const f32 delta_013_3 = delta_01_0 * vec3_dot(a, simplex->p[0]) + delta_01_1 * vec3_dot(a, simplex->p[1]);

			vec3_sub(a, simplex->p[3], simplex->p[0]);
			const f32 delta_03_0 = vec3_dot(a, simplex->p[3]);
			vec3_sub(a, simplex->p[0], simplex->p[3]);
			const f32 delta_03_3 = vec3_dot(a, simplex->p[0]);
			vec3_sub(a, simplex->p[0], simplex->p[1]);
			const f32 delta_013_1 = delta_03_0 * vec3_dot(a, simplex->p[0]) + delta_03_3 * vec3_dot(a, simplex->p[3]);

			vec3_sub(a, simplex->p[3], simplex->p[1]);
			const f32 delta_13_1 = vec3_dot(a, simplex->p[3]);
			vec3_sub(a, simplex->p[1], simplex->p[3]);
			const f32 delta_13_3 = vec3_dot(a, simplex->p[1]);
			vec3_sub(a, simplex->p[1], simplex->p[0]);
			const f32 delta_013_0 = delta_13_1 * vec3_dot(a, simplex->p[1]) + delta_13_3 * vec3_dot(a, simplex->p[3]);

			vec3_sub(a, simplex->p[0], simplex->p[2]);
			const f32 delta_0123_2 = delta_013_0 * vec3_dot(a, simplex->p[0]) + delta_013_1 * vec3_dot(a, simplex->p[1]) + delta_013_3 * vec3_dot(a, simplex->p[3]);

			if (delta_0123_2 > 0.0f)
			{
				vec3_sub(a, simplex->p[0], simplex->p[3]);
				const f32 delta_023_3 = delta_02_0 * vec3_dot(a, simplex->p[0]) + delta_02_2 * vec3_dot(a, simplex->p[2]);

				vec3_sub(a, simplex->p[0], simplex->p[2]);
				const f32 delta_023_2 = delta_03_0 * vec3_dot(a, simplex->p[0]) + delta_03_3 * vec3_dot(a, simplex->p[3]);

				vec3_sub(a, simplex->p[3], simplex->p[2]);
				const f32 delta_23_2 = vec3_dot(a, simplex->p[3]);
				vec3_sub(a, simplex->p[2], simplex->p[3]);
				const f32 delta_23_3 = vec3_dot(a, simplex->p[2]);
				vec3_sub(a, simplex->p[2], simplex->p[0]);
				const f32 delta_023_0 = delta_23_2 * vec3_dot(a, simplex->p[2]) + delta_23_3 * vec3_dot(a, simplex->p[3]);

				vec3_sub(a, simplex->p[0], simplex->p[1]);
				const f32 delta_0123_1 = delta_023_0 * vec3_dot(a, simplex->p[0]) + delta_023_2 * vec3_dot(a, simplex->p[2]) + delta_023_3 * vec3_dot(a, simplex->p[3]);

				if (delta_0123_1 > 0.0f)
				{
					vec3_sub(a, simplex->p[3], simplex->p[1]);
					const f32 delta_123_1 = delta_23_2 * vec3_dot(a, simplex->p[2]) + delta_23_3 * vec3_dot(a, simplex->p[3]);

					vec3_sub(a, simplex->p[3], simplex->p[2]);
					const f32 delta_123_2 = delta_13_1 * vec3_dot(a, simplex->p[1]) + delta_13_3 * vec3_dot(a, simplex->p[3]);

					vec3_sub(a, simplex->p[1], simplex->p[3]);
					const f32 delta_123_3 = delta_12_1 * vec3_dot(a, simplex->p[1]) + delta_12_2 * vec3_dot(a, simplex->p[2]);

					vec3_sub(a, simplex->p[3], simplex->p[0]);
					const f32 delta_0123_0 = delta_123_1 * vec3_dot(a, simplex->p[1]) + delta_123_2 * vec3_dot(a, simplex->p[2]) + delta_123_3 * vec3_dot(a, simplex->p[3]);

					if (delta_0123_0 > 0.0f)
					{
						/* intersection */
						const f32 delta = delta_0123_0 + delta_0123_1 + delta_0123_2 + delta_0123_3;
						lambda[0] = delta_0123_0 / delta;
						lambda[1] = delta_0123_1 / delta;
						lambda[2] = delta_0123_2 / delta;
						lambda[3] = delta_0123_3 / delta;
						vec3_set(c_v,
							(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[1])[0] + lambda[2]*(simplex->p[2])[0] + lambda[3]*(simplex->p[3])[0]),
							(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[1])[1] + lambda[2]*(simplex->p[2])[1] + lambda[3]*(simplex->p[3])[1]),
							(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[1])[2] + lambda[2]*(simplex->p[2])[2] + lambda[3]*(simplex->p[3])[2]));
					}
					else
					{
						/* check 123 subset */
						if (delta_123_3 > 0.0f)
						{
							if (delta_123_2 > 0.0f)
							{
								if (delta_123_1 > 0.0f)
								{
									const f32 delta = delta_123_1 + delta_123_2 + delta_123_3;
									lambda[0] = delta_123_1 / delta;
									lambda[1] = delta_123_2 / delta;
									lambda[2] = delta_123_3 / delta;
									vec3_set(c_v,
										(lambda[0]*(simplex->p[1])[0] + lambda[1]*(simplex->p[2])[0] + lambda[2]*(simplex->p[3])[0]),
										(lambda[0]*(simplex->p[1])[1] + lambda[1]*(simplex->p[2])[1] + lambda[2]*(simplex->p[3])[1]),
										(lambda[0]*(simplex->p[1])[2] + lambda[1]*(simplex->p[2])[2] + lambda[2]*(simplex->p[3])[2]));
									simplex->type = 2;
									vec3_copy(simplex->p[0], simplex->p[1]);		
									vec3_copy(simplex->p[1], simplex->p[2]);		
									vec3_copy(simplex->p[2], simplex->p[3]);		
									simplex->dot[0] = simplex->dot[1];
									simplex->dot[1] = simplex->dot[2];
									simplex->id[0] = simplex->id[1];
									simplex->id[1] = simplex->id[2];
								}
								else
								{
									/* check 23 */
									if (delta_23_3 > 0.0f)
									{
										if (delta_23_2 > 0.0f)
										{
											const f32 delta = delta_23_2 + delta_23_3;
											lambda[0] = delta_23_2 / delta;
											lambda[1] = delta_23_3 / delta;
											vec3_set(c_v,
												(lambda[0]*(simplex->p[2])[0] + lambda[1]*(simplex->p[3])[0]),
												(lambda[0]*(simplex->p[2])[1] + lambda[1]*(simplex->p[3])[1]),
												(lambda[0]*(simplex->p[2])[2] + lambda[1]*(simplex->p[3])[2]));
											simplex->type = 1;
											vec3_copy(simplex->p[0], simplex->p[2]);		
											vec3_copy(simplex->p[1], simplex->p[3]);		
											simplex->dot[0] = simplex->dot[2];
											simplex->dot[2] = -1.0f;
											simplex->id[0] = simplex->id[2];
											simplex->id[2] = UINT32_MAX;
										}
										else
										{
											vec3_copy(c_v, simplex->p[3]);
											simplex->type = 0;
											vec3_copy(simplex->p[0], simplex->p[3]);
											simplex->dot[1] = -1.0f;
											simplex->dot[2] = -1.0f;
											simplex->id[1] = UINT32_MAX;
											simplex->id[2] = UINT32_MAX;
										}
									}
									else
									{
										return 1;
									}
								}
							}
							else
							{
								/* check 13 subset */
								if (delta_13_3 > 0.0f)
								{
									if (delta_13_1 > 0.0f)
									{
										const f32 delta = delta_13_1 + delta_13_3;
										lambda[0] = delta_13_1 / delta;
										lambda[1] = delta_13_3 / delta;
										vec3_set(c_v,
											(lambda[0]*(simplex->p[1])[0] + lambda[1]*(simplex->p[3])[0]),
											(lambda[0]*(simplex->p[1])[1] + lambda[1]*(simplex->p[3])[1]),
											(lambda[0]*(simplex->p[1])[2] + lambda[1]*(simplex->p[3])[2]));
										simplex->type = 1;
										vec3_copy(simplex->p[0], simplex->p[1]);
										vec3_copy(simplex->p[1], simplex->p[3]);		
										simplex->dot[0] = simplex->dot[1];
										simplex->dot[2] = -1.0f;
										simplex->id[0] = simplex->id[1];
										simplex->id[2] = UINT32_MAX;
									}
									else
									{
										vec3_copy(c_v, simplex->p[3]);
										simplex->type = 0;
										vec3_copy(simplex->p[0], simplex->p[3]);
										simplex->dot[1] = -1.0f;
										simplex->dot[2] = -1.0f;
										simplex->id[1] = UINT32_MAX;
										simplex->id[2] = UINT32_MAX;
									}
								}
								else
								{
									return 1;
								}
							}	
						}
						else
						{
							return 1;
						}
					}
				}
				else
				{
					/* check 023 subset */
					if (delta_023_3 > 0.0f)
					{
						if (delta_023_2 > 0.0f)
						{
							if (delta_023_0 > 0.0f)
							{
								const f32 delta = delta_023_0 + delta_023_2 + delta_023_3;
								lambda[0] = delta_023_0 / delta;
								lambda[1] = delta_023_2 / delta;
								lambda[2] = delta_023_3 / delta;
								vec3_set(c_v,
									(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[2])[0] + lambda[2]*(simplex->p[3])[0]),
									(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[2])[1] + lambda[2]*(simplex->p[3])[1]),
									(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[2])[2] + lambda[2]*(simplex->p[3])[2]));
								simplex->type = 2;
								vec3_copy(simplex->p[1], simplex->p[2]);		
								vec3_copy(simplex->p[2], simplex->p[3]);		
								simplex->dot[1] = simplex->dot[2];
								simplex->id[1] = simplex->id[2];
							}
							else
							{
								/* check 23 subset */
								if (delta_23_3 > 0.0f)
								{
									if (delta_23_2 > 0.0f)
									{
										const f32 delta = delta_23_2 + delta_23_3;
										lambda[0] = delta_23_2 / delta;
										lambda[1] = delta_23_3 / delta;
										vec3_set(c_v,
											(lambda[0]*(simplex->p[2])[0] + lambda[1]*(simplex->p[3])[0]),
											(lambda[0]*(simplex->p[2])[1] + lambda[1]*(simplex->p[3])[1]),
											(lambda[0]*(simplex->p[2])[2] + lambda[1]*(simplex->p[3])[2]));
										simplex->type = 1;
										vec3_copy(simplex->p[0], simplex->p[2]);
										vec3_copy(simplex->p[1], simplex->p[3]);
										simplex->dot[0] = simplex->dot[2];
										simplex->dot[2] = -1.0f;
										simplex->id[0] = simplex->id[2];
										simplex->id[2] = UINT32_MAX;
									}
									else
									{
										vec3_copy(c_v, simplex->p[3]);
										simplex->type = 0;
										vec3_copy(simplex->p[0], simplex->p[3]);
										simplex->dot[1] = -1.0f;
										simplex->dot[2] = -1.0f;
										simplex->id[1] = UINT32_MAX;
										simplex->id[2] = UINT32_MAX;
									}
								}
								else
								{
									return 1;
								}
							}
						}
						else
						{
							/* check 03 subset */
							if (delta_03_3 > 0.0f)
							{
								if (delta_03_0 > 0.0f)
								{
									const f32 delta = delta_03_0 + delta_03_3;
									lambda[0] = delta_03_0 / delta;
									lambda[1] = delta_03_3 / delta;
									vec3_set(c_v,
										(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[3])[0]),
										(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[3])[1]),
										(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[3])[2]));
									simplex->type = 1;
									vec3_copy(simplex->p[1], simplex->p[3]);
									simplex->dot[2] = -1.0f;
									simplex->id[2] = UINT32_MAX;
								}
								else
								{
									vec3_copy(c_v, simplex->p[3]);
									simplex->type = 0;
									vec3_copy(simplex->p[0], simplex->p[3]);
									simplex->dot[1] = -1.0f;
									simplex->dot[2] = -1.0f;
									simplex->id[1] = UINT32_MAX;
									simplex->id[2] = UINT32_MAX;
								}
							}
							else
							{
								return 1;
							}
						}
					}
					else
					{
						return 1;
					}
				}
			}
			else
			{
				/* check 013 subset */
				if (delta_013_3 > 0.0f)
				{
					if (delta_013_1 > 0.0f)
					{
						if (delta_013_0 > 0.0f)
						{
							const f32 delta = delta_013_0 + delta_013_1 + delta_013_3;
							lambda[0] = delta_013_0 / delta;
							lambda[1] = delta_013_1 / delta;
							lambda[2] = delta_013_3 / delta;
							vec3_set(c_v,
								(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[1])[0] + lambda[2]*(simplex->p[3])[0]),
								(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[1])[1] + lambda[2]*(simplex->p[3])[1]),
								(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[1])[2] + lambda[2]*(simplex->p[3])[2]));
							simplex->type = 2;
							vec3_copy(simplex->p[2], simplex->p[3]);
						}
						else
						{
							/* check 13 subset */
							if (delta_13_3 > 0.0f)
							{
								if (delta_13_1 > 0.0f)
								{
									const f32 delta = delta_13_1 + delta_13_3;
									lambda[0] = delta_13_1 / delta;
									lambda[1] = delta_13_3 / delta;
									vec3_set(c_v,
										(lambda[0]*(simplex->p[1])[0] + lambda[1]*(simplex->p[3])[0]),
										(lambda[0]*(simplex->p[1])[1] + lambda[1]*(simplex->p[3])[1]),
										(lambda[0]*(simplex->p[1])[2] + lambda[1]*(simplex->p[3])[2]));
									simplex->type = 1;
									vec3_copy(simplex->p[0], simplex->p[1]);
									vec3_copy(simplex->p[1], simplex->p[3]);
									simplex->dot[2] = -1.0f;
									simplex->id[2] = UINT32_MAX;
								}
								else
								{
									vec3_copy(c_v, simplex->p[3]);
									simplex->type = 0;
									vec3_copy(simplex->p[0], simplex->p[3]);
									simplex->dot[1] = -1.0f;
									simplex->dot[2] = -1.0f;
									simplex->id[1] = UINT32_MAX;
									simplex->id[2] = UINT32_MAX;
								}
							}
							else
							{
								return 1;
							}
						}	
					}
					else
					{
						/* check 03 subset */
						if (delta_03_3 > 0.0f)
						{
							if (delta_03_0 > 0.0f)
							{
								const f32 delta = delta_03_0 + delta_03_3;
								lambda[0] = delta_03_0 / delta;
								lambda[1] = delta_03_3 / delta;
								vec3_set(c_v,
									(lambda[0]*(simplex->p[0])[0] + lambda[1]*(simplex->p[3])[0]),
									(lambda[0]*(simplex->p[0])[1] + lambda[1]*(simplex->p[3])[1]),
									(lambda[0]*(simplex->p[0])[2] + lambda[1]*(simplex->p[3])[2]));
								simplex->type = 1;
								vec3_copy(simplex->p[1], simplex->p[3]);
								simplex->dot[2] = -1.0f;
								simplex->id[2] = UINT32_MAX;
							}
							else
							{
								vec3_copy(c_v, simplex->p[3]);
								simplex->type = 0;
								vec3_copy(simplex->p[0], simplex->p[3]);
								simplex->dot[1] = -1.0f;
								simplex->dot[2] = -1.0f;
								simplex->id[1] = UINT32_MAX;
								simplex->id[2] = UINT32_MAX;
							}
						}
						else
						{
							return 1;
						}
					}
				}
				else
				{
					return 1;
				}
			}
		}
		else
		{
			return 1;
		}
	}

	return 0;
}

u32 GJK_test(const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2, const f32 abs_tol, const f32 tol)
{
	struct gjk_simplex simplex = GJK_internal_simplex_init();
	/* c_v - closest point in each iteration, dir = -c_v */
	vec3 dir, c_v;
	vec4 lambda;
	u64 support_id;
	f32 ma; /* max dot product of current simplex */

	/* arbitrary starting search direction */
	vec3_set(c_v, 1.0f, 0.0f, 0.0f);

	do
	{
		simplex.type += 1;
		vec3_scale(dir, c_v, -1.0f);
		support_id = convex_minkowski_difference_world_support(simplex.p[simplex.type], dir, pos_1, vs_1, n_1, pos_2, vs_2, n_2);
		if (vec3_dot(simplex.p[simplex.type], dir) < 0.0f)
		{
			return 0;
		}

		/* Degenerate Case: Check if new support point is already in simplex */
		if (simplex.id[0] == support_id || simplex.id[1] == support_id || simplex.id[2] == support_id || simplex.id[3] == support_id)
		{
			return 0;
		}

		/* find closest point v to origin using naive Johnson's algorithm, update simplex data 
		 * Degenerate Case: due to numerical issues, determinat signs may flip, which may result
		 * either in wrong sub-simplex being chosen, or no valid simplex at all. In that case c_v
		 * stays the same, and we terminate the algorithm. [See page 142].
		 */
		if (GJK_internal_johnsons_algorithm(&simplex, c_v, lambda))
		{
			return 0;
		}

		simplex.id[simplex.type] = support_id;
		simplex.dot[simplex.type] = vec3_dot(simplex.p[simplex.type], simplex.p[simplex.type]);

		/* 
		 * If the simplex is of type 3, or a tetrahedron, we have encapsulated 0, or, if v is sufficiently
		 * close to the origin, within a margin of error, return an intersection.
		 */
		if (simplex.type == 3)
		{
			return 1;
		}
		else
		{
			ma = simplex.dot[0];
			ma = fmax(ma, simplex.dot[1]);
			ma = fmax(ma, simplex.dot[2]);
			ma = fmax(ma, simplex.dot[3]);

			/* For error bound discussion, see sections 4.3.5, 4.3.6 */
			if (vec3_dot(c_v, c_v) <= tol * ma)
			{
				return 1;
			}
		}
	} while (1);
}

static void GJK_internal_closest_points_on_bodies(vec3 c_1, vec3 c_2, vec3ptr vs_1, const vec3 pos_1, vec3ptr vs_2, const vec3 pos_2, const u64 simplex_id[4], const vec4 lambda, const u32 simplex_type)
{
	vec3_copy(c_1, pos_1);
	vec3_copy(c_2, pos_2);
	if (simplex_type == 0)
	{
		vec3_translate(c_1, vs_1[simplex_id[0] >> 32]);
		vec3_translate(c_2, vs_2[simplex_id[0] & 0xffffffff]);
	}
	else
	{
		vec3 v_1, v_2;
		for (u32 i = 0; i <= simplex_type; ++i)
		{
			vec3_scale(v_1, vs_1[simplex_id[i] >> 32], lambda[i]);
			vec3_scale(v_2, vs_2[simplex_id[i] & 0xffffffff], lambda[i]);
			vec3_translate(c_1, v_1);
			vec3_translate(c_2, v_2);
		}	
	}
}

static f32 GJK_distance_internal(struct gjk_simplex *simplex, vec3 c_1, vec3 c_2, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2, const f32 rel_tol, const f32 abs_tol)
{ 
	*simplex = GJK_internal_simplex_init();
	/* c_v - closest point in each iteration, dir = -c_v */
	vec3 dir, c_v;
	/* lambda = coefficients for simplex point mix */ 
	vec4 lambda;
	u64 support_id;
	f32 ma; /* max dot product of current simplex */
	f32 c_v_distance_sq = FLT_MAX; /* closest point on simplex distance to origin */
	const f32 rel = rel_tol * rel_tol;

	/* arbitrary starting search direction */
	vec3_set(c_v, 1.0f, 0.0f, 0.0f);
	u64 old_support = UINT64_MAX;

	do
	{
		simplex->type += 1;
		vec3_scale(dir, c_v, -1.0f);
		support_id = convex_minkowski_difference_world_support(simplex->p[simplex->type], dir, pos_1, vs_1, n_1, pos_2, vs_2, n_2);
		if (c_v_distance_sq - vec3_dot(simplex->p[simplex->type], c_v) <= rel * c_v_distance_sq + abs_tol
				|| simplex->id[0] == support_id || simplex->id[1] == support_id 
				|| simplex->id[2] == support_id || simplex->id[3] == support_id)
		{
			assert(simplex->id != 0);
			assert(c_v_distance_sq != FLT_MAX);
			simplex->type -= 1;
			GJK_internal_closest_points_on_bodies(c_1, c_2, vs_1, pos_1, vs_2, pos_2, simplex->id, lambda, simplex->type);
			return sqrtf(c_v_distance_sq);
		}

		/* find closest point v to origin using naive Johnson's algorithm, update simplex data 
		 * Degenerate Case: due to numerical issues, determinant signs may flip, which may result
		 * either in wrong sub-simplex being chosen, or no valid simplex at all. In that case c_v
		 * stays the same, and we terminate the algorithm. [See page 142].
		 */
		if (GJK_internal_johnsons_algorithm(simplex, c_v, lambda))
		{
			assert(c_v_distance_sq != FLT_MAX);
			simplex->type -= 1;
			GJK_internal_closest_points_on_bodies(c_1, c_2, vs_1, pos_1, vs_2, pos_2, simplex->id, lambda, simplex->type);
			return sqrtf(c_v_distance_sq);
		}

		simplex->id[simplex->type] = support_id;
		simplex->dot[simplex->type] = vec3_dot(simplex->p[simplex->type], simplex->p[simplex->type]);

		/* 
		 * If the simplex is of type 3, or a tetrahedron, we have encapsulated 0, or, if v is sufficiently
		 * close to the origin, within a margin of error, return an intersection.
		 */
		if (simplex->type == 3)
		{
			return 0.0f;
		}
		else
		{
			ma = simplex->dot[0];
			ma = fmax(ma, simplex->dot[1]);
			ma = fmax(ma, simplex->dot[2]);
			ma = fmax(ma, simplex->dot[3]);

			/* For error bound discussion, see sections 4.3.5, 4.3.6 */
			c_v_distance_sq = vec3_dot(c_v, c_v);
			if (c_v_distance_sq <= abs_tol * ma)
			{
				return 0.0f;
			}
		}
	} while (1);

	return 0.0f;
}

f32 GJK_distance(vec3 c_1, vec3 c_2, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2, const f32 rel_tol, const f32 abs_tol)
{
	struct gjk_simplex simplex;
	return GJK_distance_internal(&simplex, c_1, c_2, pos_1, vs_1, n_1, pos_2, vs_2, n_2, rel_tol, abs_tol);
}

static u32 EPA_internal_check_unique_identifiers(const u64 id[4])
{
	for (u32 i = 0; i < 4; ++i)
	{
		for (u32 j = i+1; j < 4; ++j)
		{
			if (id[i] == id[j])
			{
				return 0;
			}
		}
	}

	return 1;
}

static u32 EPA_internal_tetrahedron_from_line(struct gjk_simplex *simplex, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2)
{
	vec3 tmp, p_1, support_dir;
	vec3_copy(p_1, simplex->p[1]);	
	vec3_sub(tmp, simplex->p[0], simplex->p[1]);

	const u64 tmp_id = simplex->id[1];

	u32 min_axis = 0;
	if (tmp[0]*tmp[0] > tmp[1]*tmp[1]) { min_axis = 1; }
	if (tmp[min_axis]*tmp[min_axis] > tmp[2]*tmp[2]) { min_axis = 2; }
	
	mat3 rotation;
	rotation_matrix(rotation, tmp, 2.0f * MM_PI_F / 3.0f);

	/* For best numerical stability, we choose the first vector to be the axis for 
	 * which the projection of p_01 is the smallest. (More independent) */
	vec3_set(simplex->p[1], 0.0f, 0.0f, 0.0f);
	simplex->p[1][min_axis] = 1.0f;
	vec3_cross(support_dir, tmp, simplex->p[1]);
	simplex->id[1] = convex_minkowski_difference_world_support(simplex->p[1], support_dir, pos_1, vs_1, n_1, pos_2, vs_2, n_2);

	vec3_copy(tmp, support_dir);
	mat3_vec_mul(support_dir, rotation, tmp);
	simplex->id[2] = convex_minkowski_difference_world_support(simplex->p[2], support_dir, pos_1, vs_1, n_1, pos_2, vs_2, n_2);

	vec3_copy(tmp, support_dir);
	mat3_vec_mul(support_dir, rotation, tmp);
	simplex->id[3] = convex_minkowski_difference_world_support(simplex->p[3], support_dir, pos_1, vs_1, n_1, pos_2, vs_2, n_2);
	
	vec3_set(tmp, 0.0f, 0.0f, 0.0f);
	u32 valid = 0;
	if (EPA_internal_check_unique_identifiers(simplex->id) && tetrahedron_point_test(simplex->p, tmp)) 
	{  
		valid = 1;
	}
	else
	{
		simplex->id[0] = tmp_id;
		vec3_copy(simplex->p[0], p_1);
		if (EPA_internal_check_unique_identifiers(simplex->id) && tetrahedron_point_test(simplex->p, tmp)) 
		{  
			valid = 1;
		};
	}

	return valid;
}

static u32 EPA_internal_tetrahedron_from_triangle(struct gjk_simplex *simplex, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2)
{
	vec3 n, AB, AC;
	vec3 origin = VEC3_ZERO;
	vec3_sub(AB, simplex->p[1], simplex->p[0]);
	vec3_sub(AC, simplex->p[2], simplex->p[0]);
	vec3_cross(n, AB, AC);

	u32 valid = 0;
	for (u32 i = 0; i < 2; ++i)
	{
		vec3_negative(n);
		simplex->id[3] = convex_minkowski_difference_world_support(simplex->p[3], n, pos_1, vs_1, n_1, pos_2, vs_2, n_2);		
		if (EPA_internal_check_unique_identifiers(simplex->id) && tetrahedron_point_test(simplex->p, origin)) 
		{  
			valid = 1;
			break;
		};
	}

	return valid;
}

static u32 EPA_internal_setup_tetrahedron(struct gjk_simplex *simplex, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2)
{
	u32 valid = 0;
	switch (simplex->type)
	{
		case 0: break;

		case 1: 
		{
			valid = EPA_internal_tetrahedron_from_line(simplex, pos_1, vs_1, n_1, pos_2, vs_2, n_2);
		} break;

		case 2: 
		{
			valid = EPA_internal_tetrahedron_from_triangle(simplex, pos_1, vs_1, n_1, pos_2, vs_2, n_2); 
		} break;

		case 3: 
		{
			const vec3 origin = VEC3_ZERO;
			valid = tetrahedron_point_test(simplex->p, origin);
		} break;
	}

	return valid;
}

struct EPA_entry
{
	u64 id[3];
	vec3 ABC[3];		/* ABC is ccw ordered */
	vec3 closest_point;
	vec3 lambda;		/* coefficient of closest point to origin */
	f32 distance_sq;	/* distance squared to origin */
	u16 adjacent[3];	/* adjacent entries to edges in order AB, BC, CA */
	u8 twin_edge[3];	/* for entry e and edge i, twin_edge[i] = j s.t. e.adj[i].adj[j] = e. */
	u8 valid;
};

void EPA_entry_print(struct EPA_entry *e)
{
	printf("{\n");
	printf("\t.id = { %lu, %lu, %lu }\n", e->id[0], e->id[1], e->id[2]);
	printf("\t.A = { %f, %f, %f }\n", e->ABC[0][0], e->ABC[0][1], e->ABC[0][2]);
	printf("\t.B = { %f, %f, %f }\n", e->ABC[1][0], e->ABC[1][1], e->ABC[1][2]);
	printf("\t.C = { %f, %f, %f }\n", e->ABC[2][0], e->ABC[2][1], e->ABC[2][2]);
	printf("\t.closest_point = { %f, %f, %f }\n", e->closest_point[0], e->closest_point[1], e->closest_point[2]);
	printf("\t.lambda = { %f, %f, %f }\n", e->lambda[0], e->lambda[1], e->lambda[2]);
	printf("\t.distance_sq = %f\n", e->distance_sq);
	printf("\t.adjacent = { %u, %u, %u }\n", e->adjacent[0], e->adjacent[1], e->adjacent[2]);
	printf("\t.twin_edge = { %u, %u, %u }\n", e->twin_edge[0], e->twin_edge[1], e->twin_edge[2]);
	printf("\t.valid = %u\n", e->valid);
	printf("}\n");
}

static f32 EPA_internal_initiate_entry_from_CCW_triangle(struct EPA_entry *entry, const u64 id_A, const u64 id_B, const u64 id_C, const vec3 A, const vec3 B, const vec3 C, const u16 adjacent_0, const u16 adjacent_1, const u16 adjacent_2, const u8 twin_edge_0, const u8 twin_edge_1, const u8 twin_edge_2)
{
	entry->id[0] = id_A;
	entry->id[1] = id_B;
	entry->id[2] = id_C;
	vec3_copy(entry->ABC[0], A);
	vec3_copy(entry->ABC[1], B);
	vec3_copy(entry->ABC[2], C);
	f32 determinant = triangle_origin_closest_point(entry->lambda, A, B, C);
	vec3_scale(entry->closest_point, A, entry->lambda[0]);
	vec3_translate_scaled(entry->closest_point, B, entry->lambda[1]);
	vec3_translate_scaled(entry->closest_point, C, entry->lambda[2]);
	entry->distance_sq = vec3_dot(entry->closest_point, entry->closest_point);
	entry->adjacent[0] = adjacent_0;
	entry->adjacent[1] = adjacent_1;
	entry->adjacent[2] = adjacent_2;
	entry->twin_edge[0] = twin_edge_0;
	entry->twin_edge[1] = twin_edge_1;
	entry->twin_edge[2] = twin_edge_2;
	entry->valid = 1;
	return determinant;
}

#ifdef MG_DEBUG
#define EPA_ASSERT_VALID_ENTRIES(entries)	EPA_assert_valid_entries(entries)
#else
#undef EPA_ASSERT_VALID_ENTRIES(entries)
#endif

static void EPA_assert_valid_entries(struct gen_array_list *list)
{
	for (u32 i = 0; i < list->max_count; ++i)
	{
		const struct EPA_entry *e = gen_array_list_address(list, i);
		if (e->valid)
		{
			for (u32 j = 0; j < 3; ++j)
			{
				const struct EPA_entry *adj = gen_array_list_address(list, e->adjacent[j]);

				assert(i == adj->adjacent[e->twin_edge[j]]);
				assert(j == adj->twin_edge[e->twin_edge[j]]);
			}
		}
	}
}

static u32 EPA_internal_initiate_entries_from_tetrahedron(struct gen_array_list *entries, const struct gjk_simplex * simplex, const f32 abs_tol)
{
	u64 gen_index;

	/* front face = CCW, front face is facing away from origin */
	vec3 AB, AC, AD, cross;
	vec3_sub(AB, simplex->p[1], simplex->p[0]);
	vec3_sub(AC, simplex->p[2], simplex->p[0]);
	vec3_sub(AD, simplex->p[3], simplex->p[0]);
	
	vec3_cross(cross, AB, AC);
	if (vec3_dot(cross, AD) < 0.0f)
	{
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[0], simplex->id[1], simplex->id[2], simplex->p[0], simplex->p[1], simplex->p[2], 1, 3, 2, 2, 2, 0);
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[0], simplex->id[3], simplex->id[1], simplex->p[0], simplex->p[3], simplex->p[1], 2, 3, 0, 2, 0, 0);
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[0], simplex->id[2], simplex->id[3], simplex->p[0], simplex->p[2], simplex->p[3], 0, 3, 1, 2, 1, 0);
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[1], simplex->id[3], simplex->id[2], simplex->p[1], simplex->p[3], simplex->p[2], 1, 2, 0, 1, 1, 1);
	}
	else
	{
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[0], simplex->id[2], simplex->id[1], simplex->p[0], simplex->p[2], simplex->p[1], 2, 3, 1, 2, 0, 0);
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[0], simplex->id[1], simplex->id[3], simplex->p[0], simplex->p[1], simplex->p[3], 0, 3, 2, 2, 2, 0);
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[0], simplex->id[3], simplex->id[2], simplex->p[0], simplex->p[3], simplex->p[2], 1, 3, 0, 2, 1, 0);
		EPA_internal_initiate_entry_from_CCW_triangle(gen_array_list_reserve(&gen_index, entries), simplex->id[1], simplex->id[2], simplex->id[3], simplex->p[1], simplex->p[2], simplex->p[3], 0, 2, 1, 1, 1, 1);
	}


	u32 internal = 4;
	u32 valid = 1;
	const f32 tol = abs_tol * abs_tol;
	for (u32 i = 0; i < 4; ++i)
	{
		struct EPA_entry *e = gen_array_list_address(entries, i);
		if (e->distance_sq < tol)
		{
			valid = 0;
			break;
		}

		if (e->lambda[0] < 0.0f || e->lambda[1] < 0.0f || e->lambda[2] < 0.0f)
		{
			internal -= 1;
		}
	}

	EPA_ASSERT_VALID_ENTRIES(entries);
	return valid*internal;
}

struct horizon
{
	u16 *sentry;
	u8 *sentry_edge;
	u16 count;
};

static struct horizon EPA_internal_horizon(struct arena *mem, struct gen_array_list *entries, const u64 entry_start, const vec3 support)
{
	struct horizon horizon =
	{
		.count = 0,
		.sentry = arena_push(mem, NULL, entries->count * sizeof(u16)),
		.sentry_edge = arena_push(mem, NULL, entries->count * sizeof(u8)),
	};

	u16 remove_count = 0;
	u16 *to_remove = arena_push(mem, NULL, entries->count * sizeof(u16));
	u16 stack_count = 3;
	u16 *stack_adjacent = arena_push(mem, NULL, 3 * entries->count * sizeof(u16));
	u8 *stack_adjacent_twin_edge = arena_push(mem, NULL, 3 * entries->count * sizeof(u8));

	/* Keep the entries in CCW */
	struct EPA_entry *entry = gen_array_list_address(entries, entry_start);
	stack_adjacent[0] = entry->adjacent[2];
	stack_adjacent[1] = entry->adjacent[1];
	stack_adjacent[2] = entry->adjacent[0];
	stack_adjacent_twin_edge[0] = entry->twin_edge[2];
	stack_adjacent_twin_edge[1] = entry->twin_edge[1];
	stack_adjacent_twin_edge[2] = entry->twin_edge[0];

	while (stack_count--)
	{
		const u16 adj_i = stack_adjacent[stack_count];
		const u16 adj_twin_edge = stack_adjacent_twin_edge[stack_count];
		struct EPA_entry *adjacent = gen_array_list_address(entries, adj_i);
		if (adjacent->valid)
		{
			/* entry is not visible from support */
			if (vec3_dot(adjacent->closest_point, support) < adjacent->distance_sq)
			{
				horizon.sentry[horizon.count] = adj_i;
				horizon.sentry_edge[horizon.count] = adj_twin_edge;
				horizon.count += 1;
			}
			else
			{
				adjacent->valid = 0;
				to_remove[remove_count++] = adj_i;
				stack_adjacent[stack_count] = adjacent->adjacent[(adj_twin_edge + 2) % 3];
				stack_adjacent[stack_count + 1] = adjacent->adjacent[(adj_twin_edge + 1) % 3];
				stack_adjacent_twin_edge[stack_count] = adjacent->twin_edge[(adj_twin_edge + 2) % 3];
				stack_adjacent_twin_edge[stack_count + 1] = adjacent->twin_edge[(adj_twin_edge + 1) % 3];
				stack_count += 2;
			}
		}
	}

	for (u16 i = 0; i < remove_count; ++i)
	{
		gen_array_list_remove_index(entries, to_remove[i]);
	}

	return horizon;
}

u32 GJK_EPA(struct arena *mem, struct contact_manifold *c_m, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2, const f32 rel_tol, const f32 abs_tol)
{
	struct arena record = *mem;
	struct gjk_simplex simplex;
	vec3 c_1, c_2;
	u32 collision;

	if (GJK_distance_internal(&simplex, c_1, c_2, pos_1, vs_1, n_1, pos_2, vs_2, n_2, rel_tol, abs_tol) == 0.0f)
	{
		collision = 1;
		struct gen_array_list *entries = gen_array_list_new(mem, (4 + EPA_MAX_ITERATIONS * 2), sizeof(struct EPA_entry));
		if (EPA_internal_setup_tetrahedron(&simplex, pos_1, vs_1, n_1, pos_2, vs_2, n_2) && EPA_internal_initiate_entries_from_tetrahedron(entries, &simplex, abs_tol)) 
		{ 
			struct min_heap *heap = min_heap_new(mem, 4 + EPA_MAX_ITERATIONS * 2);

			/* (2) push entries with internal closest point to origin onto min queue */
			struct EPA_entry *entry;
			printf("--- INITIAL ENTRIES ---\n");
			for (u32 i = 0; i < 4; ++i)
			{
				entry = gen_array_list_address(entries, i);
				EPA_entry_print(entry);
				if (entry->lambda[0] > 0.0f && entry->lambda[1] > 0.0f && entry->lambda[2] > 0.0f)
				{
					min_heap_push(heap, entry->distance_sq, i);
				}
			}
			assert(heap->count > 0 && "heap count should be larger than 0");

			const f32 rel = (1.0f + rel_tol) * (1.0f + rel_tol);
			f32 pen_depth_sq_upper_bound = FLT_MAX;
			vec3 support;
			for (u32 i = 0; heap->count > 0 && i < EPA_MAX_ITERATIONS; ++i)
			{
				EPA_ASSERT_VALID_ENTRIES(entries);

				min_heap_print(stdout, heap);
				const u64 best_index = min_heap_pop(heap);
				entry = gen_array_list_generation_address(entries, best_index);
				if (!entry) { continue; }

				printf("---- SMALLEST VALID ----\n");
				EPA_entry_print(entry);

				const u64 support_id = convex_minkowski_difference_world_support(support, entry->closest_point, pos_1, vs_1, n_1, pos_2, vs_2, n_2);
				const f32 dot = vec3_dot(support, entry->closest_point);
				pen_depth_sq_upper_bound = fmin(pen_depth_sq_upper_bound, dot*dot / entry->distance_sq);
			
				if (pen_depth_sq_upper_bound <= rel * entry->distance_sq) { break; }	

				entry->valid = 0;
				const struct arena tmp_storage = *mem;
				{
					// (4) flood fill in CCW order of triangles to get horizon of w
					const struct horizon horizon = EPA_internal_horizon(mem, entries, best_index, support);	
					u64 *indices = arena_push(mem, NULL, horizon.count * sizeof(u64));
					for (u32 j = 0; j < horizon.count; ++j) 
					{ 
						gen_array_list_reserve(indices + j, entries); 
					}
					
					for (u32 j = 0; j < horizon.count; ++j)
					{

						struct EPA_entry *sentry = gen_array_list_address(entries, horizon.sentry[j]);
						struct EPA_entry *new_entry = gen_array_list_address(entries, indices[j]);

						const u8 A_i = (horizon.sentry_edge[j] + 1) % 3;
						const u8 B_i = (horizon.sentry_edge[j] + 0) % 3;

						f32 determinant = EPA_internal_initiate_entry_from_CCW_triangle(new_entry,
								sentry->id[A_i],
								sentry->id[B_i],
								support_id,
								sentry->ABC[A_i], 
								sentry->ABC[B_i],
								support, 
							       	horizon.sentry[j],
							       	(u16) indices[(j+1) % horizon.count], 
								(u16) indices[(horizon.count+j-1) % horizon.count], 
								horizon.sentry_edge[j],
							       	2, 
								1);

						sentry->adjacent[horizon.sentry_edge[j]] = (u16) indices[j];
						sentry->twin_edge[horizon.sentry_edge[j]] = 0;

						/* Bad construction, triangle is either affinely dependent or CW */
						if (determinant <= 0.0f)
						{
							goto EPA_END;
						}

						if (new_entry->lambda[0] > 0.0f && new_entry->lambda[1] > 0.0f && new_entry->lambda[2] > 0.0f 
								&& entry->distance_sq <= new_entry->distance_sq && new_entry->distance_sq <= pen_depth_sq_upper_bound)
						{
							printf("---- ADDED ----\n");
							EPA_entry_print(new_entry);
							min_heap_push(heap, new_entry->distance_sq, indices[j]);
						}
					}
				}

				struct EPA_entry *next_best = NULL;
				while (heap->count && NULL == (next_best = gen_array_list_generation_address(entries, min_heap_peek(heap))))
				{
					min_heap_pop(heap);
				}

				/*
				 * (5) If next best triangle is further away that pen_depth, we have skipped 
				 * the best candidate, so we immediately return using the current pen_depth guess.
				 */
				if (next_best && pen_depth_sq_upper_bound < next_best->distance_sq)
				{
					break;
				}
				
				gen_array_list_remove_index(entries, best_index);
				*mem = tmp_storage;
			}

			EPA_END:	
			printf("---FINAL--\n");
			EPA_entry_print(entry);
			c_m->penetration_depth = sqrtf(entry->distance_sq);
			vec3_copy(c_m->p_1, pos_1);
			vec3_copy(c_m->p_2, pos_2);
			vec3 v_1, v_2;
			for (u32 i = 0; i < 3; ++i)
			{
				vec3_scale(v_1, vs_1[entry->id[i] >> 32], entry->lambda[i]);
				vec3_scale(v_2, vs_2[entry->id[i] & 0xffffffff], entry->lambda[i]);
				vec3_translate(c_m->p_1, v_1);
				vec3_translate(c_m->p_2, v_2);
			}	
		}
		else
		{
			printf("-------------------UNVALID TETRAHEDRON IN GJK_EPA\n");	
			vec3_set(c_m->p_1, 0.0f, 0.0f, 0.0f);
			vec3_set(c_m->p_2, 0.0f, 0.0f, 0.0f);
			c_m->penetration_depth = 0.0f;
			collision = 0;
		}
	}
	else
	{
		collision = 0;
	}

	*mem = record;
	return collision;
}

u32 GJKC_internal_closet_sub_simplex(vec3 simplex[4], vec3 dir, u32 *simplex_type, const f32 error_bound)
{
	switch (*simplex_type)
	{
		case SIMPLEX_1:
		{
			vec3 AB, AO;
			vec3_sub(AB, simplex[0], simplex[1]);
			vec3_scale(AO, simplex[1], -1.0f);
			vec3_triple_product(dir, AB, AO, AB);
			*simplex_type = 1;
			break;
		}
		case SIMPLEX_2:
		{
			vec3 N, AB, AC, AO;
			vec3_scale(AO, simplex[2], -1.0f);	
			vec3_sub(AB, simplex[0], simplex[2]);	
			vec3_sub(AC, simplex[1], simplex[2]);	
			vec3_cross(N, AB, AC);
			vec3_cross(dir, AB, N);

			/* Check infront AB Segment */
			if (vec3_dot(dir, AO) > 0.0f)
			{
				vec3_copy(simplex[1], simplex[2]);
				vec3_triple_product(dir, AB, AO, AB);
				*simplex_type = SIMPLEX_1;	
			}
			else
			{
				vec3_cross(dir, N, AC);
				/* Check infront AC Segment */
				if (vec3_dot(dir, AO) > 0.0f)
				{
					vec3_copy(simplex[0], simplex[2]);
					vec3_triple_product(dir, AC, AO, AC);
					*simplex_type = SIMPLEX_1;	
				}
				/**
				 * Inside triangle, check above / below (triangle is CCW, BCA). The triangle
				 * returned will be CCW relative to the origin, such that new planes in tetrahedron
				 * case will point away from origin.
				 */
				else
				{
					/* Above triangle, return CCW BCA (relative to O) */
					if (vec3_dot(N, AO) > 0.0f)
					{
						vec3_copy(dir, N);
						*simplex_type = SIMPLEX_2;	
					}
					/* Below triangle, return CCW CBA (relative to O) */
					else
					{
						vec3_copy(dir, simplex[0]);
						vec3_copy(simplex[0], simplex[1]);
						vec3_copy(simplex[1], dir);
						vec3_scale(dir, N, -1.0f);
						*simplex_type = SIMPLEX_2;	
					}
				}	
			}

			break;
		}
		case SIMPLEX_3:
		{
			vec3 BAD, AB, AD, AO;
			vec3_scale(AO, simplex[3], -1.0f);	
			vec3_sub(AB, simplex[0], simplex[3]);	
			vec3_sub(AD, simplex[2], simplex[3]);	
			vec3_cross(BAD, AD, AB);

			/* Check if origin is infront of BAD */
			if (vec3_dot(BAD, AO) > 0.0f)
			{
				vec3 DAC, AC;
				vec3_cross(DAC, AC, AD);
				/* Check if origin is infront of DAC */
				if (vec3_dot(DAC, AO) > 0.0f)
				{
					/* BAD, DAC, AD possible */
					vec3 BAD_2;
					vec3_cross(BAD_2, AD, BAD);
					/* Check segment DAC, AD */
					if (vec3_dot(BAD_2, AO) > 0.0f)
					{
						vec3 DAC_1;
						vec3_cross(DAC_1, DAC, AD);
						/* AD Closest */
						if (vec3_dot(DAC_1, AO) > 0.0f)
						{
							vec3_triple_product(dir, AD, AO, AD);
							vec3_copy(simplex[0], simplex[2]);
							vec3_copy(simplex[1], simplex[3]);
							*simplex_type = SIMPLEX_1;
						}
						/* Face DAC closest */
						else
						{
							vec3_copy(dir, DAC);
							/* CDA */
							vec3_copy(simplex[0], simplex[1]);
							vec3_copy(simplex[1], simplex[2]);
							vec3_copy(simplex[2], simplex[3]);
							*simplex_type = SIMPLEX_2;
						}
					}
					/* Face BAD closest */
					else
					{
						vec3_copy(dir, BAD);
						/* BAD */
						vec3_copy(simplex[1], simplex[3]);
						*simplex_type = SIMPLEX_2;
					}
				}
				else
				{
					vec3 BAD_2;
					vec3_cross(BAD_2, AD, BAD);
					/* Check segment AD, see wall of text below... */
					if (vec3_dot(BAD_2, AO) > 0.0f)
					{
						/* AD was closest */
						vec3_triple_product(dir, AD, AO, AD);
						vec3_copy(simplex[0], simplex[2]);
						vec3_copy(simplex[1], simplex[3]);
						*simplex_type = SIMPLEX_1;
					}
					else
					{
						/* Either BAD, CAB, AB left, we check BAD */
						vec3 BAD_1;
						vec3_cross(BAD_1, BAD, AB);
						/* face CAB or AB closest */
						if (vec3_dot(BAD_1, AO) > 0.0f)
						{
							vec3 CAB, CAB_2;
							vec3_cross(CAB, AB, AC);
							vec3_cross(CAB_2, AB, CAB);
							/* face AB closest */
							if (vec3_dot(CAB_2, AO) > 0.0f)
							{
								vec3_triple_product(dir, AB, AO, AB);
								vec3_copy(simplex[1], simplex[3]);
								*simplex_type = SIMPLEX_1;
							}
							/* face CAB closest */
							else
							{
								vec3_copy(dir, CAB);
								/* BCA */
								vec3_copy(simplex[2], simplex[3]);
								*simplex_type = SIMPLEX_2;
							}
						}
						/* face BAD closest */
						else
						{
							vec3_copy(dir, BAD);
							/* BAD */
							vec3_copy(simplex[1], simplex[3]);
							*simplex_type = SIMPLEX_2;
						}
					}
				}
			}
			else
			{
				vec3 CAB, AC;
				vec3_sub(AC, simplex[1], simplex[3]);	
				vec3_cross(CAB, AB, AC);
				/* Check if origin is infront of CAB */
				if (vec3_dot(CAB, AO) > 0.0f)
				{
					vec3 CAB_2;
					vec3_cross(CAB_2, AB, CAB);
					/* Check if segment AB */
					if (vec3_dot(CAB_2, AO) > 0.0f)
					{
						/**
						 * Since the origin lies infront of CAB, but not of BAD, 
						 * if it lies in the direction of the segment AB  splitting
						 * CAB and BAD, the segment must be closest, as CAB and AC
						 * cannot possibly be closer. For the remaining face DAC to
						 * be possibly be closer, the origin must be infront of DAC,
						 * but the origin necessarily has to be infront of CAB
						 * as well, but away in the direction towards AB.
						 */
						vec3_triple_product(dir, AB, AO, AB);
						vec3_copy(simplex[1], simplex[3]);
						*simplex_type = SIMPLEX_1;
					}
					else
					{
						vec3 CAB_1;
						vec3_cross(CAB_1, CAB, AC);
						if (vec3_dot(CAB_1, AO) > 0.0f)
						{
							/* Either DAC or AC left */
							vec3 DAC, DAC_2;
							vec3_cross(DAC, AC, AD);
							vec3_cross(DAC_2, AC, DAC);
							/* Check if segment AC */
							if (vec3_dot(DAC_2, AO) > 0.0f)
							{
								vec3_triple_product(dir, AC, AO, AC);
								vec3_copy(simplex[0], simplex[3]);
								*simplex_type = SIMPLEX_1;
							}
							else
							{
								vec3_copy(dir, DAC);
								/* CDA */
								vec3_copy(simplex[0], simplex[1]);
								vec3_copy(simplex[1], simplex[2]);
								vec3_copy(simplex[2], simplex[3]);
								*simplex_type = SIMPLEX_2;
							}
						}
						else
						{
							/* The origin lies fully infront of triangle CAB */
							vec3_copy(dir, CAB);
							/* BCA */
							vec3_copy(simplex[2], simplex[3]);
							*simplex_type = SIMPLEX_2;
						}
					}
				}
				else
				{
					vec3 DAC;
					vec3_cross(DAC, AC, AD);
					/* Check if origin is infront of DAC */
					if (vec3_dot(DAC, AO) > 0.0f)
					{
						/**
						 * At this point, we know no other faces has the origin infront
						 * of them, so we only have check the neighboring segments AD,
						 * AC of DAC.
						 */	
						vec3 DAC_1;
						vec3_cross(DAC_1, DAC, AD);
						if (vec3_dot(DAC_1, AO) > 0.0f)
						{
							/* AD is closest to the origin */
							vec3_triple_product(dir, AD, AO, AD);
							vec3_copy(simplex[0], simplex[2]);
							vec3_copy(simplex[1], simplex[3]);
							*simplex_type = SIMPLEX_1;
						}
						else
						{
							vec3 DAC_2;
							vec3_cross(DAC_2, AC, DAC);
							if (vec3_dot(DAC_2, AO) > 0.0f)
							{
								/* AC is closest to the origin */
								vec3_triple_product(dir, AC, AO, AC);
								vec3_copy(simplex[0], simplex[3]);
								*simplex_type = SIMPLEX_1;
							}
							else
							{
								/* DAC is closest to the origin */
								vec3_copy(dir, DAC);
								/* CDA */
								vec3_copy(simplex[0], simplex[1]);
								vec3_copy(simplex[1], simplex[2]);
								vec3_copy(simplex[2], simplex[3]);
								*simplex_type = SIMPLEX_2;
							}
						}
					}
					/* The origin is behind all face planes, intersecting the tetrahedron. */
					else
					{
						return 1;
					}
				}
			}

			break;
		}
	}

	return 0;
}

u32 GJKC_test(const f32 *vs_1, const u32 n_1, const f32 *vs_2, const u32 n_2, const f32 tol)
{
	vec3 dir, simplex[4];
	u32 simplex_type = SIMPLEX_0;
	const f32 error_bound = 0.001f;

	/* Get initial point */
	vec3_set(dir, 1.0f, 0.0f, 0.0f);
	convex_minkowski_difference_support(simplex[0], dir, (vec3ptr) vs_1, n_1, (vec3ptr) vs_2, n_2);
	vec3_scale(dir, simplex[0], -1.0f);
	
	const u32 max_iter = 100;
	for (u32 i = 0; i < max_iter; ++i)
	//for(;;)
	{
		simplex_type += 1;
		/* (1) Add support */
		convex_minkowski_difference_support(simplex[simplex_type], dir, (vec3ptr) vs_1, n_1, (vec3ptr) vs_2, n_2);
		//for (u32 i = 0; i < simplex_type+1;++i)
		//{
		//	printf("[%i]", i);
		//	vec3_print("", simplex[i]);
		//}

		/**
		 * (2) If no point on the Minkowski difference gives a non-negative dot product with the new support
		 * then no point passes or is equal to 0 in that direction. Note that since we search in the
		 * opposite direction from the closest point on Convex(V_k) to the origin, the distance from any
		 * iteration simplex to the origin will always be strictly decreasing.
		 * TODO: Here we should use an error bound..? try to derive using Goldberg...
		 */
		if (vec3_dot(simplex[simplex_type], dir) < error_bound) { return 0; }

		/**
		 * (3) Find new support direction from V_{k+1} by analyzing it as a 
		 * point/segment/triangle/tetrahedron. Simplex is set to the closest sub-simplex to the origin.
		 * TODO: error bounds ...
		 */
		if (GJKC_internal_closet_sub_simplex(simplex, dir, &simplex_type, error_bound)) { return 1; }
	}

	return 0;
}

u32 GJKC_world_test(const vec3 pos_1, const f32 *vs_1, const u32 n_1, const vec3 pos_2, const f32 *vs_2, const u32 n_2, const f32 tol)
{
	vec3 dir, simplex[4];
	u32 simplex_type = SIMPLEX_0;
	const f32 error_bound = 0.001f;

	/* Get initial point */
	vec3_set(dir, 1.0f, 0.0f, 0.0f);
	convex_minkowski_difference_world_support(simplex[0], dir, pos_1, (vec3ptr) vs_1, n_1, pos_2, (vec3ptr) vs_2, n_2);
	vec3_scale(dir, simplex[0], -1.0f);
	
	const u32 max_iter = 100;
	for (u32 i = 0; i < max_iter; ++i)
	//for(;;)
	{
		simplex_type += 1;
		/* (1) Add support */
		convex_minkowski_difference_world_support(simplex[simplex_type], dir, pos_1, (vec3ptr) vs_1, n_1, pos_2, (vec3ptr) vs_2, n_2);
		//for (u32 i = 0; i < simplex_type+1;++i)
		//{
		//	printf("[%i]", i);
		//	vec3_print("", simplex[i]);
		//}

		/**
		 * (2) If no point on the Minkowski difference gives a non-negative dot product with the new support
		 * then no point passes or is equal to 0 in that direction. Note that since we search in the
		 * opposite direction from the closest point on Convex(V_k) to the origin, the distance from any
		 * iteration simplex to the origin will always be strictly decreasing.
		 * TODO: Here we should use an error bound..? try to derive using Goldberg...
		 */
		if (vec3_dot(simplex[simplex_type], dir) < error_bound) { return 0; }

		/**
		 * (3) Find new support direction from V_{k+1} by analyzing it as a 
		 * point/segment/triangle/tetrahedron. Simplex is set to the closest sub-simplex to the origin.
		 * TODO: error bounds ...
		 */
		if (GJKC_internal_closet_sub_simplex(simplex, dir, &simplex_type, error_bound)) { return 1; }
	}

	return 0;
}

/**************************************************************/

void *callback_spawn_mgl_line(void *data)
{
	u64 offset = 0;
	struct arena *arena = *GET_ADDRESS_AND_OFFSET(data, struct arena **, sizeof(struct arena *), &offset);
	const f32 *origin = GET_ADDRESS_AND_OFFSET(data, f32 *, sizeof(vec3), &offset);
	const f32 *direction = GET_ADDRESS_AND_OFFSET(data, f32 *, sizeof(vec3), &offset);
	const f32 distance = *GET_ADDRESS_AND_OFFSET(data, f32 *, sizeof(f32), &offset);
	const f32 *color = GET_ADDRESS_AND_OFFSET(data, f32 *, sizeof(vec4), &offset);

	spawn_mgl_line(arena, origin, direction, distance, color);

	return NULL;
}

void spawn_mgl_line(struct arena *arena, const vec3 origin, const vec3 direction, const f32 distance, const vec4 color)
{
	struct mgl_line *line = (struct mgl_line *) arena_push_packed(arena, NULL, sizeof(struct mgl_line));
	vec4_copy(line->c0, color);
	vec4_copy(line->c1, color);
	vec3_copy(line->p0, origin);
	vec3_scale(line->p1, direction, distance);
	vec3_translate(line->p1, origin);
}

static i32 ind_data[] = {
	0, 1, 2, 0, 2, 3,
	3, 2, 5, 3, 5, 4,
	1, 6, 5, 1, 5, 2,
	7, 6, 1, 7, 1, 0,
	4, 5, 6, 4, 6, 7,
	7, 0, 3, 7, 3, 4,
};

void spawn_mgl_cube_AABB(struct drawbuffer *d_buf, const struct AABB *aabb, const vec4 color)
{
	vec3 v = { aabb->center[0] - aabb->hw[0], aabb->center[1] - aabb->hw[1], aabb->center[2] - aabb->hw[2] };
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	v[1] += 2.0f * aabb->hw[1];
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	v[0] += 2.0f * aabb->hw[0];
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	v[1] -= 2.0f * aabb->hw[1];
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	v[2] += 2.0f * aabb->hw[2];
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	v[1] += 2.0f * aabb->hw[1];
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	v[0] -= 2.0f * aabb->hw[0];
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	v[1] -= 2.0f * aabb->hw[1];
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));

	i32 indices[36];
	for (i32 i = 0; i < 36; ++i)
	{
		indices[i] = d_buf->next_index + ind_data[i];
	}
	d_buf->next_index += 8;
	arena_push_packed(&d_buf->i_buf, indices, sizeof(indices));
}

void spawn_mgl_cube_OBB(struct drawbuffer *d_buf, const struct OBB *obb, const vec4 color)
{
	vec3 y_axis;
	vec3_cross(y_axis, obb->z_axis, obb->x_axis);
	mat3 transform;
	mat3_set_columns(transform, obb->x_axis, y_axis, obb->z_axis);

	vec3 v, tmp = { -obb->hw[0], -obb->hw[1], -obb->hw[2] };
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	tmp[1] += 2.0f * obb->hw[1];
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	tmp[0] += 2.0f * obb->hw[0];
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	tmp[1] -= 2.0f * obb->hw[1];
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	tmp[2] += 2.0f * obb->hw[2];
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	tmp[1] += 2.0f * obb->hw[1];
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	tmp[0] -= 2.0f * obb->hw[0];
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	tmp[1] -= 2.0f * obb->hw[1];
	mat3_vec_mul(v, transform, tmp);
	vec3_translate(v, obb->center);
	arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));

	i32 indices[36];
	for (i32 i = 0; i < 36; ++i)
	{
		indices[i] = d_buf->next_index + ind_data[i];
	}
	d_buf->next_index += 8;
	arena_push_packed(&d_buf->i_buf, indices, sizeof(indices));
}

void spawn_mgl_cylinder(struct drawbuffer *d_buf, const struct cylinder *cyl, const i32 refinement, const vec4 color)
{
	if (refinement < 3) { return; }

	vec3 v; 

	const f32 angle_inc = MM_PI_2_F / refinement;
	v[1] = cyl->center[1] + cyl->half_height;
	for (i32 i = 0; i < refinement; ++i)
	{
		v[0] = cosf(angle_inc * i) * cyl->radius + cyl->center[0];
		v[2] = sinf(angle_inc * i) * cyl->radius + cyl->center[2];
		arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
		arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	}
		
	v[1] = cyl->center[1] - cyl->half_height;
	for (i32 i = 0; i < refinement; ++i)
	{
		v[0] = cosf(angle_inc * i) * cyl->radius + cyl->center[0];
		v[2] = sinf(angle_inc * i) * cyl->radius + cyl->center[2];
		arena_push_packed(&d_buf->v_buf, v, sizeof(vec3));
		arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
	}


	i32 index = d_buf->next_index;
	vec3i32 tri;
	for (i32 i = 2; i < refinement; ++i)
	{
		vec3i32_set(tri, index, index+i, index + i - 1);
		arena_push_packed(&d_buf->i_buf, tri, sizeof(tri));
	}
	
	index += refinement;
	for (i32 i = 2; i < refinement; ++i)
	{
		vec3i32_set(tri, index, index + i - 1, index+i);
		arena_push_packed(&d_buf->i_buf, tri, sizeof(tri));
	}

	i32 indices[6];
	for (i32 i = 0; i < refinement; ++i)
	{
		{
		 indices[0] = index + i;
		 indices[1] = index + i - refinement;
		 indices[2] = index + ((i+1) % refinement) - refinement;
		 indices[3] = index + i;
		 indices[4] = index + ((i+1) % refinement) - refinement;
		 indices[5] = index + ((i+1) % refinement);
		}
		arena_push_packed(&d_buf->i_buf, indices, sizeof(indices));
	}

	d_buf->next_index += 2 * refinement;
}

/* const_circle_points - number of vertices on single circle of sphere */
void spawn_mgl_sphere(struct drawbuffer *d_buf, const struct sphere *sph, const i32 refinement, const vec4 color)
{
	if (refinement < 2) { return; }

	vec3 tmp, vertex = { 0.0f, sph->radius, 0.0f };
	vec3_translate(vertex, sph->center);
	arena_push_packed(&d_buf->v_buf, vertex, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));

	const i32 points_per_strip = 2 * refinement;
	const i32 num_strips = refinement;
	const f32 inc_angle = MM_PI_F / refinement;

	for (i32 i = 1; i < num_strips; ++i)
	{
		const f32 k = inc_angle * i;
		for (i32 j = 0; j < points_per_strip; ++j)
		{
			const f32 t = inc_angle * j;
			vec3_set(tmp, cosf(t), 0.0f, -sinf(t));
			vec3_normalize(vertex, tmp);
			vec3_mul_constant(vertex, sinf(k));
			vertex[1] = cosf(k);
			vec3_mul_constant(vertex, sph->radius);
			vec3_translate(vertex, sph->center);
			arena_push_packed(&d_buf->v_buf, vertex, sizeof(vec3));
			if (i % 2 == 0)
			{
				arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
			}
			else
			{
				arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));
			}
		}
	}

	vec3_set(vertex, 0.0f, -sph->radius, 0.0f);
	vec3_translate(vertex, sph->center);
	arena_push_packed(&d_buf->v_buf, vertex, sizeof(vec3));
	arena_push_packed(&d_buf->v_buf, color, sizeof(vec4));

	vec3i32 tri;
	i32 b_i = d_buf->next_index;	

	for (i32 i = 0; i < points_per_strip; ++i)
	{
		vec3i32_set(tri, b_i + 1 + ((i + 1) % points_per_strip), b_i, b_i + i + 1);
		arena_push_packed(&d_buf->i_buf, tri, sizeof(vec3i32));
	}


	b_i += 1;
	for (i32 i = 1; i < (num_strips-1); ++i)
	{
		b_i += points_per_strip;
		for (i32 j = 0; j < points_per_strip; ++j)
		{
			vec3i32_set(tri
					, b_i + ((j + 1) % points_per_strip)
					, b_i + j - points_per_strip
					, b_i + j);
			arena_push_packed(&d_buf->i_buf, tri, sizeof(vec3i32));
			vec3i32_set(tri
					,b_i + ((j + 1) % points_per_strip)
					,b_i + ((j + 1) % points_per_strip) - points_per_strip
					,b_i + j - points_per_strip);
			arena_push_packed(&d_buf->i_buf, tri, sizeof(vec3i32));
		}
	}

	for (i32 i = 0; i < points_per_strip; ++i)
	{
		vec3i32_set(tri, b_i + points_per_strip, b_i + ((i + 1) % points_per_strip), b_i + i);
		arena_push_packed(&d_buf->i_buf, tri, sizeof(vec3i32));
	}
	d_buf->next_index = b_i + points_per_strip + 1;
}

void *callback_spawn_mgl_quad(void *data)
{
	struct drawbuffer **drawbuffer_addr_ptr = (struct drawbuffer **) data;
	const struct mgl_quad *quad = (struct mgl_quad *)((u8 *) data + sizeof(struct drawbuffer *));

	spawn_mgl_quad(*drawbuffer_addr_ptr, quad);

	return NULL;
}

void spawn_mgl_quad(struct drawbuffer *d_buf, const struct mgl_quad *quad)
{
	const u32 m_i = d_buf->next_index;
	d_buf->next_index += 4;
	const u32 indices[6] = {m_i, m_i + 1, m_i + 2, m_i, m_i + 2, m_i + 3};
	arena_push_packed(&d_buf->v_buf, quad, sizeof(struct mgl_quad));
	arena_push_packed(&d_buf->i_buf, indices, sizeof(indices));
}

void AABB_bounding_volume(struct AABB *dst, const vec3ptr v, const u32 v_count, const f32 tol)
{
	vec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
	vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	for (u32 i = 0; i < v_count; ++i)
	{
		min[0] = fminf(min[0], v[i][0]); 
		min[1] = fminf(min[1], v[i][1]);			
		min[2] = fminf(min[2], v[i][2]);			

		max[0] = fmaxf(max[0], v[i][0]);			
		max[1] = fmaxf(max[1], v[i][1]);			
		max[2] = fmaxf(max[2], v[i][2]);			
	}

	vec3_sub(dst->hw, max, min);
	vec3_mul_constant(dst->hw, 0.5f);
	vec3_add(dst->center, min, dst->hw);

	dst->hw[0] += tol;
	dst->hw[1] += tol;
	dst->hw[2] += tol;
}

void AABB_union(struct AABB *box_union, const struct AABB *a, const struct AABB *b)
{
	vec3 min, max;
	
	min[0] = fminf(a->center[0] - a->hw[0], b->center[0] - b->hw[0]);
	min[1] = fminf(a->center[1] - a->hw[1], b->center[1] - b->hw[1]);
	min[2] = fminf(a->center[2] - a->hw[2], b->center[2] - b->hw[2]);
                                                                      
	max[0] = fmaxf(a->center[0] + a->hw[0], b->center[0] + b->hw[0]);
	max[1] = fmaxf(a->center[1] + a->hw[1], b->center[1] + b->hw[1]);
	max[2] = fmaxf(a->center[2] + a->hw[2], b->center[2] + b->hw[2]);
	
	vec3_sub(box_union->hw, max, min);
	vec3_mul_constant(box_union->hw, 0.5f);
	vec3_add(box_union->center, box_union->hw, min);
}


void AABB_push_lines(struct drawbuffer *buf, const struct AABB *box, const vec4 color)
{
	vec3 end;
	vec3_sub(end, box->center, box->hw);

	/* 0, 1, 2, 3 */
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	end[0] += 2.0f*box->hw[0];
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	end[0] -= 2.0f*box->hw[0];
	end[1] += 2.0f*box->hw[1];
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	end[1] -= 2.0f*box->hw[1];
	end[2] += 2.0f*box->hw[2];
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	end[2] -= 2.0f*box->hw[2];
	/* 4, 5 */
	end[0] += 2.0f*box->hw[0];
	end[1] += 2.0f*box->hw[1];
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	end[1] -= 2.0f*box->hw[1];
	end[2] += 2.0f*box->hw[2];
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	/* 6 */
	end[0] -= 2.0f*box->hw[0];
	end[1] += 2.0f*box->hw[1];
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));
	/* 7 */
	end[0] += 2.0f*box->hw[0];
	arena_push_packed(&buf->v_buf, end, sizeof(vec3));
	arena_push_packed(&buf->v_buf, color, sizeof(vec4));

	i32 indices[] = {0,1,0,2,0,3,1,4,1,5,2,4,2,6,3,5,3,6,6,7,5,7,4,7};
	for (u64 i = 0; i < sizeof(indices)/sizeof(i32); ++i)
	{
		indices[i] += buf->next_index;
	}
	buf->next_index += 8;
	arena_push_packed(&buf->i_buf, indices, sizeof(indices));
}

f32 point_plane_signed_distance(const vec3 point, const struct plane *plane)
{
	const vec3 relation_to_plane_point =
       	{ 
		point[0] - plane->normal[0] * plane->signed_distance,
		point[1] - plane->normal[1] * plane->signed_distance,
		point[2] - plane->normal[2] * plane->signed_distance,
	};
	
	return vec3_dot(relation_to_plane_point, plane->normal);
}

f32 point_plane_distance(const vec3 point, const struct plane *plane)
{
	const vec3 relation_to_plane_point =
       	{ 
		point[0] - plane->normal[0] * plane->signed_distance,
		point[1] - plane->normal[1] * plane->signed_distance,
		point[2] - plane->normal[2] * plane->signed_distance,
	};
	
	union f32_bit_representation rep = { .value = vec3_dot(relation_to_plane_point, plane->normal) };
	rep.bits &= 0x7FFFFFFF;	
	return rep.value;
}

void point_plane_closest_point(vec3 closest_point, const vec3 point, const struct plane *plane)
{
	const f32 distance = point_plane_signed_distance(point, plane);	
	vec3_scale(closest_point, plane->normal, -distance);
	vec3_translate(closest_point, point);
}

i32 ray_plane_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct plane *plane)
{

	const f32 dot = vec3_dot(ray_direction, plane->normal);
	if (dot == 0)
       	{
		return 0;
       	}

	const f32 scale = (plane->signed_distance-vec3_dot(ray_origin, plane->normal)) / dot;
	vec3_scale(intersection, ray_direction, scale);
	vec3_translate(intersection, ray_origin);

	union f32_bit_representation rep = { .value = scale };

	return 1 - F32_SIGN_BIT(rep);
}

f32 point_sphere_distance(const vec3 point, const struct sphere *sph)
{
	vec3 tmp;
	vec3_sub(tmp, point, sph->center);
	const f32 distance = vec3_length(tmp) - sph->radius;
	const union f32_bit_representation rep = { .value =  distance };
	return distance - F32_SIGN_BIT(rep) * distance;
}

void point_sphere_closest_point(vec3 closest_point, const vec3 point, const struct sphere *sph)
{
	vec3_sub(closest_point, point, sph->center);
	f32 distance = vec3_length(closest_point) - sph->radius;
	const union f32_bit_representation rep = { .value =  distance };
	distance -= F32_SIGN_BIT(rep) * distance;

	/* Can be removed if we can multiply 0.0f * FLT_INF... ? */
	if (distance == 0.0f)
	{
		vec3_copy(closest_point, point);
	}
	else
	{
		vec3_mul_constant(closest_point, sph->radius / vec3_length(closest_point));
	}
	vec3_translate(closest_point, sph->center);
}

i32 ray_sphere_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct sphere * sph)
{
	vec3 m;
	vec3_sub(m, ray_origin, sph->center);
	const f32 b = vec3_dot(m, ray_direction);
	union f32_bit_representation c = { .value = vec3_dot(m,m) - sph->radius * sph->radius};
	const f32 discr = b*b - c.value;
	if (discr < 0) { return 0; }

	/* -sqrtf() if origin is not within sphere, sqrtf if within. */
	const f32 t = -b - (1.0f - (F32_SIGN_BIT(c) << 1)) * sqrtf(discr); 
	vec3_scale(intersection, ray_direction, t);
	vec3_translate(intersection, ray_origin);
	c.value = t;
	return 1 - F32_SIGN_BIT(c);
}

i32 AABB_contains(const struct AABB *a, const struct AABB *b)
{
	if (b->center[0] - b->hw[0] < a->center[0] - a->hw[0]) { return 0; }
	if (b->center[1] - b->hw[1] < a->center[1] - a->hw[1]) { return 0; }
	if (b->center[2] - b->hw[2] < a->center[2] - a->hw[2]) { return 0; }
	
	if (b->center[0] + b->hw[0] > a->center[0] + a->hw[0]) { return 0; }
	if (b->center[1] + b->hw[1] > a->center[1] + a->hw[1]) { return 0; }
	if (b->center[2] + b->hw[2] > a->center[2] + a->hw[2]) { return 0; }

	return 1;
}

f32 point_AABB_distance(const vec3 point, const struct AABB *aabb)
{
	union f32_bit_representation x,y,z;

	x.value = point[0] - aabb->center[0];
	y.value = point[1] - aabb->center[1];
	z.value = point[2] - aabb->center[2];

	x.bits &= 0x7FFFFFFF;
	y.bits &= 0x7FFFFFFF;
	z.bits &= 0x7FFFFFFF;

	x.value -= aabb->hw[0]; 
	y.value -= aabb->hw[1];
	z.value -= aabb->hw[2];

	x.value *= (1 - F32_SIGN_BIT(x));  
	y.value *= (1 - F32_SIGN_BIT(y)); 
	z.value *= (1 - F32_SIGN_BIT(z)); 

	return sqrtf(x.value*x.value + y.value*y.value + z.value*z.value);
}

f32 point_OBB_distance(const vec3 point, const struct OBB *obb)
{
	vec3 y_axis;
	vec3_cross(y_axis, obb->z_axis, obb->x_axis);
	mat3 transform;
	mat3_set_rows(transform, obb->x_axis, y_axis, obb->z_axis);

	vec3 tmp, point_obb_space;
	vec3_sub(tmp, point, obb->center);
	mat3_vec_mul(point_obb_space, transform, tmp);

	union f32_bit_representation x,y,z;

	x.value = point_obb_space[0];
	y.value = point_obb_space[1];
	z.value = point_obb_space[2];

	x.bits &= 0x7FFFFFFF;
	y.bits &= 0x7FFFFFFF;
	z.bits &= 0x7FFFFFFF;

	x.value -= obb->hw[0]; 
	y.value -= obb->hw[1];
	z.value -= obb->hw[2];

	x.value *= (1 - F32_SIGN_BIT(x));  
	y.value *= (1 - F32_SIGN_BIT(y)); 
	z.value *= (1 - F32_SIGN_BIT(z)); 

	return sqrtf(x.value*x.value + y.value*y.value + z.value*z.value);
}

void point_AABB_closest_point(vec3 closest_point, const vec3 point, const struct AABB *aabb)
{
	union f32_bit_representation x,y,z;

	x.value = point[0] - aabb->center[0];
	y.value = point[1] - aabb->center[1];
	z.value = point[2] - aabb->center[2];

	const vec3 sign = { -2.0f * F32_SIGN_BIT(x) + 1.0f, -2.0f * F32_SIGN_BIT(y) + 1.0f,  -2.0f * F32_SIGN_BIT(z) + 1.0f };

	x.bits &= 0x7FFFFFFF;
	y.bits &= 0x7FFFFFFF;
	z.bits &= 0x7FFFFFFF;

	x.value -= aabb->hw[0]; 
	y.value -= aabb->hw[1];
	z.value -= aabb->hw[2];

	vec3_set(closest_point
	 ,point[0]*(F32_SIGN_BIT(x)) + (1.0f - F32_SIGN_BIT(x)) * (aabb->center[0] + sign[0] * aabb->hw[0])  
	 ,point[1]*(F32_SIGN_BIT(y)) + (1.0f - F32_SIGN_BIT(y)) * (aabb->center[1] + sign[1] * aabb->hw[1]) 
	 ,point[2]*(F32_SIGN_BIT(z)) + (1.0f - F32_SIGN_BIT(z)) * (aabb->center[2] + sign[2] * aabb->hw[2]));
}

void point_OBB_closest_point(vec3 closest_point, const vec3 point, const struct OBB *obb)
{
	vec3 y_axis;
	vec3_cross(y_axis, obb->z_axis, obb->x_axis);
	mat3 transform;
	mat3_set_rows(transform, obb->x_axis, y_axis, obb->z_axis);

	vec3 tmp, point_obb_space;
	vec3_sub(tmp, point, obb->center);
	mat3_vec_mul(point_obb_space, transform, tmp);

	union f32_bit_representation x,y,z;

	x.value = point_obb_space[0];
	y.value = point_obb_space[1];
	z.value = point_obb_space[2];

	const vec3 sign = { -2.0f * F32_SIGN_BIT(x) + 1.0f, -2.0f * F32_SIGN_BIT(y) + 1.0f,  -2.0f * F32_SIGN_BIT(z) + 1.0f };

	x.bits &= 0x7FFFFFFF;
	y.bits &= 0x7FFFFFFF;
	z.bits &= 0x7FFFFFFF;

	x.value -= obb->hw[0]; 
	y.value -= obb->hw[1];
	z.value -= obb->hw[2];

	vec3_set(tmp
	 ,point_obb_space[0]*(F32_SIGN_BIT(x)) + (1.0f - F32_SIGN_BIT(x)) * sign[0] * obb->hw[0] 
	 ,point_obb_space[1]*(F32_SIGN_BIT(y)) + (1.0f - F32_SIGN_BIT(y)) * sign[1] * obb->hw[1] 
	 ,point_obb_space[2]*(F32_SIGN_BIT(z)) + (1.0f - F32_SIGN_BIT(z)) * sign[2] * obb->hw[2]);
	
	mat3_set_columns(transform, obb->x_axis, y_axis, obb->z_axis);
	mat3_vec_mul(closest_point, transform, tmp);
	vec3_translate(closest_point, obb->center);
}

i32 ray_AABB_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct AABB *aabb)
{
	const vec3 p = { ray_origin[0] - aabb->center[0], ray_origin[1] - aabb->center[1], ray_origin[2] - aabb->center[2] };

	union f32_bit_representation x,y,z;
	x.value = p[0];
	y.value = p[1];
	z.value = p[2];
	
	const vec3 sign = { -2.0f * F32_SIGN_BIT(x) + 1.0f, -2.0f * F32_SIGN_BIT(y) + 1.0f,  -2.0f * F32_SIGN_BIT(z) + 1.0f };

	x.bits &= 0x7FFFFFFF;
	y.bits &= 0x7FFFFFFF;
	z.bits &= 0x7FFFFFFF;

	x.value -= aabb->hw[0]; 
	y.value -= aabb->hw[1];
	z.value -= aabb->hw[2];

	const i32 sx = F32_SIGN_BIT(x);
	const i32 sy = F32_SIGN_BIT(y);
	const i32 sz = F32_SIGN_BIT(z);

	/* Check planes towards */
	if (sx + sy + sz == 3)
	{
		x.value = ray_direction[0];
		struct plane plane;
		vec3_set(plane.normal, -2.0f * F32_SIGN_BIT(x) + 1.0f, 0.0f, 0.0f);
		plane.signed_distance = aabb->hw[0];
		if (ray_direction[0] != 0.0f
			         && ray_plane_intersection(intersection, p, ray_direction, &plane)
				 && (intersection[1] >= -aabb->hw[1] && intersection[1] <= aabb->hw[1])
				 && (intersection[2] >= -aabb->hw[2] && intersection[2] <= aabb->hw[2])
				)
		{
			vec3_translate(intersection, aabb->center);
			return 1;	
		}

		y.value = ray_direction[1];
		vec3_set(plane.normal, 0.0f, -2.0f * F32_SIGN_BIT(y) + 1.0f, 0.0f);
		plane.signed_distance = aabb->hw[1];
		if (ray_direction[1] != 0.0f
				 && ray_plane_intersection(intersection, p, ray_direction, &plane)
				 && (intersection[0] >= -aabb->hw[0] && intersection[0] <= aabb->hw[0])
				 && (intersection[2] >= -aabb->hw[2] && intersection[2] <= aabb->hw[2])
				)
		{
			vec3_translate(intersection, aabb->center);
			return 1;	
		}

		z.value = ray_direction[2];
		vec3_set(plane.normal, 0.0f, 0.0f, -2.0f * F32_SIGN_BIT(z) + 1.0f);
		plane.signed_distance = aabb->hw[2];
		if (ray_direction[2] != 0.0f
				 && ray_plane_intersection(intersection, p, ray_direction, &plane)
				 && (intersection[0] >= -aabb->hw[0] && intersection[0] <= aabb->hw[0])
				 && (intersection[1] >= -aabb->hw[1] && intersection[1] <= aabb->hw[1])
				)
		{
			vec3_translate(intersection, aabb->center);
			return 1;	
		}

	}
	/* check planes against */
	else
	{
		struct plane plane;
		vec3_set(plane.normal, sign[0], 0.0f, 0.0f);
		plane.signed_distance = aabb->hw[0];
		if (sx == 0 && ray_plane_intersection(intersection, p, ray_direction, &plane)
				 && (intersection[1] >= -aabb->hw[1] && intersection[1] <= aabb->hw[1])
				 && (intersection[2] >= -aabb->hw[2] && intersection[2] <= aabb->hw[2])
				)
		{
			vec3_translate(intersection, aabb->center);
			return 1;	
		}

		vec3_set(plane.normal, 0.0f, sign[1], 0.0f);
		plane.signed_distance = aabb->hw[1];
		if (sy == 0 && ray_plane_intersection(intersection, p, ray_direction, &plane)
				 && (intersection[0] >= -aabb->hw[0] && intersection[0] <= aabb->hw[0])
				 && (intersection[2] >= -aabb->hw[2] && intersection[2] <= aabb->hw[2])
				)
		{
			vec3_translate(intersection, aabb->center);
			return 1;	
		}

		vec3_set(plane.normal, 0.0f, 0.0f, sign[2]);
		plane.signed_distance = aabb->hw[2];
		if (sz == 0 && ray_plane_intersection(intersection, p, ray_direction, &plane)
				 && (intersection[0] >= -aabb->hw[0] && intersection[0] <= aabb->hw[0])
				 && (intersection[1] >= -aabb->hw[1] && intersection[1] <= aabb->hw[1])
				)
		{
			vec3_translate(intersection, aabb->center);
			return 1;	
		}
	}

	return 0;
}

i32 ray_OBB_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct OBB *obb)
{
	vec3 y_axis;
	vec3_cross(y_axis, obb->z_axis, obb->x_axis);
	mat3 transform;
	mat3_set_rows(transform, obb->x_axis, y_axis, obb->z_axis);

	vec3 tmp, p, ray_direction_obb;
	vec3_sub(tmp, ray_origin, obb->center);
	mat3_vec_mul(p, transform, tmp);
	mat3_vec_mul(ray_direction_obb, transform, ray_direction);

	mat3_set_columns(transform, obb->x_axis, y_axis, obb->z_axis);

	union f32_bit_representation x,y,z;

	x.value = p[0];
	y.value = p[1];
	z.value = p[2];

	const vec3 sign = { -2.0f * F32_SIGN_BIT(x) + 1.0f, -2.0f * F32_SIGN_BIT(y) + 1.0f,  -2.0f * F32_SIGN_BIT(z) + 1.0f };

	x.bits &= 0x7FFFFFFF;
	y.bits &= 0x7FFFFFFF;
	z.bits &= 0x7FFFFFFF;

	x.value -= obb->hw[0]; 
	y.value -= obb->hw[1];
	z.value -= obb->hw[2];

	const i32 sx = F32_SIGN_BIT(x);
	const i32 sy = F32_SIGN_BIT(y);
	const i32 sz = F32_SIGN_BIT(z);

	/* Check planes towards */
	if (sx + sy + sz == 3)
	{
		x.value = ray_direction_obb[0];
		struct plane plane;
		vec3_set(plane.normal, -2.0f * F32_SIGN_BIT(x) + 1.0f, 0.0f, 0.0f);
		plane.signed_distance = obb->hw[0];
		if (ray_direction_obb[0] != 0.0f
			         && ray_plane_intersection(tmp, p, ray_direction_obb, &plane)
				 && (tmp[1] >= -obb->hw[1] && tmp[1] <= obb->hw[1])
				 && (tmp[2] >= -obb->hw[2] && tmp[2] <= obb->hw[2])
				)
		{
			mat3_vec_mul(intersection, transform, tmp);
			vec3_translate(intersection, obb->center);
			return 1;	
		}

		y.value = ray_direction_obb[1];
		vec3_set(plane.normal, 0.0f, -2.0f * F32_SIGN_BIT(y) + 1.0f, 0.0f);
		plane.signed_distance = obb->hw[1];
		if (ray_direction_obb[1] != 0.0f
				 && ray_plane_intersection(tmp, p, ray_direction_obb, &plane)
				 && (tmp[0] >= -obb->hw[0] && tmp[0] <= obb->hw[0])
				 && (tmp[2] >= -obb->hw[2] && tmp[2] <= obb->hw[2])
				)
		{
			mat3_vec_mul(intersection, transform, tmp);
			vec3_translate(intersection, obb->center);
			return 1;	
		}

		z.value = ray_direction_obb[2];
		vec3_set(plane.normal, 0.0f, 0.0f, -2.0f * F32_SIGN_BIT(z) + 1.0f);
		plane.signed_distance = obb->hw[2];
		if (ray_direction_obb[2] != 0.0f
				 && ray_plane_intersection(tmp, p, ray_direction_obb, &plane)
				 && (tmp[0] >= -obb->hw[0] && tmp[0] <= obb->hw[0])
				 && (tmp[1] >= -obb->hw[1] && tmp[1] <= obb->hw[1])
				)
		{
			mat3_vec_mul(intersection, transform, tmp);
			vec3_translate(intersection, obb->center);
			return 1;	
		}

	}
	/* check planes against */
	else
	{
		struct plane plane;
		vec3_set(plane.normal, sign[0], 0.0f, 0.0f);
		plane.signed_distance = obb->hw[0];
		if (sx == 0 && ray_plane_intersection(tmp, p, ray_direction_obb, &plane)
				 && (tmp[1] >= -obb->hw[1] && tmp[1] <= obb->hw[1])
				 && (tmp[2] >= -obb->hw[2] && tmp[2] <= obb->hw[2])
				)
		{
			mat3_vec_mul(intersection, transform, tmp);
			vec3_translate(intersection, obb->center);
			return 1;	
		}

		vec3_set(plane.normal, 0.0f, sign[1], 0.0f);
		plane.signed_distance = obb->hw[1];
		if (sy == 0 && ray_plane_intersection(tmp, p, ray_direction_obb, &plane)
				 && (tmp[0] >= -obb->hw[0] && tmp[0] <= obb->hw[0])
				 && (tmp[2] >= -obb->hw[2] && tmp[2] <= obb->hw[2])
				)
		{
			mat3_vec_mul(intersection, transform, tmp);
			vec3_translate(intersection, obb->center);
			return 1;	
		}

		vec3_set(plane.normal, 0.0f, 0.0f, sign[2]);
		plane.signed_distance = obb->hw[2];
		if (sz == 0 && ray_plane_intersection(tmp, p, ray_direction_obb, &plane)
				 && (tmp[0] >= -obb->hw[0] && tmp[0] <= obb->hw[0])
				 && (tmp[1] >= -obb->hw[1] && tmp[1] <= obb->hw[1])
				)
		{
			mat3_vec_mul(intersection, transform, tmp);
			vec3_translate(intersection, obb->center);
			return 1;	
		}
	}

	return 0;
}

f32 point_cylinder_distance(const vec3 point, const struct cylinder *cyl)
{
	vec3 tmp;
	vec3_sub(tmp, point, cyl->center);
	
	union f32_bit_representation r,y;
	
	r.value = sqrtf(tmp[0]*tmp[0] + tmp[2]*tmp[2]) - cyl->radius;

	y.value = tmp[1];
	y.bits &= 0x7FFFFFFF;
	y.value -= cyl->half_height;

	return sqrtf(y.value*y.value*(1.0f - F32_SIGN_BIT(y)) + r.value*r.value*(1.0f - F32_SIGN_BIT(r)));
}

void point_cylinder_closest_point(vec3 closest_point, const vec3 point, const struct cylinder *cyl)
{
	vec3 tmp;
	vec3_sub(tmp, point, cyl->center);
	
	union f32_bit_representation r,y;
	
	const f32 xz_dist = sqrtf(tmp[0]*tmp[0] + tmp[2]*tmp[2]);
	r.value = xz_dist - cyl->radius;

	y.value = tmp[1];
	const f32 y_sign = 1.0f - 2.0f * F32_SIGN_BIT(y);
	y.bits &= 0x7FFFFFFF;
	y.value -= cyl->half_height;

	closest_point[0] = tmp[0] * F32_SIGN_BIT(r) + (1.0f - F32_SIGN_BIT(r)) * cyl->radius * tmp[0] / xz_dist;
	closest_point[1] = tmp[1] * F32_SIGN_BIT(y) + (1.0f - F32_SIGN_BIT(y)) * cyl->half_height * y_sign;
	closest_point[2] = tmp[2] * F32_SIGN_BIT(r) + (1.0f - F32_SIGN_BIT(r)) * cyl->radius * tmp[2] / xz_dist;
	vec3_translate(closest_point, cyl->center);
}

i32 ray_cylinder_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct cylinder *cyl)
{
	vec3_sub(intersection, ray_origin, cyl->center);

	union f32_bit_representation y, r;
	
	r.value = intersection[0]*intersection[0] + intersection[2]*intersection[2] - cyl->radius*cyl->radius;

	y.value = intersection[1];
	const f32 y_sign = 1.0f - 2.0f * F32_SIGN_BIT(y);
	y.bits &= 0x7FFFFFFF; 
	y.value -= cyl->half_height;

	switch ((F32_SIGN_BIT(y) << 1) + F32_SIGN_BIT(r))
	{
		/* above/below AND infront/behind cylinder  */
		case 0:
		{
			/* ray turned away from surface plane */
			const f32 d = y_sign * ray_direction[1];
			if (d >= 0.0f) { return 0; }
			
			vec3 tmp = { intersection[0], intersection[1], intersection[2] };
			vec3_translate_scaled(tmp, ray_direction, y.value / (-d));
			const union f32_bit_representation r2 = { .value = tmp[0]*tmp[0] + tmp[2]*tmp[2] - cyl->radius*cyl->radius };
			if (F32_SIGN_BIT(r2)) 
			{ 
				vec3_add(intersection, tmp, cyl->center);
				return 1;
		       	}

			if (ray_direction[0] == 0.0f && ray_direction[2] == 0.0f) { return 0; }
			const f32 r_dist = sqrtf(ray_direction[0]*ray_direction[0] + ray_direction[2]*ray_direction[2]);
			const f32 n_d = (intersection[0]*ray_direction[0] + intersection[2]*ray_direction[2]) / r_dist;
			//const f32 n_n = intersection[0]*intersection[0] + intersection[2]*intersection[2];
			const f32 discr = n_d*n_d - r.value;
			if (discr < 0.0f) { return 0; }

			const f32 t = -n_d - sqrtf(discr); 
			vec3_translate_scaled(intersection, ray_direction, t / r_dist);
			y.value = intersection[1];
			y.bits &= 0x7FFFFFFF; 
			y.value -= cyl->half_height;
			vec3_translate(intersection, cyl->center);
			return F32_SIGN_BIT(y);

			return 0;
		}
		/* directly above or below cylinder  */
		case 1:
		{
			/* ray turned away from surface plane */
			const f32 d = y_sign * ray_direction[1];
			if (d >= 0.0f) { return 0; }

			vec3_translate_scaled(intersection, ray_direction, y.value / (-d));
			r.value = intersection[0]*intersection[0] + intersection[2]*intersection[2] - cyl->radius*cyl->radius;
			vec3_translate(intersection, cyl->center);
			return F32_SIGN_BIT(r);
		}
		/* infront of cylinder (same height as the cylinder) */
		case 2:
		{
			if (ray_direction[0] == 0.0f && ray_direction[2] == 0.0f) { return 0; }
			const f32 r_dist = sqrtf(ray_direction[0]*ray_direction[0] + ray_direction[2]*ray_direction[2]);
			const f32 n_d = (intersection[0]*ray_direction[0] + intersection[2]*ray_direction[2]) / r_dist;
			const f32 discr = n_d*n_d - r.value;
			if (discr < 0.0f) { return 0; }

			const f32 t = -n_d - sqrtf(discr); 
			vec3_translate_scaled(intersection, ray_direction, t / r_dist);
			y.value = intersection[1];
			y.bits &= 0x7FFFFFFF; 
			y.value -= cyl->half_height;
			vec3_translate(intersection, cyl->center);
			return F32_SIGN_BIT(y);
		}
		/* inside cylinder */
		case 3:
		{
			union f32_bit_representation t = { .value = FLT_MAX };

			if (ray_origin[1] != 0.0f)
			{
				t.value = ray_direction[1];
				t.value = ((1.0f - F32_SIGN_BIT(t)) * (cyl->half_height - intersection[1]) + F32_SIGN_BIT(t) * (cyl->half_height + intersection[1])) / ((1.0f - 2.0f * F32_SIGN_BIT(t)) * t.value);
			}

			if (ray_direction[0] != 0.0f || ray_direction[2] != 0.0f) 
			{
				const f32 r_dist = sqrtf(ray_direction[0]*ray_direction[0] + ray_direction[2]*ray_direction[2]);
				const f32 n_d = (intersection[0]*ray_direction[0] + intersection[2]*ray_direction[2]) / r_dist;
				const f32 discr = n_d*n_d - r.value;
				assert(discr >= 0.0f);
				t.value = fminf(t.value, (-n_d + sqrtf(discr)) / r_dist);
			}
	
			vec3_translate_scaled(intersection, ray_direction, t.value);
			vec3_translate(intersection, cyl->center);
			return 1;
		}
		default:
		{
			assert(false && "should not happen");
			return 0;
		}
	}
}


f32 AABB_distance(const struct AABB *a, const struct AABB *b)
{
	f32 t, m, dist = 0.0f;

	t = b->center[0] - b->hw[0] - (a->center[0] + a->hw[0]);
	m = a->center[0] - a->hw[0] - (b->center[0] + b->hw[0]);
	if (t > 0.0f || m > 0.0f) { t = fmaxf(t,m); dist += t*t; }

	t = b->center[1] - b->hw[1] - (a->center[1] + a->hw[1]);
	m = a->center[1] - a->hw[1] - (b->center[1] + b->hw[1]);
	if (t > 0.0f || m > 0.0f) { t = fmaxf(t,m); dist += t*t; }

	t = b->center[2] - b->hw[2] - (a->center[2] + a->hw[2]);
	m = a->center[2] - a->hw[2] - (b->center[2] + b->hw[2]);
	if (t > 0.0f || m > 0.0f) { t = fmaxf(t,m); dist += t*t; }

	return dist;
}

i32 AABB_test(const struct AABB *a, const struct AABB *b)
{
	if (b->center[0] - b->hw[0] - (a->center[0] + a->hw[0]) > 0.0f 
			|| a->center[0] - a->hw[0] - (b->center[0] + b->hw[0]) > 0.0f) { return 0; }
	if (b->center[1] - b->hw[1] - (a->center[1] + a->hw[1]) > 0.0f 
			|| a->center[1] - a->hw[1] - (b->center[1] + b->hw[1]) > 0.0f) { return 0; }
	if (b->center[2] - b->hw[2] - (a->center[2] + a->hw[2]) > 0.0f 
			|| a->center[2] - a->hw[2] - (b->center[2] + b->hw[2]) > 0.0f) { return 0; }

	return 1;
}

i32 AABB_intersection(struct AABB *dst, const struct AABB *a, const struct AABB *b)
{
	vec3 interpolation;
	for (i32 i = 0; i < 3; ++i)
	{
		const f32 t = b->center[i] - b->hw[i] - (a->center[i] + a->hw[i]);
		const f32 m = a->center[i] - a->hw[i] - (b->center[i] + b->hw[i]);
		if (t > 0.0f || m > 0.0f) 
		{ 
			return 0;
       		}
		else
		{
			const f32 dist = fabs(a->center[i] - b->center[i]);
			if (dist + a->hw[i] <= b->hw[i]) 
			{ 
				dst->hw[i] = a->hw[i];
				interpolation[i] = 1.0f; 
			}
			else if (dist + b->hw[i] <= a->hw[i])
		       	{
			       	dst->hw[i] = b->hw[i];
				interpolation[i] = 0.0f;
			}
			else  
			{
				dst->hw[i] = (a->hw[i] + b->hw[i] - dist) / 2.0f;
				interpolation[i] = (b->hw[i] - dst->hw[i]) / dist; 
			}
		}
	}

	vec3_interpolate_piecewise(dst->center, a->center, b->center, interpolation);
	assert(dst->hw[0] > 0.0f && dst->hw[1] > 0.0f && dst->hw[2]);
	return 1;

}

f32 cylinder_distance(const struct cylinder *a, const struct cylinder *b)
{
	vec2 dist = { 0.0f, 0.0f };
	f32 m, t = (a->center[0] - b->center[0]) * (a->center[0] - b->center[0])
	       	+ (a->center[2] - b->center[2]) * (a->center[2] - b->center[2]);
	if (t > (a->radius+b->radius)*(a->radius+b->radius)) { dist[0] = sqrtf(t) - a->radius - b->radius; }
	
	m = b->center[1] - b->half_height - (a->center[1] + a->half_height);
	t = a->center[1] - a->half_height - (b->center[1] + b->half_height);
	if (m > 0.0f || t > 0.0f) { dist[1] = fmaxf(m,t); }

	return vec2_length(dist);
}

i32 cylinder_test(const struct cylinder *a, const struct cylinder *b)
{
	f32 m, t = (a->center[0] - b->center[0]) * (a->center[0] - b->center[0])
	       	+ (a->center[2] - b->center[2]) * (a->center[2] - b->center[2])
	       	- (a->radius + b->radius) * (a->radius + b->radius);
	if (t > 0.0f) { return 0; }

	m = b->center[1] - b->half_height - (a->center[1] + a->half_height);
	t = a->center[1] - a->half_height - (b->center[1] + b->half_height);
	if (m > 0.0f || t > 0.0f) { return 0; }

	return 1;
}

//i32 cylinder_intersection(struct tmp *dst, const struct cylinder *a, const struct cylinder *b)
//{
//	return 0;
//}

//f32 sphere_distance(const struct sphere *a, const struct sphere *b)
//{
//	return 0.0f;
//}
//
//i32 sphere_test(const struct sphere *a, const struct sphere *b)
//{
//	return 0;
//}
//
//i32 sphere_intersection(struct tmp *dst, const struct sphere *a, const struct sphere *b)
//{
//	return 0;
//}
//
//f32 OBB_distance(const struct OBB *a, const struct OBB *b)
//{
//	return 0.0f;
//}
//
//i32 OBB_test(const struct OBB *a, const struct OBB *b)
//{
//	return 0;
//}
//
//i32 OBB_intersection(struct tmp *dst, const struct OBB *a, const struct OBB *b)
//{
//	return 0;
//}

u32 tetrahedron_point_test(const vec3 tetra[4], const vec3 p)
{
	vec3 v[4], n;
	f32 d_1, d_2;
	
	vec3_sub(v[0], tetra[0], p);
	vec3_sub(v[1], tetra[1], p);
	vec3_sub(v[2], tetra[2], p);
	vec3_sub(v[3], tetra[3], p);

	vec3_recenter_cross(n, v[0], v[1], v[2]);
	d_1 = vec3_dot(n, v[0]);
	d_2 = vec3_dot(n, v[3]);
	if ( (d_1 < 0.0f && d_2 < 0.0f) || (d_1 > 0.0f && d_2 > 0.0f) ) { return 0; }

	vec3_recenter_cross(n, v[1], v[0], v[3]);
	d_1 = vec3_dot(n, v[1]);
	d_2 = vec3_dot(n, v[2]);
	if ( (d_1 < 0.0f && d_2 < 0.0f) || (d_1 > 0.0f && d_2 > 0.0f) ) { return 0; }

	vec3_recenter_cross(n, v[2], v[0], v[3]);
	d_1 = vec3_dot(n, v[2]);
	d_2 = vec3_dot(n, v[1]);
	if ( (d_1 < 0.0f && d_2 < 0.0f) || (d_1 > 0.0f && d_2 > 0.0f) ) { return 0; }

	vec3_recenter_cross(n, v[3], v[1], v[2]);
	d_1 = vec3_dot(n, v[3]);
	d_2 = vec3_dot(n, v[0]);
	if ( (d_1 < 0.0f && d_2 < 0.0f) || (d_1 > 0.0f && d_2 > 0.0f) ) { return 0; }

	return 1;
}

f32 triangle_origin_closest_point(vec3 lambda, const vec3 A, const vec3 B, const vec3 C)
{
	//vec3 AB, AC;
	//vec3_sub(AB, B, A);
	//vec3_sub(AC, C, A);

	//const f32 dot_AB_A = vec3_dot(AB, A);
	//const f32 dot_AB_B = vec3_dot(AB, B);
	//const f32 dot_AB_C = vec3_dot(AB, C);
	//const f32 dot_AC_A = vec3_dot(AC, A);
	//const f32 dot_AC_B = vec3_dot(AC, B);
	//const f32 dot_AC_C = vec3_dot(AC, C);

	//const f32 delta_12 =   dot_AB_B * dot_AC_C - dot_AB_C * dot_AC_B;
	//const f32 delta_02 =  -(dot_AB_C * dot_AC_A - dot_AB_A * dot_AC_C);
	//const f32 delta_01 =   dot_AB_A * dot_AC_B - dot_AB_B * dot_AC_A;
	//const f32 delta = delta_12 + delta_02 + delta_01;

	//vec3_set(lambda, delta_12 / delta, delta_02 / delta, delta_01 / delta);
	
	vec3 a;

	vec3_sub(a, B, A);
	const f32 delta_01_0 = vec3_dot(a, B);
	vec3_sub(a, A, B);
	const f32 delta_01_1 = vec3_dot(a, A);
	vec3_sub(a, A, C);
	const f32 delta_012_2 = delta_01_0 * vec3_dot(a, A) + delta_01_1 * vec3_dot(a, B);

	vec3_sub(a, C, A);
	const f32 delta_02_0 = vec3_dot(a, C);
	vec3_sub(a, A, C);
	const f32 delta_02_2 = vec3_dot(a, A);
	vec3_sub(a, A, B);
	const f32 delta_012_1 = delta_02_0 * vec3_dot(a, A) + delta_02_2 * vec3_dot(a, C);
		
	vec3_sub(a, C, B);
	const f32 delta_12_1 = vec3_dot(a, C);
	vec3_sub(a, B, C);
	const f32 delta_12_2 = vec3_dot(a, B);
	vec3_sub(a, B, A);
	const f32 delta_012_0 = delta_12_1 * vec3_dot(a, B) + delta_12_2 * vec3_dot(a, C);
		
	const f32 delta = delta_012_0 + delta_012_1 + delta_012_2;
	lambda[0] = delta_012_0 / delta;
	lambda[1] = delta_012_1 / delta;
	lambda[2] = delta_012_2 / delta;

	return delta;
}

u32 triangle_origin_closest_point_is_internal(vec3 lambda, const vec3 A, const vec3 B, const vec3 C)
{
	triangle_origin_closest_point(lambda, A, B, C);
	return (lambda[0] < 0.0f || lambda[1] < 0.0f || lambda[2] < 0.0f) ? 0 : 1;
}
