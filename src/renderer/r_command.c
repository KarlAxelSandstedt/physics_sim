#include <string.h>

#include "r_common.h"

struct r_cmd_queue *g_r_cmd_queue;

void r_cmd_queue_init(struct arena *mem_persistent, const u32 max_command_count)
{
	struct r_cmd_queue copy =
	{
		.cmd_count = 0,
		.max_cmd_count = max_command_count,
	};

	if (mem_persistent)
	{
		g_r_cmd_queue = arena_push(mem_persistent, &copy, sizeof(struct r_cmd_queue));
		g_r_cmd_queue->cmds = arena_push(mem_persistent, NULL, max_command_count * sizeof(struct r_cmd));
	}
	else
	{
		g_r_cmd_queue = malloc(sizeof(struct r_cmd_queue));
		memcpy(g_r_cmd_queue, &copy, sizeof(struct r_cmd_queue));
		g_r_cmd_queue->cmds = malloc(max_command_count * sizeof(struct r_cmd));
	}
}

void r_cmd_queue_clear(void)
{
	g_r_cmd_queue->cmd_count = 0;
}

void r_cmd_add(struct r_cmd *cmd)
{
	if (g_r_cmd_queue->cmd_count <= g_r_cmd_queue->max_cmd_count)
	{
		g_r_cmd_queue->cmds[g_r_cmd_queue->cmd_count++] = *cmd;
	}
	else
	{
		assert(0 && "Renderer Command Queue full, consider making it larger\n");
	}
}
