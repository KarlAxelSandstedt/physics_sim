#ifndef __MG_GL_H__
#define __MG_GL_H__

#include "mg_common.h"
#include "system_common.h"

#if __OS__ == __LINUX__

#include <GL/glx.h>

extern void 		(*mglXSwapBuffers)(Display *, GLXDrawable);
extern XVisualInfo *	(*mglXChooseVisual)(Display *, int, int *);
extern GLXContext	(*mglXCreateContext)(Display *, XVisualInfo *, GLXContext, Bool);
extern Bool		(*mglXMakeCurrent)(Display *, GLXDrawable, GLXContext);

#elif __OS__ == __WIN64__

#include <windows.h>
#include <GL\gl.h>
#include "GL\glcorearb.h"

#endif

/************************************************************************************************/
/*		      		MOSSY GROTTO GRAPHICS LIBRARY				 	*/
/************************************************************************************************/

/**
 * NOTES:
 * 	Debugging requires opengl 4.3.
 */

void mgl_init(void);
void mgl_debug_init(void);
void mgl_shutdown(void);

extern void 	(*mglSwapBuffers)(struct window *win);
extern const GLubyte * 	(*mglGetString)(GLenum);
extern void	(*mglGetShaderiv)(GLuint, GLenum, GLint *);
extern void	(*mglGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);

extern void 	(*mglClearColor)(GLclampf, GLclampf, GLclampf, GLclampf);
extern void	(*mglBlendFunc)(GLenum, GLenum);
extern void 	(*mglFinish)(void);
extern void 	(*mglFlush)(void);
extern void 	(*mglClear)(GLbitfield);
extern void 	(*mglEnable)(GLenum);
extern void 	(*mglDisable)(GLenum);
extern void	(*mglCullFace)(GLenum);
extern void	(*mglFrontFace)(GLenum);
extern void	(*mglPolygonMode)(GLenum, GLenum);

extern void	(*mglGenBuffers)(GLsizei, GLuint *);
extern void	(*mglBindBuffer)(GLenum, GLuint);
extern void	(*mglBufferData)(GLenum, GLsizeiptr, const void *, GLenum);
extern void	(*mglBufferSubData)(GLenum, GLintptr, GLsizeiptr,  const void *);
extern void	(*mglDeleteBuffers)(GLsizei, const GLuint *);

extern void	(*mglGenVertexArrays)(GLsizei, GLuint *);
extern void	(*mglBindVertexArray)(GLuint array);
extern void 	(*mglDeleteVertexArrays)(GLsizei, const GLuint *arrays);
extern void	(*mglEnableVertexAttribArray)(GLuint);
extern void	(*mglDisableVertexAttribArray)(GLuint);
extern void	(*mglVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
extern void	(*mglVertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const void *);
extern void	(*mglVertexAttribLPointer)(GLuint, GLint, GLenum, GLsizei, const void *);

extern GLuint	(*mglCreateShader)(GLenum);
extern void	(*mglShaderSource)(GLuint, GLsizei, const GLchar **, const GLint *);
extern void	(*mglCompileShader)(GLuint);
extern void	(*mglAttachShader)(GLuint, GLuint);
extern void	(*mglDetachShader)(GLuint, GLuint);
extern void	(*mglDeleteShader)(GLuint);

extern GLuint	(*mglCreateProgram)(void);
extern void	(*mglLinkProgram)(GLuint);
extern void	(*mglUseProgram)(GLuint);
extern void	(*mglDeleteProgram)(GLuint);

extern GLuint	(*mglGetUniformLocation)(GLuint, const GLchar *);
extern void	(*mglUniform1f)(GLint, GLfloat);
extern void	(*mglUniform2f)(GLint, GLfloat, GLfloat);
extern void	(*mglUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
extern void	(*mglUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
extern void	(*mglUniform1i)(GLint, GLint);
extern void	(*mglUniform2i)(GLint, GLint, GLint);
extern void	(*mglUniform3i)(GLint, GLint, GLint, GLint);
extern void	(*mglUniform4i)(GLint, GLint, GLint, GLint, GLint);
extern void	(*mglUniform1ui)(GLint, GLuint);
extern void	(*mglUniform2ui)(GLint, GLuint, GLuint);
extern void	(*mglUniform3ui)(GLint, GLuint, GLuint, GLuint);
extern void	(*mglUniform4ui)(GLint, GLuint, GLuint, GLuint, GLuint);
extern void	(*mglUniform1fv)(GLint, GLsizei, const GLfloat *);
extern void	(*mglUniform2fv)(GLint, GLsizei, const GLfloat *);
extern void	(*mglUniform3fv)(GLint, GLsizei, const GLfloat *);
extern void	(*mglUniform4fv)(GLint, GLsizei, const GLfloat *);
extern void	(*mglUniform1iv)(GLint, GLsizei, const GLint *);
extern void	(*mglUniform2iv)(GLint, GLsizei, const GLint *);
extern void	(*mglUniform3iv)(GLint, GLsizei, const GLint *);
extern void	(*mglUniform4iv)(GLint, GLsizei, const GLint *);
extern void	(*mglUniform1uiv)(GLint, GLsizei, const GLuint *);
extern void	(*mglUniform2uiv)(GLint, GLsizei, const GLuint *);
extern void	(*mglUniform3uiv)(GLint, GLsizei, const GLuint *);
extern void	(*mglUniform4uiv)(GLint, GLsizei, const GLuint *);
extern void	(*mglUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat *);
extern void	(*mglUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat *);

extern void	(*mglGenTextures)(GLsizei, GLuint *);
extern void	(*mglBindTexture)(GLenum, GLuint);
extern void	(*mglDeleteTextures)(GLsizei, GLuint *);
extern void	(*mglTexParameteri)(GLenum, GLenum, GLint);
extern void	(*mglTexParameterfv)(GLenum, GLenum, const GLfloat *);
extern void	(*mglTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
extern void	(*mglActiveTexture)(GLenum);
extern void	(*mglGenerateMipmap)(GLenum);

extern void 	(*mglDrawArrays)(GLenum, GLint, GLsizei);
extern void	(*mglDrawElements)(GLenum, GLsizei, GLenum, const void *);

extern void	(*mglViewport)(GLint, GLint, GLsizei, GLsizei);

#endif
