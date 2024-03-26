#include <float.h>

#include "rigid_body.h"

void rigid_body_update_local_box(struct rigid_body *body)
{
	vec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
	vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	for (u32 i = 0; i < body->mesh.v_count; ++i)
	{
		min[0] = fminf(min[0], body->mesh.v[i][0]); 
		min[1] = fminf(min[1], body->mesh.v[i][1]);			
		min[2] = fminf(min[2], body->mesh.v[i][2]);			

		max[0] = fmaxf(max[0], body->mesh.v[i][0]);			
		max[1] = fmaxf(max[1], body->mesh.v[i][1]);			
		max[2] = fmaxf(max[2], body->mesh.v[i][2]);			
	}

	vec3_sub(body->local_box.hw, max, min);
	vec3_mul_constant(body->local_box.hw, 0.5f);
	vec3_add(body->local_box.center, min, body->local_box.hw);
}

void rigid_body_proxy(struct AABB *proxy, struct rigid_body *body)
{
	rigid_body_update_local_box(body);
	vec3_add(proxy->center, body->local_box.center, body->position);
	vec3_set(proxy->hw, body->local_box.hw[0] + body->margin,
			body->local_box.hw[1] + body->margin,
			body->local_box.hw[2] + body->margin);
}

#define VOL	0 
#define T_X 	1
#define T_Y 	2
#define T_Z 	3
#define T_XX	4
#define T_YY	5
#define T_ZZ	6
#define T_XY	7
#define T_YZ	8
#define T_ZX	9
	    
//TODO: REPLACE using table
static u32 comb(const u32 o, const u32 u)
{
	assert(u <= o);

	u32 v1 = 1;
	u32 v2 = 1;
	u32 rep = (u <= o-u) ? u : o-u;

	for (u32 i = 0; i < rep; ++i)
	{
		v1 *= (o-i);
		v2 *= (i+1);
	}

	assert(v1 % v2 == 0);

	return v1 / v2;
}

void statics_print(FILE *file, struct rigid_body *body)
{
	mat3_print("intertia tensor", body->inertia_tensor);
	fprintf(stderr, "mass: %f\n", body->mass);
}

static f32 statics_internal_line_integrals(const vec2 v0, const vec2 v1, const vec2 v2, const u32 p, const u32 q, const vec3 int_scalars)
{
	assert(p <= 4 && q <= 4);
	
	f32 sum = 0.0f;
	for (u32 i = 0; i <= p; ++i)
	{
		for (u32 j = 0; j <= q; ++j)
		{
			sum += int_scalars[0] * comb(p, i) * comb(q, j) * powf(v1[0],i) * powf(v0[0],p-i) * powf(v1[1],j) * powf(v0[1],q-j) / comb(p+q, i+j);
			sum += int_scalars[1] * comb(p, i) * comb(q, j) * powf(v2[0],i) * powf(v1[0],p-i) * powf(v2[1],j) * powf(v1[1],q-j) / comb(p+q, i+j);
			sum += int_scalars[2] * comb(p, i) * comb(q, j) * powf(v0[0],i) * powf(v2[0],p-i) * powf(v0[1],j) * powf(v2[1],q-j) / comb(p+q, i+j);
		}
	}

	return sum / (p+q+1);
}
	
/*
 *  alpha beta gamma CCW
 */ 
static void statics_internal_calculate_integrals(f32 integrals[10], const vec3 p0, const vec3 p1, const vec3 p2)
{
	vec3 n, a, b;
	vec2 v0, v1, v2;
	vec3_sub(a, p1, p0);
	vec3_sub(b, p2, p0);
	vec3_cross(n, a, b);
	vec3_mul_constant(n, 1.0f / vec3_length(n));
	const f32 d = -vec3_dot(n, p0);

	u32 max_index = 0;
	if (n[max_index]*n[max_index] < n[1]*n[1]) { max_index = 1; }
	if (n[max_index]*n[max_index] < n[2]*n[2]) { max_index = 2; }

	/* maxized normal direction determines projected surface integral axes (we maximse the projected surface area) */

	
	const u32 a_i = (1+max_index) % 3;
	const u32 b_i = (2+max_index) % 3;
	const u32 y_i = max_index % 3;

	//vec3_set(n, n[a_i], n[b_i], n[y_i]);

	/* TODO: REPLACE */
	union { f32 f; u32 bits; } val = { .f = n[y_i] };
	const f32 n_sign = (val.bits >> 31) ? -1.0f : 1.0f;

	vec2_set(v0, p0[a_i], p0[b_i]);
	vec2_set(v1, p1[a_i], p1[b_i]);
	vec2_set(v2, p2[a_i], p2[b_i]);
	
	const vec3 delta_a =
	{
		v1[0] - v0[0],
		v2[0] - v1[0],
		v0[0] - v2[0],
	};
	
	const vec3 delta_b = 
	{
		v1[1] - v0[1],
		v2[1] - v1[1],
		v0[1] - v2[1],
	};

	/* simplify cross product of v1-v0, v2-v0 to get this */
	const f32 P_1 	=  n_sign * ((v0[0] + v1[0])*delta_b[0] + (v1[0] + v2[0])*delta_b[1] + (v0[0] + v2[0])*delta_b[2]) / 2.0f;
	const f32 P_a 	=  (n_sign / 2.0f) * statics_internal_line_integrals(v0, v1, v2, 2, 0, delta_b);
	const f32 P_aa 	=  (n_sign / 3.0f) * statics_internal_line_integrals(v0, v1, v2, 3, 0, delta_b);
	const f32 P_aaa =  (n_sign / 4.0f) * statics_internal_line_integrals(v0, v1, v2, 4, 0, delta_b);
	const f32 P_b 	= -(n_sign / 2.0f) * statics_internal_line_integrals(v0, v1, v2, 0, 2, delta_a);
	const f32 P_bb 	= -(n_sign / 3.0f) * statics_internal_line_integrals(v0, v1, v2, 0, 3, delta_a);
	const f32 P_bbb = -(n_sign / 4.0f) * statics_internal_line_integrals(v0, v1, v2, 0, 4, delta_a);
	const f32 P_ab 	=  (n_sign / 2.0f) * statics_internal_line_integrals(v0, v1, v2, 2, 1, delta_b);
	const f32 P_aab =  (n_sign / 3.0f) * statics_internal_line_integrals(v0, v1, v2, 3, 1, delta_b);
	const f32 P_abb =  (n_sign / 3.0f) * statics_internal_line_integrals(v0, v1, v2, 1, 3, delta_b);

	const f32 a_y_div = n_sign / n[y_i];
	const f32 n_y_div = 1.0f / n[y_i];

	/* surface integrals */
	const f32 S_a 	= a_y_div * P_a;
	const f32 S_aa 	= a_y_div * P_aa;
	const f32 S_aaa = a_y_div * P_aaa;
	const f32 S_aab = a_y_div * P_aab;
	const f32 S_b 	= a_y_div * P_b;
	const f32 S_bb 	= a_y_div * P_bb;
	const f32 S_bbb = a_y_div * P_bbb;
	const f32 S_bby = -a_y_div * n_y_div * (n[a_i]*P_abb + n[b_i]*P_bbb + d*P_bb);
	const f32 S_y 	= -a_y_div * n_y_div * (n[a_i]*P_a + n[b_i]*P_b + d*P_1);
	const f32 S_yy 	= a_y_div * n_y_div * n_y_div * (n[a_i]*n[a_i]*P_aa + 2.0f*n[a_i]*n[b_i]*P_ab + n[b_i]*n[b_i]*P_bb 
			+ 2.0f*d*n[a_i]*P_a + 2.0f*d*n[b_i]*P_b + d*d*P_1);	
	const f32 S_yyy = -a_y_div * n_y_div * n_y_div * n_y_div * (n[a_i]*n[a_i]*n[a_i]*P_aaa + 3.0f*n[a_i]*n[a_i]*n[b_i]*P_aab
			+ 3.0f*n[a_i]*n[b_i]*n[b_i]*P_abb + n[b_i]*n[b_i]*n[b_i]*P_bbb + 3.0f*d*n[a_i]*n[a_i]*P_aa 
			+ 6.0f*d*n[a_i]*n[b_i]*P_ab + 3.0f*d*n[b_i]*n[b_i]*P_bb + 3.0f*d*d*n[a_i]*P_a
		       	+ 3.0f*d*d*n[b_i]*P_b + d*d*d*P_1);
	const f32 S_yya = a_y_div * n_y_div * n_y_div * (n[a_i]*n[a_i]*P_aaa + 2.0f*n[a_i]*n[b_i]*P_aab + n[b_i]*n[b_i]*P_abb 
			+ 2.0f*d*n[a_i]*P_aa + 2.0f*d*n[b_i]*P_ab + d*d*P_a);	

	if (max_index == 2)
	{
		integrals[VOL] += S_a * n[0];
	}
	else if (max_index == 1)
	{
		integrals[VOL] += S_b * n[0];
	}
	else
	{
		integrals[VOL] += S_y * n[0];
	}

	integrals[T_X + a_i] += S_aa * n[a_i] / 2.0f;
	integrals[T_X + b_i] += S_bb * n[b_i] / 2.0f;
	integrals[T_X + y_i] += S_yy * n[y_i] / 2.0f;

	integrals[T_XX + a_i] += S_aaa * n[a_i] / 3.0f;
	integrals[T_XX + b_i] += S_bbb * n[b_i] / 3.0f;
	integrals[T_XX + y_i] += S_yyy * n[y_i] / 3.0f;

	integrals[T_XY + a_i] += S_aab * n[a_i] / 2.0f;
	integrals[T_XY + b_i] += S_bby * n[b_i] / 2.0f;
	integrals[T_XY + y_i] += S_yya * n[y_i] / 2.0f;
}

/*
 * Mirtich's Algorithm (Dynamic Solutions to Multibody Systems, Appendix D)
 */
void statics_setup(struct rigid_body *body, struct arena *stack, struct tri_mesh *mesh, const f32 density)
{
	struct arena record = *stack;
	
	body->mesh = *mesh;
	const vec3ptr v = mesh->v;
	const vec3u32ptr tri = mesh->tri;
	f32 integrals[10] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; 

	for (u32 i = 0; i < mesh->tri_count; ++i)
	{
		statics_internal_calculate_integrals(integrals, v[tri[i][0]], v[tri[i][1]], v[tri[i][2]]); 
	}

	//fprintf(stderr, "Volume integrals: %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n\n",
	//		 integrals[VOL ],
	//		 integrals[T_X ],
	//		 integrals[T_Y ],
	//		 integrals[T_Z ],
	//		 integrals[T_XX],
	//		 integrals[T_YY],
	//		 integrals[T_ZZ],
        //                 integrals[T_XY],
        //                 integrals[T_YZ],
        //                 integrals[T_ZX]);

	body->mass = integrals[VOL] * density;
	assert(body->mass >= 0.0f);

	/* center of mass */
	vec3 com = 
	{ 
		integrals[T_X] * density / body->mass,
	       	integrals[T_Y] * density / body->mass,
	       	integrals[T_Z] * density / body->mass,
	};

	fprintf(stderr, "Center of Mass: { %f, %f, %f }\n", com[0], com[1], com[2]);

	const f32 I_xx = density * (integrals[T_YY] + integrals[T_ZZ]) - body->mass * (com[1]*com[1] + com[2]*com[2]);
	const f32 I_yy = density * (integrals[T_XX] + integrals[T_ZZ]) - body->mass * (com[0]*com[0] + com[2]*com[2]);
	const f32 I_zz = density * (integrals[T_XX] + integrals[T_YY]) - body->mass * (com[0]*com[0] + com[1]*com[1]);
	const f32 I_xy = density * integrals[T_XY] - body->mass * com[0] * com[1];
	const f32 I_xz = density * integrals[T_ZX] - body->mass * com[0] * com[2];
	const f32 I_yz = density * integrals[T_YZ] - body->mass * com[1] * com[2];

	mat3_set(body->inertia_tensor, I_xx, -I_xy, -I_xz,
		       		 	 -I_xy,  I_yy, -I_yz,
					 -I_xz, -I_yz, I_zz);

	/* set local frame coordinates */
	vec3_copy(body->position, com);
	vec3_negative(com);
	for (u32 i = 0; i < mesh->v_count; ++i)
	{
		vec3_translate(v[i], com);
	}
	
	*stack = record;
}
