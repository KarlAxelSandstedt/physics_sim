#include <string.h>

#include "r_public.h"
#include "r_local.h"

#ifdef __linux__
#define v_lightning 	"../lightning.vert"
#define f_lightning	"../lightning.frag"
#define v_color 	"../color.vert"
#define f_color		"../color.frag"
#define	v_widget	"../widget.vert"
#define	f_widget	"../widget.frag"
#elif _WIN64
#define v_lightning 	"../lightning.vert"
#define f_lightning	"../lightning.frag"
#define v_color 	"../color.vert"
#define f_color		"../color.frag"
#define	v_widget	"../widget.vert"
#define	f_widget	"../widget.frag"
#endif

static void shader_source_and_compile(GLuint shader, const char *filepath)
{
	FILE *file = fopen(filepath, "rb");

	char buf[4096];
	memset(buf, 0, sizeof(buf));
	fseek(file, 0, SEEK_END);
	long int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	fread(buf, 1, size, file);

	const GLchar *buf_ptr = buf;
	mglShaderSource(shader, 1, &buf_ptr, 0);

	fclose(file);

	mglCompileShader(shader);	

	GLint compiled;
	mglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLsizei len = 0;
		mglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		mglGetShaderInfoLog(shader, len, &len, buf);
		fprintf(stderr, "Error %s:%u - %s\n", __FILE__, __LINE__, buf);
	}
}

void r_compile_shader(u32 *prg, const char *v_filepath, const char *f_filepath)
{
	GLuint v_sh = mglCreateShader(GL_VERTEX_SHADER);
	GLuint f_sh = mglCreateShader(GL_FRAGMENT_SHADER);

	shader_source_and_compile(v_sh, v_filepath);
	shader_source_and_compile(f_sh, f_filepath);

	*prg = mglCreateProgram();

	mglAttachShader(*prg, v_sh);
	mglAttachShader(*prg, f_sh);

	mglLinkProgram(*prg);

	mglDetachShader(*prg, v_sh);
	mglDetachShader(*prg, f_sh);

	mglDeleteShader(v_sh);
	mglDeleteShader(f_sh);
}

struct render_state *r_init(struct arena *mem, struct graphics_context *gtx)
{
	r_cmd_queue_init(mem, 256);
	struct render_state state = { 0 };
	state.gl.vao_bound = -1;
	state.gl.prg_bound = -1;

	mglEnable(GL_CULL_FACE);
	mglFrontFace(GL_CCW);
	mglCullFace(GL_BACK);
	
	mglEnable(GL_DEPTH_TEST);

	mglEnable(GL_BLEND);
	mglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	struct render_state *re = arena_push(mem, &state, sizeof(state));
	r_compile_shader(&re->lightning_prg, v_lightning, f_lightning);
	r_compile_shader(&re->widget_prg, v_widget, f_widget);
	r_compile_shader(&re->color_prg, v_color, f_color);

	const vec3 position = {3.0f, 1.0f, -3.0f};
	const vec3 left = {1.0f, 0.0f, 0.0f};
	const vec3 up = {0.0f, 1.0f, 0.0f};
	const vec3 dir = {0.0f, 0.0f, 1.0f};
	camera_construct(&re->cam, 
			position, 
			left,
			up,
			dir,
			0.0f,
			0.0f,
			0.0250f,
			1024.0f,
			(f32) gtx->win.size[0] / gtx->win.size[1],
			2.0f * MM_PI_F / 3.0f );

	/**
	 * Super simple rendering for simulation:
	 * 	entity:
	 * 		- vec3 pos
	 * 		- vec4 rot
	 * 		- vertices[]
	 *		- indices[]
	 *		- visuals { ... }
	 *	draw_entities(*objects, count);
	 *
	 * How do we set the correct buffers?
	 * - gl_config_set_entity_drawing();
	 * 	- gl.entity_vao
	 * 	- gl.entity_vbo
	 * 	- gl.entity_ebo
	 * 	- draw_entities(*objects, count);
	 * 	if (buffer_full)
	 * 		emit_warning();
	 */
	drawbuffer_new(&re->gl, &re->entity_buf, 128*1024*1024, 64*1024*1024, DRAWBUFFER_COLOR_LIGHTNING_INDEX, re->lightning_prg);
	//drawbuffer_new(&re->gl, &re->widget_buf, 1024*1024, 1024*1024, DRAWBUFFER_WIDGET, re->widget_prg);
	drawbuffer_new(&re->gl, &re->color_buf, 1024*1024, 1024*1024, DRAWBUFFER_COLOR, re->color_prg);

	return re;
}
