#pragma once

#include <GLES3/gl3.h>

#define glProgramUniform2fv(a,b,c,d) glUniform2fv(b,c,d)
#define glProgramUniform3fv(a,b,c,d) glUniform3fv(b,c,d)
#define glProgramUniform4fv(a,b,c,d) glUniform4fv(b,c,d)
#define glProgramUniformMatrix4fv(a,b,c,d,e) glUniformMatrix4fv(b,c,d,e)
#define glClearDepth glClearDepthf
#define glEnd()
#define glBegin(a)

#define glNormal3d(a, b, c)
#define glVertex3d(a, b, c)
#define glTexCoord2d(a, b)
#define glPointSize(a)
#define glFramebufferTexture3DEXT(...)
#define glDeleteFramebuffersEXT glDeleteFramebuffers
#define glDeleteRenderbuffersEXT glDeleteRenderbuffers
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0
#define GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE

#define glewInit()
#define glFramebufferTexture(a,b,d,e) glFramebufferTexture2D(a,b,GL_TEXTURE_2D,d,e)