#ifndef __R_COMMON_H__
#define __R_COMMON_H__

#include "mg_common.h"
#include "mg_mempool.h"

enum drawbuffer_type 
{
	DRAWBUFFER_COLOR,			/* vec3 pos | vec4 color */
	DRAWBUFFER_COLOR_LIGHTNING,		/* vec3 pos | vec4 color | vec3 normal */
	DRAWBUFFER_COLOR_LIGHTNING_INDEX,	/* vec3 pos | vec4 color | vec3 normal | i32 index */
	DRAWBUFFER_WIDGET,			/* widgets / text drawing */
};

struct drawbuffer 
{
	struct arena v_buf;
	struct arena i_buf;
	enum drawbuffer_type type;
	u64 stride;
	i32 next_index;
	u32 prg;
	u32 vao;
	u32 vbo;
	u32 ebo;
	u32 index;
};

/******************** r_command.c ********************/

enum r_cmd_type
{
	R_CMD_VIEWPORT,
	R_CMD_MOTION,
	R_CMD_LEFT,
	R_CMD_RIGHT,
	R_CMD_FORWARD,
	R_CMD_BACKWARD,
	R_CMD_COUNT,
};

union r_cmd_value
{
	vec2i32 motion;
	i32 pressed;
};

struct r_cmd
{
	enum r_cmd_type type;
	union r_cmd_value value;
};

struct r_cmd_queue
{
	struct r_cmd *cmds;
	u32 cmd_count;
	u32 max_cmd_count;
};

extern struct r_cmd_queue *g_r_cmd_queue;

void r_cmd_queue_init(struct arena *mem_persistent, const u32 max_command_count);
void r_cmd_queue_clear(void);
void r_cmd_add(struct r_cmd *cmd);

#endif
