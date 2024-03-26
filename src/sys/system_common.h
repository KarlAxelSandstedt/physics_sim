#ifndef __SYSTEM_DEFINITION_H__
#define __SYSTEM_DEFINITION_H__

#include "mg_common.h"
#include "bit_vector.h"


/* MACROS, GLOBALS and SYSTEM-LEVEL STRUCTS */
#if __OS__ == __LINUX__
#	define INLINE		inline
#	define ALIGN(m)	__attribute__((aligned (m)))	
#	if   _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 
#		define ALIGN_ALLOC(ptr_addr, size, alignment)	posix_memalign((void **)(ptr_addr),(alignment),(size))
#	endif
#endif

#ifdef __GNUC__
#define __MG_COMPILER__ __GCC__
#endif

#if __MG_COMPILER__ == __GCC__
#	ifdef	__SSE__
#		define	__SSE_EXT__
#	endif
#	ifdef	__SSE2__
#		define	__SSE2_EXT__
#	endif
#	ifdef	__SSE3__
#		define	__SSE3_EXT__
#	endif
#	ifdef	__SSE4_2__
#		define	__SSE4_EXT__
#	endif
#endif

#if __OS__ == __WIN64__

#define INLINE		__forceinline
#define ALIGN(m)	__declspec(align(m))
#define ALIGN_ALLOC(ptr_addr, size, alignment) (*(ptr_addr) = _aligned_malloc((size),(alignment)))

/* Only targeting 64-bit, SSE, SSE2 in instruction set per def? Need in compilation step to find a way to check SSE extensions properly */
#define __SSE_EXT__
#define __SSE2_EXT__

#endif

/**
 * Program specific keycodes. node that given a previous entry's value, if the next one has not a specific value, 
 * it will be set to the previous value + 1. We should not override any keycodes for lowercase letters, nor numerics.
 * Keep this under 255 and we can keep everythin in a char!
 */
enum mg_keycode {
	MG_SHIFT,
	MG_SPACE,
	MG_ESCAPE,
	MG_F1,
	MG_F2,
	MG_F3,
	MG_F4,
	MG_F5,
	MG_F6,
	MG_F7,
	MG_F8,
	MG_F9,
	MG_F10,
	MG_F11,
	MG_F12,
	MG_0 = 48,
	MG_1,
	MG_2,
	MG_3,
	MG_4,
	MG_5,
	MG_6,
	MG_7,
	MG_8,
	MG_9,
	MG_A = 65,
	MG_B,  
	MG_C, 
	MG_D, 
	MG_E, 
	MG_F, 
	MG_G, 
	MG_H, 
	MG_I, 
	MG_J, 
	MG_K, 
	MG_L, 
	MG_M, 
	MG_N, 
	MG_O, 
	MG_P, 
	MG_Q, 
	MG_R, 
	MG_S, 
	MG_T, 
	MG_U, 
	MG_V, 
	MG_W, 
	MG_X, 
	MG_Y, 
	MG_Z, 
	MG_a = 97,
	MG_b,  
	MG_c, 
	MG_d, 
	MG_e, 
	MG_f, 
	MG_g, 
	MG_h, 
	MG_i, 
	MG_j, 
	MG_k, 
	MG_l, 
	MG_m, 
	MG_n, 
	MG_o, 
	MG_p, 
	MG_q, 
	MG_r, 
	MG_s, 
	MG_t, 
	MG_u, 
	MG_v, 
	MG_w, 
	MG_x, 
	MG_y, 
	MG_z,
	MG_NO_SYMBOL,
};

extern struct system_event_queue *g_sys_queue;
#define MAX_SYSTEM_EVENTS 256

enum mg_button
{
	MG_BUTTON_LEFT = 0,
	MG_BUTTON_RIGHT,
	MG_BUTTON_SCROLL,
	MG_BUTTON_NONMAPPED,
	MG_BUTTON_COUNT,
};

enum system_event_type 
{
	SYSTEM_KEY_PRESSED,
	SYSTEM_KEY_RELEASED,
	SYSTEM_BUTTON_PRESSED,
	SYSTEM_BUTTON_RELEASED,
	SYSTEM_CURSOR_POSITION,
	SYSTEM_COUNTER_MOTION,
	SYSTEM_WINDOW_CLOSE,
	SYSTEM_WINDOW_CURSOR_ENTER,
	SYSTEM_WINDOW_CURSOR_LEAVE,
	SYSTEM_WINDOW_FOCUS_IN,
	SYSTEM_WINDOW_FOCUS_OUT,
	SYSTEM_WINDOW_EXPOSE,
	SYSTEM_WINDOW_CONFIG,
	SYSTEM_WINDOW_MINIMIZE,
	SYSTEM_NO_EVENT,
};

union event_value 
{
	i32 key; /* Input key */
	i32 button; /* Input Mouse button */
	vec2i32 cursor_position; /* Input Mouse position */
	vec2i32 cursor_motion; /* Input Mouse position */
};

struct system_event {
	enum system_event_type type;
	union event_value value;	
};

struct system_event_queue {
	struct system_event events[MAX_SYSTEM_EVENTS];
	u32 num_events;
};

struct mg_input_state {
	/* keyboard state */
	u32 shift_pressed : 1;
	u32 caps_pressed : 1;
	u32 control_pressed : 1;
	u32 alt_pressed : 1;	
	struct bit_vec key_pressed;

	/* mouse state */
	u32 button_pressed[3];

	/* if cursor should be warped */
	u32 cursor_warp : 1;

	vec2i32 cursor_position; /* bottom left = (0,0) */
};

extern struct mg_input_state *g_input_state;

struct gl_config
{
	char *gl_vendor;
	char *gl_renderer;
	char *gl_version;
	char *gl_shading_language;
#ifdef MGL_DEBUG
	/*
	 * Here we setup a full mgl call history, buffer read/write history, so we can dynamically warn
	 * against wonk.
	 */
	//TODO: gl_history; 
#endif
};

#endif
