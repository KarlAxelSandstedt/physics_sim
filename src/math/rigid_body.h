#ifndef __RIGID_BODY_H__
#define __RIGID_BODY_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "geometry.h"
#include "mmath.h"

struct rigid_body
{
	vec3 velocity;
	struct AABB local_box;	/* bounding AABB */

	i32 proxy;
	f32 margin;
	u32 active : 1;
	u32 dynamic: 1;


	/* static state */
	struct tri_mesh mesh; 		/* hull */
	struct AABB bounding_box;	/* bounding AABB */
	mat3 inertia_tensor;		/* intertia tensor of body frame */
	f32 mass;			/* total body mass */

	/* dynamic state */
	quat rotation;		/* TODO: */
	quat angular_momentum;	/* TODO: */
	vec3 position;	/* center of mass world frame position */
	vec3 linear_momentum;   /* L = mv */
};

void rigid_body_update_local_box(struct rigid_body *body);
void rigid_body_proxy(struct AABB *proxy, struct rigid_body *body);

void statics_print(FILE *file, struct rigid_body *body);
void statics_setup(struct rigid_body *body, struct arena *stack, struct tri_mesh *hull, const f32 density);

#endif
