#ifndef __MGL_PRIMITIVES_H__
#define __MGL_PRIMITIVES_H__

#include "mg_common.h"
#include "mmath.h"

struct mgl_point_colored {
	vec3 p;	/* position */
	vec4 c;	/* color */
};

struct mgl_line {
	vec3 p0; /* point 0 */
	vec4 c0; /* color 0 */
	vec3 p1; /* point 1 */
	vec4 c1; /* color 1 */
};


/**
 *	2 --- 3
 *	|     |
 *	1 --- 0
 * */
struct mgl_quad {
	struct mgl_point_colored points[4];		
};

void mgl_quad_centered_at(struct mgl_quad *quad, const vec3 center, const vec3 normal, const vec2 side_lengths, const vec4 color);

#endif
