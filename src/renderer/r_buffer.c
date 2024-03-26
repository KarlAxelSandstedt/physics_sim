#include "r_public.h"
#include "r_local.h"

void drawbuffer_new(struct gl_state *state, struct drawbuffer *buf, const u64 vbo_size, const u64 ebo_size, const enum drawbuffer_type type, const u32 prg)
{
	buf->v_buf = arena_alloc(vbo_size);
	buf->i_buf = arena_alloc(ebo_size);
	buf->prg = prg;
	buf->next_index = 0;
	buf->stride = 0;
	buf->type = type;

	mglGenVertexArrays(1, &buf->vao);
	mglBindVertexArray(buf->vao);

	mglGenBuffers(1, &buf->vbo);
	mglGenBuffers(1, &buf->ebo);
	mglBindBuffer(GL_ARRAY_BUFFER, buf->vbo);
	mglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->ebo);
	mglBufferData(GL_ARRAY_BUFFER, buf->v_buf.mem_size, NULL, GL_DYNAMIC_DRAW);
	mglBufferData(GL_ELEMENT_ARRAY_BUFFER, buf->i_buf.mem_size, NULL, GL_DYNAMIC_DRAW);
	
	state->vao_bound = buf->vao;
	state->vbo_bound = buf->vbo;
	state->ebo_bound = buf->ebo;

	switch (type)
	{
		case DRAWBUFFER_COLOR:
		{
			buf->stride = sizeof(vec3) + sizeof(vec4);
			mglEnableVertexAttribArray(0);
			mglEnableVertexAttribArray(1);

			mglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, buf->stride, (void *)(0));
			mglVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, buf->stride, (void *)(sizeof(vec3)));
			break;
		}
		case DRAWBUFFER_COLOR_LIGHTNING:
		{
			buf->stride = sizeof(vec3) + sizeof(vec4) + sizeof(vec3);
			mglEnableVertexAttribArray(0);
			mglEnableVertexAttribArray(1);
			mglEnableVertexAttribArray(2);

			mglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, buf->stride, (void *)(0));
			mglVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, buf->stride, (void *)(sizeof(vec3)));
			mglVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, buf->stride, (void *)(sizeof(vec3) + sizeof(vec4)));
			break;
		}
		case DRAWBUFFER_COLOR_LIGHTNING_INDEX:
		{
			buf->stride = sizeof(vec3) + sizeof(vec4) + sizeof(vec3) + sizeof(i32);
			buf->index = 0;
			mglEnableVertexAttribArray(0);
			mglEnableVertexAttribArray(1);
			mglEnableVertexAttribArray(2);
			mglEnableVertexAttribArray(3);

			mglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, buf->stride, (void *)(0));
			mglVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, buf->stride, (void *)(sizeof(vec3)));
			mglVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, buf->stride, (void *)(sizeof(vec3) + sizeof(vec4)));
			mglVertexAttribIPointer(3, 1, GL_INT, buf->stride, (void *)(2*sizeof(vec3) + sizeof(vec4)));
			break;
		}
		case DRAWBUFFER_WIDGET:
		{
			assert(0 && "TO BE IMPLEMENTED!\n");
			buf->stride = sizeof(vec3) + sizeof(vec4) + sizeof(vec3) + sizeof(i32);
			buf->index = 0;
			mglEnableVertexAttribArray(0);
			mglEnableVertexAttribArray(1);
			mglEnableVertexAttribArray(2);
			mglEnableVertexAttribArray(3);

			mglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, buf->stride, (void *)(0));
			mglVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, buf->stride, (void *)(sizeof(vec3)));
			mglVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, buf->stride, (void *)(sizeof(vec3) + sizeof(vec4)));
			mglVertexAttribIPointer(3, 1, GL_INT, buf->stride, (void *)(2*sizeof(vec3) + sizeof(vec4)));
			break;
		}	
	}
}

void drawbuffer_clear(struct drawbuffer *buf)
{
	arena_flush(&buf->v_buf);
	arena_flush(&buf->i_buf);
	buf->next_index = 0;
	buf->index = 0;
}

void drawbuffer_free(struct gl_state *state, struct drawbuffer *buf)
{
	mglDeleteVertexArrays(1, &buf->vao);
	mglDeleteBuffers(1, &buf->vbo);
	mglDeleteBuffers(1, &buf->ebo);
	arena_free(&buf->v_buf);
	arena_free(&buf->i_buf);
}

void drawbuffer_bind(struct gl_state *state, const struct drawbuffer *buf)
{
	if (buf->vao != state->vao_bound)
	{
		mglBindVertexArray(buf->vao);
		state->vao_bound = buf->vao;
	}

	if (buf->prg != state->prg_bound)
	{
		mglUseProgram(buf->prg);
		state->prg_bound = buf->prg;
	}

	if (buf->vbo != state->vbo_bound)
	{
		mglBindBuffer(GL_ARRAY_BUFFER, buf->vbo);
		state->vbo_bound = buf->vbo;
	}

	if (buf->ebo != state->ebo_bound)
	{
		mglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->ebo);
		state->ebo_bound = buf->ebo;
	}
}

void drawbuffer_buffer_data(struct gl_state *state, const struct drawbuffer *buf)
{
	const u64 v_size = buf->v_buf.mem_size - buf->v_buf.mem_left;
	const u64 i_size = buf->i_buf.mem_size - buf->i_buf.mem_left;

	drawbuffer_bind(state, buf);
	mglBufferSubData(GL_ARRAY_BUFFER, 0, v_size, buf->v_buf.stack_ptr - v_size);
	mglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, i_size, buf->i_buf.stack_ptr - i_size);
}

void drawbuffer_draw(struct gl_state *state, const struct drawbuffer *buf, const GLenum mode)
{
	drawbuffer_bind(state, buf);
	mglDrawElements(mode, (buf->i_buf.mem_size - buf->i_buf.mem_left) / sizeof(i32), GL_UNSIGNED_INT, (const void *) 0);
}

void drawbuffer_draw_partial(struct gl_state *state, const struct drawbuffer *buf, const GLenum mode, const i32 i_count, const i32 i_offset)
{
	drawbuffer_bind(state, buf);
	mglDrawElements(mode, i_count, GL_UNSIGNED_INT, (const void *) (i64) i_offset);
}
