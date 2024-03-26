#include "mg_common.h"

#include <assert.h>
#include <windows.h>
#include <GL\gl.h>
#include "glcorearb.h"

#include "mg_logger.h"

#define	LOAD_PROC(mgl_fp, type, gl_name)								\
	(mgl_fp) = (type) wglGetProcAddress((gl_name)); 						\
	if (mgl_fp == NULL) 										\
	{												\
		LOG_MESSAGE(TAG_SYSTEM, SEVERITY_FATAL, 0, "System could not load %s", (gl_name));	\
	}

void *gl_handle = NULL;

const GLubyte * (*mglGetString)(GLenum) = glGetString;
void 	(*mglClearColor)(GLclampf, GLclampf, GLclampf, GLclampf) = glClearColor;
void	(*mglBlendFunc)(GLenum, GLenum) = glBlendFunc;
void 	(*mglClear)(GLbitfield) = glClear;
void	(*mglFinish)(void) = glFinish;
void	(*mglFlush)(void) = glFlush;
void 	(*mglEnable)(GLenum) = glEnable;
void 	(*mglDisable)(GLenum) = glDisable;
void	(*mglViewport)(GLint, GLint, GLsizei, GLsizei) = glViewport;
void 	(*mglDebugMessageCallback)(GLDEBUGPROC, void *) = NULL;
void 	(*mglGenBuffers)(GLsizei, GLuint *) = NULL;
void 	(*mglBindBuffer)(GLenum, GLuint) = NULL;
void 	(*mglBufferData)(GLenum, GLsizeiptr, const void *, GLenum) = NULL;
void 	(*mglBufferSubData)(GLenum, GLintptr, GLsizeiptr,  const void *) = NULL;
void 	(*mglDeleteBuffers)(GLsizei, const GLuint *) = NULL;
void 	(*mglGenVertexArrays)(GLsizei, GLuint *) = NULL;
void 	(*mglBindVertexArray)(GLuint array) = NULL;
void 	(*mglDeleteVertexArrays)(GLsizei, const GLuint *arrays) = NULL;
void 	(*mglEnableVertexAttribArray)(GLuint) = NULL;
void 	(*mglDisableVertexAttribArray)(GLuint) = NULL;
void 	(*mglVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *) = NULL;
void 	(*mglVertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const void *) = NULL;
void 	(*mglVertexAttribLPointer)(GLuint, GLint, GLenum, GLsizei, const void *) = NULL;
GLuint 	(*mglCreateShader)(GLenum) = NULL;
void 	(*mglShaderSource)(GLuint, GLsizei, const GLchar **, const GLint *) = NULL;
void 	(*mglCompileShader)(GLuint) = NULL;
void 	(*mglAttachShader)(GLuint, GLuint) = NULL;
void 	(*mglDetachShader)(GLuint, GLuint) = NULL;
void 	(*mglDeleteShader)(GLuint) = NULL;
GLuint 	(*mglCreateProgram)(void) = NULL;
void 	(*mglLinkProgram)(GLuint) = NULL;
void 	(*mglUseProgram)(GLuint) = NULL;
void 	(*mglDeleteProgram)(GLuint) = NULL;
void 	(*mglDrawElements)(GLenum, GLsizei, GLenum, const void *) = NULL;
GLuint	(*mglGetUniformLocation)(GLuint, const GLchar *) = NULL;
void	(*mglUniform1f)(GLint, GLfloat) = NULL;
void	(*mglUniform2f)(GLint, GLfloat, GLfloat) = NULL;
void	(*mglUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = NULL;
void	(*mglUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void	(*mglUniform1i)(GLint, GLint) = NULL;
void	(*mglUniform2i)(GLint, GLint, GLint) = NULL;
void	(*mglUniform3i)(GLint, GLint, GLint, GLint) = NULL;
void	(*mglUniform4i)(GLint, GLint, GLint, GLint, GLint) = NULL;
void	(*mglUniform1ui)(GLint, GLuint) = NULL;
void	(*mglUniform2ui)(GLint, GLuint, GLuint) = NULL;
void	(*mglUniform3ui)(GLint, GLuint, GLuint, GLuint) = NULL;
void	(*mglUniform4ui)(GLint, GLuint, GLuint, GLuint, GLuint) = NULL;
void	(*mglUniform1fv)(GLint, GLsizei, const GLfloat *) = NULL;
void	(*mglUniform2fv)(GLint, GLsizei, const GLfloat *) = NULL;
void	(*mglUniform3fv)(GLint, GLsizei, const GLfloat *) = NULL;
void	(*mglUniform4fv)(GLint, GLsizei, const GLfloat *) = NULL;
void	(*mglUniform1iv)(GLint, GLsizei, const GLint *) = NULL;
void	(*mglUniform2iv)(GLint, GLsizei, const GLint *) = NULL;
void	(*mglUniform3iv)(GLint, GLsizei, const GLint *) = NULL;
void	(*mglUniform4iv)(GLint, GLsizei, const GLint *) = NULL;
void	(*mglUniform1uiv)(GLint, GLsizei, const GLuint *) = NULL;
void	(*mglUniform2uiv)(GLint, GLsizei, const GLuint *) = NULL;
void	(*mglUniform3uiv)(GLint, GLsizei, const GLuint *) = NULL;
void	(*mglUniform4uiv)(GLint, GLsizei, const GLuint *) = NULL;
void	(*mglUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void	(*mglGenTextures)(GLsizei, GLuint *) = NULL;
void	(*mglBindTexture)(GLenum, GLuint) = NULL;
void	(*mglDeleteTextures)(GLsizei, GLuint *) = NULL;
void	(*mglTexParameteri)(GLenum, GLenum, GLint) = glTexParameteri;
void	(*mglTexParameterfv)(GLenum, GLenum, const GLfloat *) = glTexParameterfv;
void	(*mglTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *) = glTexImage2D;
void	(*mglActiveTexture)(GLenum) = NULL;
void	(*mglGenerateMipmap)(GLenum) = NULL;
void	(*mglGetShaderiv)(GLuint, GLenum, GLint *) = NULL;
void	(*mglGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;

#ifdef MGL_DEBUG

#include "mg_logger.h"

void APIENTRY mgl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param)
{
	char *src_str = "";
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:		src_str = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:	src_str = "Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:	src_str = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:	src_str = "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:	src_str = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER:		src_str = "Other"; break;
	}

	char *type_str = "";
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR: 		type_str = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	type_str = "Deprecated Behavior"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: 	type_str = "Undefined Behavior"; break;
		case GL_DEBUG_TYPE_PORTABILITY:        	type_str = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:        	type_str = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER: 	       	type_str = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: 	       	type_str = "Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP: 	       	type_str = "Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER: 	       	type_str = "Other"; break;
	}

	char *severity_str = "";
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_NOTIFICATION:	severity_str = "Severity : Notification"; break;
		case GL_DEBUG_SEVERITY_LOW:	    	severity_str = "Severity : Low"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:	    	severity_str = "Severity : Medium"; break;
		case GL_DEBUG_SEVERITY_HIGH:	    	severity_str = "Severity : High"; break;
	}

	LOG_MESSAGE(TAG_OPENGL, SEVERITY_NOTE, 0, " [%s, %s, %s ] - %s", src_str, type_str, severity_str, message);
}

void mgl_debug_init(void)
{
	LOAD_PROC(mglDebugMessageCallback, void (*)(GLDEBUGPROC, void *), "glDebugMessageCallback");
	mglEnable(GL_DEBUG_OUTPUT);
	mglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	mglDebugMessageCallback(&mgl_debug_message_callback, NULL);
}

#endif

void mgl_functions_init(void)
{
	LOAD_PROC(mglGenBuffers, void (*)(GLsizei, GLuint *), "glGenBuffers");
	LOAD_PROC(mglBindBuffer, void (*)(GLenum, GLuint), "glBindBuffer");
	LOAD_PROC(mglBufferData, void (*)(GLenum, GLsizeiptr, const void *, GLenum), "glBufferData");
	LOAD_PROC(mglBufferSubData, void (*)(GLenum, GLintptr, GLsizeiptr,  const void *), "glBufferSubData");
	LOAD_PROC(mglDeleteBuffers, void (*)(GLsizei, const GLuint *), "glDeleteBuffers");
	LOAD_PROC(mglDrawElements, void (*)(GLenum, GLsizei, GLenum, const void *), "glDrawElements");
	LOAD_PROC(mglGenVertexArrays, void (*)(GLsizei, GLuint *), "glGenVertexArrays");
	LOAD_PROC(mglBindVertexArray, void (*)(GLuint), "glBindVertexArray");
	LOAD_PROC(mglDeleteVertexArrays, void (*)(GLsizei, const GLuint *), "glDeleteVertexArrays");
	LOAD_PROC(mglEnableVertexAttribArray, void (*)(GLuint), "glEnableVertexAttribArray");
	LOAD_PROC(mglDisableVertexAttribArray, void (*)(GLuint), "glDisableVertexAttribArray");
	LOAD_PROC(mglVertexAttribPointer, void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *), "glVertexAttribPointer");
	LOAD_PROC(mglVertexAttribIPointer, void (*)(GLuint, GLint, GLenum, GLsizei, const void *), "glVertexAttribIPointer");
	LOAD_PROC(mglVertexAttribLPointer, void (*)(GLuint, GLint, GLenum, GLsizei, const void *), "glVertexAttribLPointer");
	LOAD_PROC(mglCreateShader, GLuint (*)(GLenum), "glCreateShader");
	LOAD_PROC(mglShaderSource, void (*)(GLuint, GLsizei, const GLchar **, const GLint *), "glShaderSource");
	LOAD_PROC(mglCompileShader, void (*)(GLuint), "glCompileShader");
	LOAD_PROC(mglAttachShader, void (*)(GLuint, GLuint), "glAttachShader");
	LOAD_PROC(mglDetachShader, void (*)(GLuint, GLuint), "glDetachShader");
	LOAD_PROC(mglDeleteShader, void (*)(GLuint), "glDeleteShader");
	LOAD_PROC(mglCreateProgram, GLuint (*)(void), "glCreateProgram");
	LOAD_PROC(mglLinkProgram, void (*)(GLuint), "glLinkProgram");
	LOAD_PROC(mglUseProgram, void (*)(GLuint), "glUseProgram");
	LOAD_PROC(mglDeleteProgram,void (*)(GLuint), "glDeleteProgram");
	LOAD_PROC(mglGetUniformLocation, GLuint (*)(GLuint, const GLchar *), "glGetUniformLocation");
	LOAD_PROC(mglUniform1f, void (*)(GLint, GLfloat), "glUniform1f");
	LOAD_PROC(mglUniform2f, void (*)(GLint, GLfloat, GLfloat), "glUniform2f");
	LOAD_PROC(mglUniform3f, void (*)(GLint, GLfloat, GLfloat, GLfloat), "glUniform3f");
	LOAD_PROC(mglUniform4f, void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat), "glUniform4f");
	LOAD_PROC(mglUniform1i, void (*)(GLint, GLint), "glUniform1i");
	LOAD_PROC(mglUniform2i, void (*)(GLint, GLint, GLint), "glUniform2i");
	LOAD_PROC(mglUniform3i, void (*)(GLint, GLint, GLint, GLint), "glUniform3i");
	LOAD_PROC(mglUniform4i, void (*)(GLint, GLint, GLint, GLint, GLint), "glUniform4i");
	LOAD_PROC(mglUniform1ui, void (*)(GLint, GLuint), "glUniform1ui");
	LOAD_PROC(mglUniform2ui, void (*)(GLint, GLuint, GLuint), "glUniform2ui");
	LOAD_PROC(mglUniform3ui, void (*)(GLint, GLuint, GLuint, GLuint), "glUniform3ui");
	LOAD_PROC(mglUniform4ui, void (*)(GLint, GLuint, GLuint, GLuint, GLuint), "glUniform4ui");
	LOAD_PROC(mglUniform1fv, void (*)(GLint, GLsizei, const GLfloat *), "glUniform1fv");
	LOAD_PROC(mglUniform2fv, void (*)(GLint, GLsizei, const GLfloat *), "glUniform2fv");
	LOAD_PROC(mglUniform3fv, void (*)(GLint, GLsizei, const GLfloat *), "glUniform3fv");
	LOAD_PROC(mglUniform4fv, void (*)(GLint, GLsizei, const GLfloat *), "glUniform4fv");
	LOAD_PROC(mglUniform1iv, void (*)(GLint, GLsizei, const GLint *), "glUniform1iv");
	LOAD_PROC(mglUniform2iv, void (*)(GLint, GLsizei, const GLint *), "glUniform2iv");
	LOAD_PROC(mglUniform3iv, void (*)(GLint, GLsizei, const GLint *), "glUniform3iv");
	LOAD_PROC(mglUniform4iv, void (*)(GLint, GLsizei, const GLint *), "glUniform4iv");
	LOAD_PROC(mglUniform1uiv, void (*)(GLint, GLsizei, const GLuint *), "glUniform1uiv");
	LOAD_PROC(mglUniform2uiv, void (*)(GLint, GLsizei, const GLuint *), "glUniform2uiv");
	LOAD_PROC(mglUniform3uiv, void (*)(GLint, GLsizei, const GLuint *), "glUniform3uiv");
	LOAD_PROC(mglUniform4uiv, void (*)(GLint, GLsizei, const GLuint *), "glUniform4uiv");
	LOAD_PROC(mglUniformMatrix2fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix2fv");
	LOAD_PROC(mglUniformMatrix3fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix3fv");
	LOAD_PROC(mglUniformMatrix4fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix4fv");
	LOAD_PROC(mglUniformMatrix2x3fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix2x3fv");
	LOAD_PROC(mglUniformMatrix3x2fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix3x2fv");
	LOAD_PROC(mglUniformMatrix2x4fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix2x4fv");
	LOAD_PROC(mglUniformMatrix4x2fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix4x2fv");
	LOAD_PROC(mglUniformMatrix3x4fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix3x4fv");
	LOAD_PROC(mglUniformMatrix4x3fv, void (*)(GLint, GLsizei, GLboolean, const GLfloat *), "glUniformMatrix4x3fv");
	LOAD_PROC(mglGenTextures, void (*)(GLsizei, GLuint *), "glGenTextures");
	LOAD_PROC(mglBindTexture, void (*)(GLenum, GLuint), "glBindTexture");
	LOAD_PROC(mglDeleteTextures, void (*)(GLsizei, GLuint *), "glDeleteTextures");
	LOAD_PROC(mglActiveTexture, void (*)(GLenum), "glActiveTexture");
	LOAD_PROC(mglGenerateMipmap, void (*)(GLenum), "glGenerateMipmap");
	LOAD_PROC(mglGetShaderiv, void (*)(GLuint, GLenum, GLint *), "glGetShaderiv");
	LOAD_PROC(mglGetShaderInfoLog, void (*)(GLuint, GLsizei, GLsizei *, GLchar *), "glGetShaderInfoLog");
}

void mgl_init(void)
{
	mgl_functions_init();
}

void mgl_shutdown(void)
{

}
