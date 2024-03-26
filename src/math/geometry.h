#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "mmath.h"
#include "r_common.h"
#include "mgl_primitives.h"

/**************************************************************/

#define SIMPLEX_0	0
#define SIMPLEX_1	1
#define SIMPLEX_2	2
#define SIMPLEX_3	3

/**************************************************************/

struct tri_mesh 
{
	vec3ptr v;		/* vertices */
	vec3u32ptr tri; 	/* CCW triangles */
	u32 v_count;	
	u32 tri_count;
};

struct DCEL_half_edge {
	i32 he;		/* half edge */
	i32 origin;	/* vertex index origin */
	i32 twin; 	/* twin half edge */
	i32 face_ccw; 	/* face to the left of half edge */
	i32 next;	/* next half edge in ccw traversal of face_ccw */
	i32 prev;	/* prev half edge in ccw traversal of face_ccw */
};

/**
 * - If ccw face is in free chain, he_index == next free face index and relation_unit == -1 
 * - If face_index == related_to, the face has no current relations
 */
struct DCEL_face {
	i32 he_index; 
	i32 relation_unit;
};

/**
 * (Computational Geometry Algorithms and Applications, Section 2.2) 
 * DCEL - doubly-connected edge list. Can represent convex 3d bodies (with no holes in polygons)
 * 	  and 2d planar graphs.
 */
struct DCEL {
	struct DCEL_face *faces; 
	struct DCEL_half_edge *he_table; /* indexed table | free chain (next == next free) containing half edge information */
	i32 next_he; /* next free slot in edges, -1 == no memory left in free chain */
	i32 next_face; /* next free slot in edges, -1 == no memory left in free chain */
	i32 num_faces;
	i32 num_he;
};

#define DCEL_ALLOC_EDGES(dcel_ptr, table_arena_ptr, n) 							\
{													\
	struct DCEL_half_edge *new_table = (struct DCEL_half_edge *)arena_push_packed(table_arena_ptr, NULL, n*sizeof(struct DCEL_half_edge)); 	\
	new_table[n-1].next = (dcel_ptr)->next_he;							\
	(dcel_ptr)->next_he = (dcel_ptr)->num_he;							\
	for (i32 k = 0; k < n-1; ++k)									\
	{												\
		new_table[k].next = (dcel_ptr)->num_he + 1 + k;						\
	}												\
	assert(&((dcel_ptr)->he_table[(dcel_ptr)->next_he]) == new_table);				\
}													\
	(dcel_ptr)->num_he += n								

#define DCEL_ALLOC_FACES(dcel_ptr, faces_arena_ptr, n) 						\
{												\
	struct DCEL_face *new_faces = (struct DCEL_face *)arena_push_packed(faces_arena_ptr, NULL, n * sizeof(struct DCEL_face));	\
	new_faces[n-1].he_index = (dcel_ptr)->next_face;					\
	new_faces[n-1].relation_unit = -1;							\
	(dcel_ptr)->next_face = (dcel_ptr)->num_faces;						\
	for (i32 k = 0; k < n-1; ++k)								\
	{											\
		new_faces[k].he_index = (dcel_ptr)->num_faces + 1 + k;				\
		new_faces[k].relation_unit = -1;						\
	}											\
	assert(&((dcel_ptr)->faces[(dcel_ptr)->next_face]) == new_faces);			\
}												\
	(dcel_ptr)->num_faces += n;									

i32 DCEL_half_edge_add(struct DCEL *dcel, struct arena *table_mem, const i32 origin, const i32 twin, const i32 face_ccw, const i32 next, const i32 prev);
i32 DCEL_half_edge_reserve(struct DCEL *dcel, struct arena *table_mem);
void DCEL_half_edge_set(struct DCEL *dcel, const i32 he, const i32 origin, const i32 twin, const i32 face_ccw, const i32 next, const i32 prev);
void DCEL_half_edge_remove(struct DCEL *dcel, const i32 he);
/* returns face index */
i32 DCEL_face_add(struct DCEL *dcel, struct arena *face_mem, const i32 edge, const i32 unit);
void DCEL_face_remove(struct DCEL *dcel, const i32 face);

/* Clarkson-Shor randomized convex hull (Computational Geometry Algorithms and Applications, Section 11) 
 * TODO: A space usage improvement found in the he original paper [Applications of Random Sampling in Computational Geometry, II, Page 23 (A linear-space variant] reduces the memory usage from O(nlogn) to O(n).
 * */
i32 convex_hull_cs(struct arena *table_mem, struct arena *face_mem, struct arena *conflict_mem, struct arena *mem_4, const f32 *vs, const i32 num_vs, const f32 EPSILON);

#ifdef MGL_DEBUG
i32 convex_hull_cs_step_draw(struct arena *table_mem, struct arena *face_mem, struct arena *conflict_mem, struct arena *mem_4, struct arena *mem_5, const f32 *vs, const i32 num_vs, const f32 EPSILON, const i32 num_steps, const u32 seed, struct drawbuffer *d_buf, const vec4 color, const i32 polygon_mode);
#endif

struct tri_mesh convex_hull_empty(void);
struct tri_mesh convex_hull_construct(struct arena *mem, struct arena *table_mem, struct arena *face_mem, struct arena *conflict_mem, struct arena *mem_4, struct arena *mem_5, const vec3ptr v, const u32 v_count, const f32 EPSILON);

/****************************************************************************/

/**
 * Gilbert-Johnson-Keerthi intersection algorithm in 3D. Based on the original paper. 
 *
 * For understanding, see [ Collision Detection in Interactive 3D environments, chapter 4.3.1 - 4.3.8 ]
 */
struct gjk_simplex
{
	vec3 p[4];
	u64 id[4];
	f32 dot[4];
	u32 type;
};

#define EPA_MAX_ITERATIONS 256

struct contact_manifold
{
	vec3 p_1;
	vec3 p_2;
	f32 penetration_depth;
};

u32 GJK_test(const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2, const f32 abs_tol, const f32 tol); /* [Page 146] -1 on error (To few points, or no initial tetrahedron). 0 == no collision, 1 == collision. */
f32 GJK_distance(vec3 c_1, vec3 c_2, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2, const f32 rel_tol, const f32 abs_tol); /* Retrieve shortest distance between objects and the convex objects' closest points, or 0.0f if collision. */
u32 GJK_EPA(struct arena *mem, struct contact_manifold *c_m, const vec3 pos_1, vec3ptr vs_1, const u32 n_1, const vec3 pos_2, vec3ptr vs_2, const u32 n_2, const f32 rel_tol, const f32 abs_tol); /* Returns 0 if no collision and contact manifold penetration depth 0.0f, otherwise != 0 and a valid contact manifold */

u32 GJKC_test(const f32 *vs_1, const u32 n_1, const f32 *vs_2, const u32 n_2, const f32 tol);
u32 GJKC_world_test(const vec3 pos_1, const f32 *vs_1, const u32 n_1, const vec3 pos_2, const f32 *vs_2, const u32 n_2, const f32 tol);

void convex_centroid(vec3 centroid, vec3ptr vs, const u32 n);
u32 convex_support(vec3 support, const vec3 dir, vec3ptr vs, const u32 n);
/* support of A-B, A,B convex */
u64 convex_minkowski_difference_support(vec3 support, const vec3 dir, vec3ptr A, const u32 n_A, vec3ptr B, const u32 n_B);

/* Get indices for initial tetrahedron using given tolerance. return 0 on no initial tetrahedron, 1 otherwise. */
i32 tetrahedron_indices(i32 indices[4], const vec3ptr v, const i32 v_count, const f32 tol);

/* Reorder (If necessary) triangle t such that it is CCW ([0] -> [1] -> [2] -> [0]) from p's point of view (switch [0] and [1]) */
void triangle_CCW_relative_to(vec3 BCA[3], const vec3 p);
void triangle_CCW_relative_to_origin(vec3 BCA[3]);

/**************************************************************/

/**
 * plane: geomtrical primitive
 * plane normal - normal of plane
 * signed_distance - Signed distance to plane; plane_normal * signed_distance is point on plane.
 */
struct plane {
	vec3 normal;
	f32 signed_distance;
};

struct sphere {
	vec3 center;
	f32 radius;
};

struct AABB {
	vec3 center;
	vec3 hw;
};

struct OBB {
	vec3 center;
	vec3 hw;
	vec3 x_axis;
	vec3 z_axis;
};

struct cylinder {
	vec3 center;
	f32 radius;
	f32 half_height;
};

struct tmp {
	i32 tmp;
};

void *callback_spawn_mgl_line(void *data);
void *callback_spawn_mgl_quad(void *data);

void spawn_mgl_line(struct arena *arena, const vec3 origin, const vec3 direction, const f32 distance, const vec4 color);
void spawn_mgl_quad(struct drawbuffer *d_buf, const struct mgl_quad *quad);
void spawn_mgl_sphere(struct drawbuffer *d_buf, const struct sphere *sph, const i32 refinement, const vec4 color);
void spawn_mgl_cube_AABB(struct drawbuffer *d_buf, const struct AABB *aabb, const vec4 color);
void spawn_mgl_cube_OBB(struct drawbuffer *d_buf, const struct OBB *obb, const vec4 color);
void spawn_mgl_cylinder(struct drawbuffer *d_buf, const struct cylinder *cyl, const i32 refinement, const vec4 color);
void spawn_cylinder_intersection(struct drawbuffer *d_buf, const struct cylinder *cyl_1, const struct cylinder *cyl_2, const i32 refinement, const vec4 color, struct arena *mem_1, struct arena *mem_2, struct arena *mem_3, struct arena *mem_4, struct arena *mem_5);

/* Bounding volume calculation */
void AABB_bounding_volume(struct AABB *dst, const vec3ptr v, const u32 v_count, const f32 tol);
void AABB_union(struct AABB *box_union, const struct AABB *a, const struct AABB *b);
void AABB_push_lines(struct drawbuffer *buf, const struct AABB *box, const vec4 color);
i32 AABB_contains(const struct AABB *a, const struct AABB *b); /* 0 if b is not contained in a, 1 otherwise */

/* Shortest distance methods from point to given primitive (negative distance == behind plane) */
f32 point_plane_distance(const vec3 point, const struct plane *plane);
f32 point_plane_signed_distance(const vec3 point, const struct plane *plane);
f32 point_sphere_distance(const vec3 point, const struct sphere *sph);
f32 point_AABB_distance(const vec3 point, const struct AABB *aabb);
f32 point_OBB_distance(const vec3 point, const struct OBB *obb);
f32 point_cylinder_distance(const vec3 point, const struct cylinder *cyl);

f32 AABB_distance(const struct AABB *a, const struct AABB *b);
f32 OBB_distance(const struct OBB *a, const struct OBB *b);
f32 sphere_distance(const struct sphere *a, const struct sphere *b);
f32 cylinder_distance(const struct cylinder *a, const struct cylinder *b);

i32 AABB_test(const struct AABB *a, const struct AABB *b);
i32 OBB_test(const struct OBB *a, const struct OBB *b);
i32 sphere_test(const struct sphere *a, const struct sphere *b);
i32 cylinder_test(const struct cylinder *a, const struct cylinder *b);

i32 AABB_intersection(struct AABB *dst, const struct AABB *a, const struct AABB *b);
i32 OBB_intersection(struct tmp *dst, const struct OBB *a, const struct OBB *b);
i32 sphere_intersection(struct tmp *dst, const struct sphere *a, const struct sphere *b);
i32 cylinder_intersection(struct tmp *dst, const struct cylinder *a, const struct cylinder *b);

/* Closest point on primitive to point */
void point_plane_closest_point(vec3 closest_point, const vec3 point, const struct plane *plane);
void point_sphere_closest_point(vec3 closest_point, const vec3 point, const struct sphere *sph);
void point_AABB_closest_point(vec3 closest_point, const vec3 point, const struct AABB *aabb);
void point_OBB_closest_point(vec3 closest_point, const vec3 point, const struct OBB *obb);
void point_cylinder_closest_point(vec3 closest_point, const vec3 point, const struct cylinder *cyl);

/* Intersection tests for ray primitive against given primitives, RETURN 0 == no intersect, 1 == intersect */
i32 ray_plane_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct plane *plane);
i32 ray_sphere_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct sphere * sph);
i32 ray_AABB_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct AABB *abb);
i32 ray_OBB_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct OBB *obb);
i32 ray_cylinder_intersection(vec3 intersection, const vec3 ray_origin, const vec3 ray_direction, const struct cylinder *cyl);

/* We make no assumption of CW or CCW ordering here, so not optimized */
u32 tetrahedron_point_test(const vec3 tetra[4], const vec3 p);

/* CCW!?? lambda is set to barocentric coordinates of the closest point */
f32 triangle_origin_closest_point(vec3 lambda, const vec3 A, const vec3 B, const vec3 C);

/* returns 0 if not internal, 1 if internal, and lambda is set to the barocentric coordinates of A,B,C */
u32 triangle_origin_closest_point_is_internal(vec3 lambda, const vec3 A, const vec3 B, const vec3 C);

#endif
