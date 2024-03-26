#define MG_RUN 1
#if MG_RUN == 1

#include <windows.h>
#include <stdio.h>

#include "mg_common.h"
#include "mg_mempool.h"
#include "system_public.h"
#include "r_public.h"
#include "sim_public.h"
#include "timer.h"
#include "widget.h"

/*
 * (O) screen size
 * () window abstraction functionality
 * () native coordinates => mg_coordinates
 * () Mouse motion 
 */
i32 CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	win_init_function_pointers();

	const u64 memsize = 1024*1024;
	struct arena global_arena = arena_alloc(memsize);

	/* inititate system resources, threads, timers, ... */
	system_resources_init(&global_arena);

	/* init graphics_context */
	char *title = "MossyPhysics";
	const vec2u32 win_position = { 200, 200 };
	const vec2u32 win_size = { 640, 480 };
	const u32 border_size = 0;
	struct graphics_context *gc = graphics_context_init(&global_arena, title, win_position, win_size, border_size);

	gl_config_log(&gc->gl, stdout);
	
	/* init ui context */
	struct ui_state *ui = ui_context_init(&global_arena, 128);

	/* init simulation context */
	struct simulation *sim = sim_init(&global_arena, &convex_volume_intersection_simulation, 733458L);

	struct render_state *re = r_init(&global_arena, gc);

	f64 old_time = time_rdtsc_in_seconds();
	f64 delta;	
	
	while (sim->running)
	{
		delta = time_rdtsc_in_seconds() - old_time;

		/* update stuff, such as unix XWarpPointer shit, and emit events, if necessary */
		gc_update(gc); 

		/* process any system IO events, window events, and check which subsystem they should be under */
	     	sim_process_system_events(sim, ui, gc);
	
	     	/* run simulation step */
	     	sim_main(sim, delta);
	     		
	     	/* simulation_draw -> ui_draw */
	     	r_main(re, gc, sim, ui, delta); 
		
		old_time += delta;
	}
	
	sim_cleanup(sim);
	graphics_context_destroy(gc);
	system_resources_cleanup();
	arena_free(&global_arena);
	

	return 0;
}
#else




#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <GL\gl.h>

#include "win32_thread.h"
#include "mg_timer.h"
#include "mg_logger.h"
#include "mg_string.h"
#include "mg_font.h"
#include "mgl.h"
#include "GL\wglext.h"
#include "widget.h"

int32_t quit = 0;

struct mouse_input {
	POINT position;
	u32 clicked : 1;
	u32 pressed : 1;
	u32 released : 1;
};

struct mouse_input minput;

struct ui_state ui_context;

int32_t screen_width = 1280;
int32_t screen_height = 720;
float aspect_ratio = 1280.0f / 720.0f;

void *drawbuffer;
void *indexbuffer;
uint32_t ui_vbo, ui_ebo, ui_vao;

bool mouse_screen_locked = false;

char *lorem = "There are many variations of passages of Lorem Ipsum available, but the majority have suffered alteration in some form, by injected humour, or randomised words which don't look even slightly believable. If you are going to use a passage of Lorem Ipsum, you need to be sure there isn't anything embarrassing hidden in the middle of text. All the Lorem Ipsum generators on the Internet tend to repeat predefined chunks as necessary, making this the first true generator on the Internet. It uses a dictionary of over 200 Latin words, combined with a handful of model sentence structures, to generate Lorem Ipsum which looks reasonable. The generated Lorem Ipsum is therefore always free from repetition, injected humour, or non-characteristic words etc.";

uint32_t vbo, ebo, vao, prg;
GLsizei tex;

struct vertex {
	float pos[3];
	float uv[2];
};

struct vertex vertices[] = {
	{   1, -1,  -1,  1.0f, 0.0f},
	{   1,  1,  -1,  1.0f, 1.0f},
	{  -1,  1,  -1,  0.0f, 1.0f},
	{  -1, -1,  -1,  0.0f, 0.0f},
};

uint32_t indices[] = {0, 1, 2, 0, 2, 3};

void create_buffers(void)
{
	mglGenBuffers(1, &vbo);
	mglGenBuffers(1, &ebo);	
	mglBindBuffer(GL_ARRAY_BUFFER, vbo);
	mglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	mglBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	mglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
}

void create_vao(void)
{
	mglGenVertexArrays(1, &vao);
	mglBindVertexArray(vao);
	mglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	mglEnableVertexAttribArray(0);
	mglEnableVertexAttribArray(1);
	mglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), 0);
	mglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)(3 * sizeof(float)));
}

#define v_s	"C:/dev/text_rendering/widget.vert"
#define f_s	"C:/dev/text_rendering/widget.frag"

void shader_source_and_compile(GLuint shader, GLenum shader_type)
{
	char *filepath;
	if (shader_type == GL_VERTEX_SHADER)
	{
		filepath = v_s;
	}
	else
	{
		filepath = f_s;
	}

	FILE *file;
	fopen_s(&file, filepath, "rb");

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
		LOG_MESSAGE(TAG_OPENGL, SEVERITY_FATAL, 0, "%s : %s", filepath, buf);
	}
}

void create_shader_program(void)
{
	GLuint v_sh = mglCreateShader(GL_VERTEX_SHADER);
	GLuint f_sh = mglCreateShader(GL_FRAGMENT_SHADER);

	shader_source_and_compile(v_sh, GL_VERTEX_SHADER);
	shader_source_and_compile(f_sh, GL_FRAGMENT_SHADER);

	prg = mglCreateProgram();

	mglAttachShader(prg, v_sh);
	mglAttachShader(prg, f_sh);

	mglLinkProgram(prg);

	mglDetachShader(prg, v_sh);
	mglDetachShader(prg, f_sh);

	mglDeleteShader(v_sh);
	mglDeleteShader(f_sh);

	mglUseProgram(prg);
}


struct mg_font font;
void create_texture(void)
{
	//font = read_ttf("../Minecraft.ttf");
	font = read_ttf("../Hack-Regular.ttf");

	mglActiveTexture(GL_TEXTURE0);
	mglGenTextures(1, &tex);
	mglBindTexture(GL_TEXTURE_2D, tex);
	mglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	mglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	mglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	mglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	mglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font.atlas.width, font.atlas.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font.atlas.mem);
	mglGenerateMipmap(GL_TEXTURE_2D);

	int32_t loc = mglGetUniformLocation(prg, "font_atlas");
	mglUniform1i(loc, 0); /* TEXTURE0 */
}

void cleanup(void)
{
	mglDeleteVertexArrays(1, &vao);
	mglDeleteBuffers(1, &vbo);
	mglDeleteBuffers(1, &ebo);
	mglDeleteProgram(prg);
	mglDeleteTextures(1, &tex);
}

void init(void)
{
	mglEnable(GL_BLEND);
	mglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	create_buffers();
	create_vao();
	create_shader_program();
	create_texture();
}

struct text_rect {
	float pos[3];
	uint32_t pixel_width;
	uint32_t char_count;
	struct vertex *vertices;
	uint32_t *indices;
};

struct text_rect tr;
struct text_rect text_window_from_string(const struct mg_font *font, const uint32_t width, const char *str)
{
	const uint32_t len = strlen(str);
	struct text_rect tr =
	{
		.pos = { 100.0f, 620.0f, 0.0f},
		.pixel_width = width,
		.char_count = len,
		.vertices = malloc(4 * len * sizeof(struct vertex)),
		.indices = malloc(6 * len * sizeof(uint32_t)),
	};

	const int32_t first_char = font->first_char;
	
	int32_t x_offset = 0;
	int32_t y_offset = 0;
	for (uint32_t i = 0; i < len; ++i)
	{
		const struct codepoint_info *cinfo = &font->codepoint_table[str[i] - first_char];

		if (x_offset + cinfo->lsb + cinfo->width >= width)
		{
			y_offset -= font->glyph_height;
			x_offset = cinfo->lsb;
		}

		tr.indices[i*6 + 0] = i*4 + 0;
		tr.indices[i*6 + 1] = i*4 + 1;
		tr.indices[i*6 + 2] = i*4 + 2;
		tr.indices[i*6 + 3] = i*4 + 0;
		tr.indices[i*6 + 4] = i*4 + 2;
		tr.indices[i*6 + 5] = i*4 + 3;

		x_offset += cinfo->lsb;

		tr.vertices[i*4 + 0].pos[0] = tr.pos[0] + (x_offset + cinfo->width); 
		tr.vertices[i*4 + 0].pos[1] = tr.pos[1] + y_offset + cinfo->y_offset; 
		tr.vertices[i*4 + 0].pos[2] = tr.pos[2] + 0;       
		tr.vertices[i*4 + 0].uv[0] = cinfo->a_x + cinfo->a_width;
		tr.vertices[i*4 + 0].uv[1] = cinfo->a_y;

		tr.vertices[i*4 + 1].pos[0] = tr.pos[0] + x_offset + cinfo->width;
		tr.vertices[i*4 + 1].pos[1] = tr.pos[1] + y_offset + cinfo->height + cinfo->y_offset;
		tr.vertices[i*4 + 1].pos[2] = tr.pos[2] + 0;       
		tr.vertices[i*4 + 1].uv[0] = cinfo->a_x + cinfo->a_width;
		tr.vertices[i*4 + 1].uv[1] = cinfo->a_y + cinfo->a_height;

		tr.vertices[i*4 + 2].pos[0] = tr.pos[0] + x_offset;
		tr.vertices[i*4 + 2].pos[1] = tr.pos[1] + y_offset + cinfo->height + cinfo->y_offset;
		tr.vertices[i*4 + 2].pos[2] = tr.pos[2] + 0;       
		tr.vertices[i*4 + 2].uv[0] = cinfo->a_x;
		tr.vertices[i*4 + 2].uv[1] = cinfo->a_y + cinfo->a_height;
		
		tr.vertices[i*4 + 3].pos[0] = tr.pos[0] + x_offset;
		tr.vertices[i*4 + 3].pos[1] = tr.pos[1] + y_offset + cinfo->y_offset;
		tr.vertices[i*4 + 3].pos[2] = tr.pos[2] + 0;       
		tr.vertices[i*4 + 3].uv[0] = cinfo->a_x;
		tr.vertices[i*4 + 3].uv[1] = cinfo->a_y;

		x_offset += cinfo->advance;
	}

	for (uint32_t i = 0; i < 4*len; ++i)
	{
		tr.vertices[i].pos[0] = (tr.vertices[i].pos[0] * 2.0f / screen_width) - 1.0f; 
		tr.vertices[i].pos[1] = (tr.vertices[i].pos[1] * 2.0f / screen_height) - 1.0f; 
	}

	return tr;
}

GLuint t_vao, t_vbo, t_ebo;
void vao_from_text_rect(struct text_rect *tr)
{
	mglGenVertexArrays(1, &t_vao);
	mglBindVertexArray(t_vao);

	mglGenBuffers(1, &t_vbo);
	mglGenBuffers(1, &t_ebo);	
	mglBindBuffer(GL_ARRAY_BUFFER, t_vbo);
	mglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, t_ebo);
	mglBufferData(GL_ARRAY_BUFFER, tr->char_count * 4 * sizeof(struct vertex), tr->vertices, GL_DYNAMIC_DRAW);
	mglBufferData(GL_ELEMENT_ARRAY_BUFFER, tr->char_count * 6 * sizeof(uint32_t), tr->indices, GL_DYNAMIC_DRAW);

	mglEnableVertexAttribArray(0);
	mglEnableVertexAttribArray(1);
	mglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), 0);
	mglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)(3 * sizeof(float)));
}


HGLRC gl_context;
HDC device_context;
void init_gl(HWND *window_handle)
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;

	device_context = GetDC(*window_handle);
	int32_t pixel_format = ChoosePixelFormat(device_context, &pfd);
	if (pixel_format == 0)
	{
		LOG_MESSAGE(TAG_SYSTEM, SEVERITY_FATAL, 0, "Couldn't find appropriate pixel format for device context.", 0);
	}

	if (!SetPixelFormat(device_context, pixel_format, &pfd))
	{
		LOG_MESSAGE(TAG_SYSTEM, SEVERITY_FATAL, 0, "Failed to set pixel format for device context.", 0);
	}

	/* FAKE CONTEXT */
	HGLRC fake_context = wglCreateContext(device_context);
	if (fake_context == NULL)
	{
		LOG_MESSAGE(TAG_RENDERER, SEVERITY_FATAL, 0, "Failed to create GL context.", 0);
	}

	if (!wglMakeCurrent(device_context, fake_context))
	{
		LOG_MESSAGE(TAG_RENDERER, SEVERITY_FATAL, 0, "Failed to set GL context.", 0);
	}

	
	HGLRC (*wglCreateContextAttribsARB)(HDC, HGLRC, const int *) = NULL;
	wglCreateContextAttribsARB = (HGLRC (*)(HDC, HGLRC, const int *)) wglGetProcAddress("wglCreateContextAttribsARB");

	wglMakeCurrent(device_context, 0);
	wglDeleteContext(fake_context);
	
	const int attrib_list[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 6,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0
	};

	gl_context = wglCreateContextAttribsARB(device_context, 0, attrib_list);
	if (gl_context == NULL)
	{
		LOG_MESSAGE(TAG_RENDERER, SEVERITY_FATAL, 0, "Failed to create GL extended context.", 0);
	}

	if (!wglMakeCurrent(device_context, gl_context))
	{
		LOG_MESSAGE(TAG_RENDERER, SEVERITY_FATAL, 0, "Failed to set GL extended context.", 0);
	}
	
	mgl_init();
#ifdef MGL_DEBUG
	mgl_debug_init();
#endif
}

void frame(struct text_rect *tr, struct ui_state * ui_context)
{
	mglClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	mglClear(GL_COLOR_BUFFER_BIT);
	
	mglBindVertexArray(ui_vao);
	if (ui_context->depth >= 0) {
		i64 offset = 0;
		for (i32 i = 0;  i <= ui_context->depth; ++i) {
			mglDrawElements(GL_TRIANGLES, 6*ui_context->depth_batch[i], GL_UNSIGNED_INT, (const void *) offset);
			offset += 6 * ui_context->depth_batch[i] * sizeof(i32);
			if (ui_context->depth_text_batch[i] > 0) {
				mglDrawElements(GL_TRIANGLES, 6 * ui_context->depth_text_batch[i], GL_UNSIGNED_INT, (const void *) offset);
				offset += 6 * ui_context->depth_text_batch[i] * sizeof(i32);
			}
		}
	}

	SwapBuffers(device_context);

	free(drawbuffer);
	free(indexbuffer);
}

void ui_build(struct ui_state *ui_context)
{
	const vec4 bg_default = { 0.3f, 0.0f, 0.3f, 1.0f };
	const vec4 br_default =	{ 0.2f, 0.0f, 0.5f, 1.0f };
	const vec4 text_default = { 1.0f, 0.7f, 0.0f, 1.0f };
	ui_visual_set_default(ui_context, bg_default, br_default, text_default);

	struct ui_unit *root;

	struct ui_unit *parent;
	struct ui_unit *child1;
	struct ui_unit *child2;
	struct ui_unit *child3;
	struct ui_unit *grandchild;
	
	struct ui_unit *wparent;
	struct ui_unit *wchild1;
	struct ui_unit *wchild2;
	struct ui_unit *wchild3;
	struct ui_unit *wgrandchild;

	root = ui_dummy_window(ui_context, "Root");
	vec2_set(root->p_tl, -aspect_ratio, 1.0f);
	vec2_set(root->p_br, aspect_ratio, -1.0f);
	vec2_set(root->size.position, -aspect_ratio, 1.0f);
	vec2_set(root->size.size, 2 * aspect_ratio, 2.0f);
	ui_unit_push(ui_context, root);

	const vec4 bg = { 0.1f, 0.1f, 0.1f, 1.0f };
	const vec4 br = { 1.0f, 0.7f, 0.0f, 1.0f };

	BORDER_COLOR(ui_context, br)
	{
		BACKGROUND_COLOR(ui_context, bg)
		{
	 		parent = ui_dummy_window(ui_context, "Dummy Window");
			vec2_set(parent->p_tl, -1.0f, 1.0f);
			vec2_set(parent->p_br, 0.0f, 0.0f);
			vec2_set(parent->size.position, -1.0f, 1.0f);
			vec2_set(parent->size.size, 1.0f, 1.0f);
		}

		const vec4 red = { 0.5f, 0.0f, 0.0f, 1.0f };
		const vec4 green = { 0.0f, 0.5f, 0.0f, 1.0f };
		const vec4 blue = { 0.0f, 0.0f, 0.5f, 1.0f };

		ui_unit_push(ui_context, parent);
		BACKGROUND_COLOR(ui_context, red)
		{
			child1 = ui_dummy_window(ui_context, "Dummy Child 1");
			vec2_set(child1->p_tl, -0.9f, 0.9f);
			vec2_set(child1->p_br, -0.7f, 0.7f);
			vec2_set(child1->size.position, -0.9f, 0.9f);
			vec2_set(child1->size.size, 0.2f, 0.2f);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			child2 = ui_dummy_window(ui_context, "Dummy Child 2");
			vec2_set(child2->p_tl, -0.9f, 0.6f);
			vec2_set(child2->p_br, -0.7f, 0.4f);
			vec2_set(child2->size.position, -0.9f, 0.6f);
			vec2_set(child2->size.size, 0.2f, 0.2f);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			child3 = ui_dummy_window(ui_context, "Dummy Child 3");
			vec2_set(child3->p_tl, -0.9f, 0.3f);
			vec2_set(child3->p_br, -0.7f, 0.1f);
			vec2_set(child3->size.position, -0.9f, 0.3f);
			vec2_set(child3->size.size, 0.2f, 0.2f);
		}
		ui_unit_pop(ui_context);

		ui_unit_push(ui_context, child1);
		BACKGROUND_COLOR(ui_context, blue)
		{
			grandchild = ui_dummy_window(ui_context, "Dummy Grand Child");
			vec2_set(grandchild->p_tl, -0.85f, 0.85f);
			vec2_set(grandchild->p_br, -0.75f, 0.75f);
			vec2_set(grandchild->size.position, -0.85f, 0.85f);
			vec2_set(grandchild->size.size, 0.1f, 0.1f);
		}
		ui_unit_pop(ui_context);

		BACKGROUND_COLOR(ui_context, bg)
		{
	 		wparent = ui_dummy_window(ui_context, "WDummy Window");
			vec2_set(wparent->p_tl, 0.0f, 0.0f);
			vec2_set(wparent->p_br, 1.0f, -1.0f);
			vec2_set(wparent->size.position, 0.0f, 0.0f);
			vec2_set(wparent->size.size, 1.0f, 1.0f);
		}

		ui_unit_push(ui_context, wparent);
		BACKGROUND_COLOR(ui_context, red)
		{
			wchild1 = ui_dummy_window(ui_context, "Dummy WChild 1");
			vec2_set(wchild1->p_tl, 0.1f, -0.1f);
			vec2_set(wchild1->p_br, 0.3f, -0.3f);
			vec2_set(wchild1->size.position, 0.1f, -0.1f);
			vec2_set(wchild1->size.size, 0.2f, 0.2f);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			wchild2 = ui_dummy_window(ui_context, "Dummy WChild 2");
			vec2_set(wchild2->p_tl, 0.1f, -0.4f);
			vec2_set(wchild2->p_br, 0.3f, -0.6f);
			vec2_set(wchild2->size.position, 0.1f, -0.4f);
			vec2_set(wchild2->size.size, 0.2f, 0.2f);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			wchild3 = ui_dummy_window(ui_context, "Dummy WChild 3");
			vec2_set(wchild3->p_tl, 0.1f, -0.7f);
			vec2_set(wchild3->p_br, 0.3f, -0.9f);
			vec2_set(wchild3->size.position, 0.1f, -0.7f);
			vec2_set(wchild3->size.size, 0.2f, 0.2f);
		}
		ui_unit_pop(ui_context);

		ui_unit_push(ui_context, wchild1);
		BACKGROUND_COLOR(ui_context, blue)
		{
			wgrandchild = ui_dummy_window(ui_context, "Dummy Grand WChild");
			vec2_set(wgrandchild->p_tl, 0.15f, -0.15f);
			vec2_set(wgrandchild->p_br, 0.25f, -0.25f);
			vec2_set(wgrandchild->size.position, 0.15f, -0.15f);
			vec2_set(wgrandchild->size.size, 0.1f, 0.1f);
		}
		ui_unit_pop(ui_context);

		vec2 bar_position = { 0.05f, 0.85f };
		vec2 button_size = { 0.16f, 0.08f };
		struct ui_unit *bar = ui_dummy_bar(ui_context, "BAR", bar_position, button_size[1]);
		ui_unit_push(ui_context, bar);

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "BUTTON1", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "BUTTON2", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "BUTTON3", button_size);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "BUTTON4", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "BUTTON5", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "BUTTON6", button_size);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "BUTTON7", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "BUTTON8", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "BUTTON9", button_size);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "BUTTON10", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "BUTTON11", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "BUTTON12", button_size);
		}

		ui_unit_pop(ui_context);

		vec2_set(bar_position, -0.95f, -0.15f);
		bar = ui_dummy_list(ui_context, "2BAR", bar_position, button_size[0]);
		ui_unit_push(ui_context, bar);

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "2BUTTON1", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "2BUTTON2", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "2BUTTON3", button_size);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "2BUTTON4", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "2BUTTON5", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "2BUTTON6", button_size);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "2BUTTON7", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "2BUTTON8", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "2BUTTON9", button_size);
		}

		BACKGROUND_COLOR(ui_context, blue)
		{
			struct ui_unit *bchild1 = ui_dummy_button(ui_context, "2BUTTON10", button_size);
		}

		BACKGROUND_COLOR(ui_context, green)
		{
			struct ui_unit *bchild2 = ui_dummy_button(ui_context, "2BUTTON11", button_size);
		}

		BACKGROUND_COLOR(ui_context, red)
		{
			struct ui_unit *bchild3 = ui_dummy_button(ui_context, "2BUTTON12", button_size);
		}

		ui_unit_pop(ui_context);

		struct ui_unit *hollow_bar;
		const vec2 hollow_pos = { 0.1f, 0.6f };
		const vec2 hollow_size = { 1.5f, 0.1f };
		BACKGROUND_COLOR(ui_context, bg)
		{
			hollow_bar = ui_hollow_bar(ui_context, "hollow_bar", hollow_size);
		}
		vec2_copy(hollow_bar->size.position, hollow_pos);
		
		const vec4 bg_text = { 10.0f/256.0f, 203.0f/256.0f, 238.0f/256.0f, 1.0f};
		const vec4 text_color = { 1.0f, 0.0f, 0.0f, 1.0f }; 

		ui_unit_push(ui_context, hollow_bar);

		struct ui_unit *text_button_bar = ui_dummy_bar(ui_context, "text_button_bar", hollow_pos, hollow_size[1]);
		ui_unit_push(ui_context, text_button_bar);

		BACKGROUND_COLOR(ui_context, bg_text)
		{
			TEXT_COLOR(ui_context, text_color)
			{
				struct ui_unit *b1 = ui_text_button(ui_context, "TEXT_BUTTON1", 0.1f, 0.0f, &font, "Button1");
				struct ui_unit *b2 = ui_text_button(ui_context, "TEXT_BUTTON2", 0.1f, 0.0f, &font, "Button222");
				struct ui_unit *b3 = ui_text_button(ui_context, "TEXT_BUTTON3", 0.1f, 0.0f, &font, "Button33333");
				struct ui_unit *b4 = ui_text_button(ui_context, "TEXT_BUTTON4", 0.1f, 0.0f, &font, "Button4444444");
				struct ui_unit *b5 = ui_text_button(ui_context, "TEXT_BUTTON5", 0.1f, 0.0f, &font, "Button555555555");
			}
		}	
		ui_unit_pop(ui_context);
		ui_unit_pop(ui_context);
	}

	ui_unit_pop(ui_context);
}

void ui_buf_init(struct ui_state *ui_context)
{
	mglGenBuffers(1, &ui_vbo);
	mglGenBuffers(1, &ui_ebo);	
	mglBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
	mglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_ebo);
	mglBufferData(GL_ARRAY_BUFFER, 10000*sizeof(f32), NULL, GL_DYNAMIC_DRAW);
	mglBufferData(GL_ELEMENT_ARRAY_BUFFER, 60000*sizeof(i32), NULL, GL_DYNAMIC_DRAW);

	mglGenVertexArrays(1, &ui_vao);
	mglBindVertexArray(ui_vao);
	mglBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
	mglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_ebo);
	mglEnableVertexAttribArray(0);
	mglEnableVertexAttribArray(1);
	mglEnableVertexAttribArray(2);
	mglEnableVertexAttribArray(3);
	mglEnableVertexAttribArray(4);
	mglEnableVertexAttribArray(5);
	mglEnableVertexAttribArray(6);
	mglEnableVertexAttribArray(7);
	mglEnableVertexAttribArray(8);

	GLsizei stride = (GLsizei) ui_drawbuffer_stride();
	mglVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
	mglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(2  * sizeof(f32)));
	mglVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void *)(4  * sizeof(f32)));
	mglVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void *)(8  * sizeof(f32)));
	mglVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void *)(12 * sizeof(f32)));
	mglVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, stride, (void *)(16 * sizeof(f32)));
	mglVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, stride, (void *)(18 * sizeof(f32)));
	mglVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, stride, (void *)(19 * sizeof(f32)));
	mglVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, stride, (void *)(20 * sizeof(f32)));
}

void ui_buffer(struct ui_state *ui_context)
{
	vec2 screen_size = { (float) screen_width, (float) screen_height };

	if (ui_context->depth == -1) {
		return;
	}

	size_t dsize = ui_drawbuffer_size(ui_context);
	size_t isize = ui_indexbuffer_size(ui_context);
	drawbuffer = malloc(dsize);
	indexbuffer = malloc(isize);
	ui_drawbuffer_data(ui_context, drawbuffer, indexbuffer, &font, screen_size);

	mglBindVertexArray(ui_vao);
	mglBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
	mglBufferSubData(GL_ARRAY_BUFFER, 0, dsize, drawbuffer);
	mglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, isize, indexbuffer);
}

void mouse_lock_to_rect(RECT *win_rect)
{
	ClipCursor(win_rect);
}

void mouse_unlock_from_rect()
{
	ClipCursor(NULL);
}

void mouse_display()
{
	ShowCursor(TRUE);
}

void mouse_hide()
{
	ShowCursor(FALSE);
}

RECT win_screen_rect(HWND window_handle)
{
	POINT upper_left, lower_right;

	RECT screen_rect;
	GetClientRect(window_handle, &screen_rect);

	upper_left.x = screen_rect.left;
	upper_left.y = screen_rect.top;
	lower_right.x = screen_rect.right + 1;
	lower_right.y = screen_rect.bottom + 1;

	ClientToScreen(window_handle, &upper_left);
	ClientToScreen(window_handle, &lower_right);

	SetRect(&screen_rect, upper_left.x, upper_left.y, lower_right.x, lower_right.y);
	return screen_rect;
}

void window_input_init(HWND window_handle)
{
	RECT win_rect = win_screen_rect(window_handle);
	mouse_lock_to_rect(&win_rect);
}


int CALLBACK WinMain(HINSTANCE h_instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	mg_precision_timer_init();
	LOG_INIT("log.txt");

	window_input_init(window_handle);	
	minput.position.x = 0;
	minput.position.y = 0;
	minput.clicked = 0;
	minput.pressed = 0;
	minput.released = 0;

	init_gl(&window_handle);
	LOG_MESSAGE(TAG_RENDERER, SEVERITY_NOTE, 0, "GL Vendor           - %s", mglGetString(GL_VENDOR));
	LOG_MESSAGE(TAG_RENDERER, SEVERITY_NOTE, 0, "GL Renderer         - %s", mglGetString(GL_RENDERER));
	LOG_MESSAGE(TAG_RENDERER, SEVERITY_NOTE, 0, "GL Version          - %s", mglGetString(GL_VERSION));
	LOG_MESSAGE(TAG_RENDERER, SEVERITY_NOTE, 0, "GL Shading Language - %s", mglGetString(GL_SHADING_LANGUAGE_VERSION));

	init();
	tr = text_window_from_string(&font, 1000, lorem);
	vao_from_text_rect(&tr);

	ui_context = ui_context_create(128);
	ui_buf_init(&ui_context);

	MSG message;
	for (;;)
	{
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE) > 0) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (quit == 1) {
			break;
		}

		//BOOL message_result = GetMessage(&message, 0, 0, 0);
		//if (message_result > 0)	/* message != WM_QUIT */
		//{
		//	TranslateMessage(&message);
		//	DispatchMessage(&message);
		//}
		//else
		//{
		//	break;
		//}
		
		float vx, vy;
		vx = (((float) minput.position.x / screen_width) - 0.5f) * 2.0f * aspect_ratio;
		vy = (((float) (screen_height - minput.position.y) / screen_height) - 0.5f) * 2.0f;
		
		vec2 tmp1, tmp2;
		vec2_set(ui_context.comm.cursor, vx, vy);
		vec2_copy(tmp1, ui_context.comm.cursor);
		vec2_sub(tmp2, tmp1, ui_context.comm.delta),
		vec2_copy(ui_context.comm.delta, tmp2);

		ui_context.comm.pressed = minput.pressed;
		ui_context.comm.released = minput.released;
		ui_context.comm.clicked = minput.clicked;

		ui_build(&ui_context);
		ui_autolayout(&ui_context);
		ui_hot(&ui_context);
		ui_unit_depth_sort(NULL, &ui_context);
		ui_buffer(&ui_context);

		frame(&tr, &ui_context);

		ui_cache(&ui_context);
	}

	ui_context_destroy(&ui_context);
	cleanup();
	mgl_shutdown();
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(gl_context);
	DestroyWindow(window_handle);
	LOG_SHUTDOWN();

	return 0;
}
#endif
