/*
 * Copyright (c) 2006-2013 Erin Catto http://www.box2d.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#include "DebugDraw.h"
#if defined(__APPLE_CC__)
#include <OpenGL/gl3.h>
#else
//#include <glew/glew.h>
#include <GLES3/gl3.h>
#endif
//#include <glfw/glfw3.h>
//#include <glwk/glwk.h>
#include <stdarg.h>
#include <stdio.h>
#ifndef DEMO
#include <LinearMath/btVector3.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#endif
#define glBindVertexArray(a)       // glBindVertexArrayOES
#define glGenVertexArrays(a, b)    // glGenVertexArraysOES
#define glDeleteVertexArrays(a, b) // glDeleteVertexArraysOES
#include "RenderGL3.h"
#define BUFFER_OFFSET(x) ((const void *)(x))
DebugDraw g_debugDraw;
Camera g_camera;
Camera s_camera;
#ifdef DEMO
btIDebugDraw *debugDraw = &g_debugDraw;
#endif
// int g_width, g_height;
float shadow[16] = {0.8, 0, 0.6, 0, -0.6, 0, 0.8, 0, 0, -1, 0, 0, 0, 0, 0, 1};
// float shadow[16] = {1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1};
// float world[16] = {-0.6, 0, -0.8, 0, 0, 1, 0, 0, 0.8, 0, -0.6, 0, 0, -5, 0,
// 1};
float world[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, -7, 1};
float proj[16] = {0.0f};
float shadow_proj[16] = {0.0f};
float m_time = 0;
float eye = 0.08;
float scale = 1.0f;
extern "C" void debugCreate(int w, int h) {
  g_debugDraw.Create();
  s_camera.m_width = 10;
  s_camera.m_height = 10;
  g_camera.m_width = w;
  g_camera.m_height = h;
  g_camera.vr = w > h;
  const char *fontPath = "/system/fonts/DroidSansMono.ttf"; // VeraMono.ttf";
  int test = 0;
  if (RenderGLInit(fontPath, &test) == false) {
    fprintf(stderr, "Could not init GUI renderer.\n");
    assert(false);
  }
}
extern "C" void debugDestroy() { g_debugDraw.Destroy(); }
extern "C" void debugFlush() {
  g_debugDraw.Flush();
  glEnable(GL_BLEND);
  if (g_camera.vr)
    RenderGLFlush(0, 0);
  else
    RenderGLFlush(g_camera.m_width, g_camera.m_height);
}
extern "C" void drawMesh(const float *m, const void *v, const void *n,
                         const void *i, int size, const btVector3 c);
extern "C" void debugTest() {
  s_camera.BuildProjectionMatrix(shadow_proj, 0.2f);
  g_camera.BuildProjectionMatrix(proj, 0.2f);
  proj[0] /= scale;
  proj[5] /= scale;
  m_time += 0.0125;
  // world[14] = sin(m_time) * 2.5 - 5;
  shadow[14] = sin(m_time + 1.0) * 0.5 - 3.5; /*
   const btVector3 from(0, -100, 0);
   const btVector3 to(0, 100, 0);
   const btVector3 color(1, 1, 1);
   //g_debugDraw.drawLine(from, to, color);
   */
}
extern "C" void glPrintf(const char *fmt, const char *err);
//
static void sCheckGLError() {
  GLenum errCode = glGetError();
  if (errCode != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error = %d\n", errCode);
    // assert(false);
  }
}
// Prints shader compilation errors
static void sPrintLog(GLuint object) {
  GLint log_length = 0;
  if (glIsShader(object)) {
    GLint tmp = 0;
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    glGetShaderiv(object, GL_SHADER_SOURCE_LENGTH, &tmp);
    if (log_length < tmp) log_length = tmp;
  } else if (glIsProgram(object))
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else {
    glPrintf("%s", "printlog: Not a shader or a program\n");
    return;
  }
  char *log = (char *)malloc(log_length);
  if (glIsShader(object)) {
    GLint tmp = 0;
    glGetShaderSource(object, log_length, &tmp, log);
    glPrintf("Source:\n%s", log);
    glGetShaderInfoLog(object, log_length, NULL, log);
  } else if (glIsProgram(object))
    glGetProgramInfoLog(object, log_length, NULL, log);
  glPrintf("Log:\n%s", log);
  free(log);
}
void gPrintLog(GLuint object) {
  GLint compile_ok = GL_FALSE;
  glGetShaderiv(object, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    fprintf(stderr, "Error compiling shader!\n");
    sPrintLog(object);
    glDeleteShader(object);
  }
}
//
static GLuint sCreateShaderFromString(const char *source, GLenum type) {
  GLuint res = glCreateShader(type);
  const char *sources[] = {source};
  glShaderSource(res, 1, sources, NULL);
  glCompileShader(res);
  GLint compile_ok = GL_FALSE;
  glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    fprintf(stderr, "Error compiling shader of type %d!\n", type);
    sPrintLog(res);
    glDeleteShader(res);
    return 0;
  }
  return res;
}
//
static GLuint sCreateShaderProgram(const char *vs, const char *fs) {
  GLuint vsId = sCreateShaderFromString(vs, GL_VERTEX_SHADER);
  GLuint fsId = sCreateShaderFromString(fs, GL_FRAGMENT_SHADER);
  assert(vsId != 0 && fsId != 0);
  GLuint programId = glCreateProgram();
  glAttachShader(programId, vsId);
  glAttachShader(programId, fsId);
#ifndef __gl3_h_
  if (glBindFragDataLocation) glBindFragDataLocation(programId, 0, "color");
#endif
  glLinkProgram(programId);
  glDeleteShader(vsId);
  glDeleteShader(fsId);
  GLint status = GL_FALSE;
  glGetProgramiv(programId, GL_LINK_STATUS, &status);
  assert(status != GL_FALSE);
  return programId;
}
//
GLuint sCreateTargetTexture(int width, int height) {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
               GL_FLOAT, 0);
#if 0
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return texture;
}
int sCreateFrameBuffer(int width, int height, int targetTextureId) {
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  GLuint depthbuffer;
  glGenRenderbuffers(1, &depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depthbuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         targetTextureId, 0);
  int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "Framebuffer is not complete: %x", status);
  }
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return framebuffer;
}
//
struct GLRenderPoints {
  void Create() {
    const char *vs =
        "//#version 400\n"
        "uniform mat4 projectionMatrix;\n"
        "/*layout(location = 0) */attribute vec3 v_position;\n"
        "/*layout(location = 1) */attribute vec4 v_color;\n"
        "/*layout(location = 2) */attribute float v_size;\n"
        "varying vec4 f_color;\n"
        "void main(void)\n"
        "{\n"
        "	f_color = v_color;\n"
        "	gl_Position = projectionMatrix * vec4(v_position, 1.0);\n"
        "   gl_PointSize = v_size;\n"
        "}\n";
    const char *fs = "//#version 400\n"
                     "varying vec4 f_color;\n"
                     "//out vec4 color;\n"
                     "void main(void)\n"
                     "{\n"
                     "	gl_FragColor = f_color;\n"
                     "}\n";
    m_programId = sCreateShaderProgram(vs, fs);
    m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
#if 0
	glBindAttribLocation(m_programId, 0, "v_position");
	glBindAttribLocation(m_programId, 1, "v_color");
	glBindAttribLocation(m_programId, 2, "v_size");
        m_vertexAttribute = 0;
		m_colorAttribute = 1;
		m_sizeAttribute = 2;
#else
    m_vertexAttribute = /*0;*/ glGetAttribLocation(m_programId, "v_position");
    m_colorAttribute = /*1;*/ glGetAttribLocation(m_programId, "v_color");
    m_sizeAttribute = /*2;*/ glGetAttribLocation(m_programId, "v_size");
#endif
    // Generate
    glGenVertexArrays(1, &m_vaoId);
    glGenBuffers(3, m_vboIds);
    glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_colorAttribute);
    glEnableVertexAttribArray(m_sizeAttribute);
    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
    glVertexAttribPointer(m_sizeAttribute, 1, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_sizes), m_sizes, GL_DYNAMIC_DRAW);
    sCheckGLError();
    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    m_count = 0;
  }
  void Destroy() {
    if (m_vaoId) {
      glDeleteVertexArrays(1, &m_vaoId);
      glDeleteBuffers(2, m_vboIds);
      m_vaoId = 0;
    }
    if (m_programId) {
      glDeleteProgram(m_programId);
      m_programId = 0;
    }
  }
  void Vertex(const btVector3 &v, const btVector3 &c, float size) {
    if (m_count == e_maxVertices) Flush();
    m_vertices[m_count] = v;
    m_colors[m_count] = c;
    m_sizes[m_count] = size;
    ++m_count;
  }
  void Flush() {
    if (m_count == 0) return;
    glUseProgram(m_programId);
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);
    glBindVertexArray(m_vaoId);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(btVector3),
                    m_vertices);
    glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(btVector3), m_colors);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(float), m_sizes);
    glVertexAttribPointer(m_sizeAttribute, 1, GL_FLOAT, GL_FALSE, 0, 0); /*
     glEnableVertexAttribArray(m_vertexAttribute);
     glEnableVertexAttribArray(m_colorAttribute);
     glEnableVertexAttribArray(m_sizeAttribute);*/
#ifdef GL_PROGRAM_POINT_SIZE
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif
    glDrawArrays(GL_POINTS, 0, m_count);
#ifdef GL_PROGRAM_POINT_SIZE
    glDisable(GL_PROGRAM_POINT_SIZE);
#endif
    sCheckGLError();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    m_count = 0;
  }
  enum { e_maxVertices = 512 };
  btVector3 m_vertices[e_maxVertices];
  btVector3 m_colors[e_maxVertices];
  float m_sizes[e_maxVertices];
  int m_count;
  GLuint m_vaoId;
  GLuint m_vboIds[3];
  GLuint m_programId;
  GLint m_projectionUniform;
  GLint m_vertexAttribute;
  GLint m_colorAttribute;
  GLint m_sizeAttribute;
};
//
struct GLRenderLines {
  void Create() {
    const char *vs = "//#version 400\n"
                     "uniform mat4 projectionMatrix;\n"
                     "uniform mat4 worldMatrix;\n"
                     "/*layout(location = 0) */attribute vec3 v_position;\n"
                     "/*layout(location = 1) */attribute vec4 v_color;\n"
                     "varying vec4 f_color;\n"
                     "void main(void)\n"
                     "{\n"
                     "	f_color = vec4(v_color.rgb,1.0);\n"
                     "	gl_Position = projectionMatrix * worldMatrix* "
                     "vec4(v_position, 1.0);\n"
                     "}\n";
    const char *fs = "//#version 400\n"
                     "varying vec4 f_color;\n"
                     "//out vec4 color;\n"
                     "void main(void)\n"
                     "{\n"
                     "	gl_FragColor = f_color;\n"
                     "}\n";
    m_programId = sCreateShaderProgram(vs, fs);
    m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
    m_worldUniform = glGetUniformLocation(m_programId, "worldMatrix");
#if 0
	glBindAttribLocation(m_programId, 0, "v_position");
	glBindAttribLocation(m_programId, 1, "v_color");
        m_vertexAttribute = 0;
		m_colorAttribute = 1;
#else
    m_vertexAttribute = /*0;*/ glGetAttribLocation(m_programId, "v_position");
    m_colorAttribute = /*1;*/ glGetAttribLocation(m_programId, "v_color");
#endif
    // Generate
    glGenVertexArrays(1, &m_vaoId);
    glGenBuffers(2, m_vboIds);
    glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_colorAttribute);
    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);
    sCheckGLError();
    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    m_count = 0;
    glFlush();
  }
  void Destroy() {
    if (m_vaoId) {
      glDeleteVertexArrays(1, &m_vaoId);
      glDeleteBuffers(2, m_vboIds);
      m_vaoId = 0;
    }
    if (m_programId) {
      glDeleteProgram(m_programId);
      m_programId = 0;
    }
  }
  void Vertex(const btVector3 &v, const btVector3 &c) {
    if (m_count == e_maxVertices) Flush();
    m_vertices[m_count] = v;
    m_colors[m_count] = c;
    ++m_count;
  }
  void Flush() {
    if (m_count == 0) return;
    glUseProgram(m_programId);
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);
    glBindVertexArray(m_vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(btVector3),
                    m_vertices);
    glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(btVector3), m_colors);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), 0);
    if (g_camera.vr) {
      glViewport(0, 0, g_camera.m_width / 2, g_camera.m_height);
      world[12] = eye;
      glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, world);
      glDrawArrays(GL_LINES, 0, m_count);
      glViewport(g_camera.m_width / 2, 0, g_camera.m_width / 2,
                 g_camera.m_height);
      world[12] = -eye;
    }
    {
      glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, world);
      glDrawArrays(GL_LINES, 0, m_count);
    }
    sCheckGLError();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    m_count = 0;
  }
  enum { e_maxVertices = 2 * 512 };
  btVector3 m_vertices[e_maxVertices];
  btVector3 m_colors[e_maxVertices];
  int m_count;
  GLuint m_vaoId;
  GLuint m_vboIds[2];
  GLuint m_programId;
  GLint m_projectionUniform, m_worldUniform;
  GLint m_vertexAttribute;
  GLint m_colorAttribute;
};
//
struct GLRenderTriangles {
  void Create() {
    const char *vs = "#version 310 es\n"
                     "uniform mat4 projectionMatrix;\n"
                     "uniform mat4 worldMatrix;\n"
                     "layout(location = 0) in vec3 vposition;\n"
                     "layout(location = 1) in vec4 vcolor;\n"
                     "out vec4 fcolor;\n"
                     "void main()\n"
                     "{\n"
                     "	fcolor = vcolor;\n"
                     "	gl_Position = projectionMatrix * worldMatrix * "
                     "vec4(vposition, 1.0);\n"
                     "}\n";
    const char *fs = "#version 310 es\n"
                     "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
                     "precision highp float;\n"
                     "#else\n"
                     "precision mediump float;\n"
                     "#endif\n"
                     "in vec4 fcolor;\n"
                     "out vec4 color;\n"
                     "void main()\n"
                     "{\n"
                     "	color = fcolor;\n"
                     "}\n";
    m_programId = sCreateShaderProgram(vs, fs);
    m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
    m_worldUniform = glGetUniformLocation(m_programId, "worldMatrix");
#if 0
	glBindAttribLocation(m_programId, 0, "v_position");
	glBindAttribLocation(m_programId, 1, "v_color");
        m_vertexAttribute = 0;
	m_colorAttribute = 1;
#else
    m_vertexAttribute = /*0;*/ glGetAttribLocation(m_programId, "vposition");
    m_colorAttribute = /*1;*/ glGetAttribLocation(m_programId, "vcolor");
#endif
    // Generate
    glGenVertexArrays(1, &m_vaoId);
    glGenBuffers(2, m_vboIds);
    glBindVertexArray(m_vaoId);
    glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_colorAttribute);
    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), BUFFER_OFFSET(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_DYNAMIC_DRAW);
    sCheckGLError();
    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    m_count = 0;
  }
  void Destroy() {
    if (m_vaoId) {
      glDeleteVertexArrays(1, &m_vaoId);
      glDeleteBuffers(2, m_vboIds);
      m_vaoId = 0;
    }
    if (m_programId) {
      glDeleteProgram(m_programId);
      m_programId = 0;
    }
  }
  void Vertex(const btVector3 &v, const btVector4 &c) {
    if (m_count == e_maxVertices) Flush();
    m_vertices[m_count] = v;
    m_colors[m_count] = c;
    ++m_count;
  }
  void Flush() {
    if (m_count == 0) return;
    glUseProgram(m_programId);
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);
    glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, world);
    glBindVertexArray(m_vaoId);
    /*glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_colorAttribute);*/
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(btVector3),
                    m_vertices);
    glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), 0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_count * sizeof(btVector3), m_colors);
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE,
                          sizeof(btVector3), 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, m_count);
    glDisable(GL_BLEND);
    sCheckGLError();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    m_count = 0;
  }
  enum { e_maxVertices = 3 * 512 };
  btVector3 m_vertices[e_maxVertices];
  btVector4 m_colors[e_maxVertices];
  int m_count;
  GLuint m_vaoId;
  GLuint m_vboIds[2];
  GLuint m_programId;
  GLint m_projectionUniform;
  GLint m_worldUniform;
  GLint m_vertexAttribute;
  GLint m_colorAttribute;
};
// extern float rotate;
struct GLRenderObjects {
  struct tMesh {
    const void *vert;
    const void *normal;
    GLfloat color[4];
    const void *index;
    int size;
    GLfloat mtx[16];
  };
  void Create() {
    const GLchar *v2d = "#version 310 es\n"
                        "layout(location=0)in vec4 aPosition;\n"
                        "void main(){\n"
                        "  gl_Position=vec4(aPosition.xyz*0.75,1.0);\n"
                        "}";
    const GLchar *f2d = "#version 310 es\n"
                        "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
                        "precision highp float;\n"
                        "#else\n"
                        "precision mediump float;\n"
                        "#endif\n"
                        "out vec4 fragColor;\n"
                        "uniform sampler2D t;\n"
                        "void main(){\n"
                        "fragColor=texture(t,gl_PointCoord*0.5+0.5);\n"
                        "}";
    m_programId2d = sCreateShaderProgram(v2d, f2d);
    if (!m_programId2d) return;
    const char *s_vs =
        "//#version 400\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform mat4 modelMatrix;\n"
        "uniform mat4 worldMatrix;\n"
        "/*layout(location = 0) */attribute vec3 v_position;\n"
        "varying float f_color;\n"
        "void main(void){\n"
        "	vec4 p = worldMatrix* modelMatrix * vec4(v_position, 1.0);\n"
        "  f_color=p.z/p.w;\n"
        "	gl_Position = projectionMatrix *p;\n"
        "}\n";
    const char *s_fs = "//#version 400\n"
                       "varying float f_color;\n"
                       "void main(void){\n"
                       "	gl_FragColor.z = f_color;\n"
                       "}\n";
    s_programId = sCreateShaderProgram(s_vs, s_fs);
    if (!s_programId) return;
    s_projectionUniform = glGetUniformLocation(s_programId, "projectionMatrix");
    s_modelUniform = glGetUniformLocation(s_programId, "modelMatrix");
    s_worldUniform = glGetUniformLocation(s_programId, "worldMatrix");
#if 0
	glBindAttribLocation(m_programId, 0, "v_position");
	glBindAttribLocation(m_programId, 1, "v_color");
        m_vertexAttribute = 0;
		m_colorAttribute = 1;
#else
    s_vertexAttribute = /*0;*/ glGetAttribLocation(s_programId, "v_position");
#endif
    const char *vs = "#version 310 es\n"
                     "uniform mat4 projectionMatrix;\n"
                     "uniform mat4 projectionShadowMatrix;\n"
                     "uniform mat4 modelMatrix;\n"
                     "uniform mat4 worldMatrix;\n"
                     "uniform mat4 shadowMatrix;\n"
                     //"uniform float deep;\n"
                     "layout(location=0) in vec3 vposition;\n"
                     "layout(location=1) in vec3 vnormal;\n"
                     "layout(location=2) in vec4 vcolor;\n"
                     "out vec4 fposition;\n"
                     "out vec4 fcolor;\n"
                     "out vec3 fshadow;\n"
                     "out vec3 fnormal;\n"
                     "void main(){\n"
                     "	fcolor = vcolor;\n"
                     "  vec4 p= modelMatrix* vec4(vposition, 1.0);\n"
                     "  fnormal= mat3(shadowMatrix* modelMatrix) * vnormal;\n"
                     "	fshadow = vec3(shadowMatrix * p)/p.w+fnormal*0.075;\n"
                     "	fposition = projectionShadowMatrix *shadowMatrix*p;\n"
                     "	gl_Position = projectionMatrix *worldMatrix*p;\n"
                     "}\n";
#define BEST_SHADOW
    const char *fs =
        "#version 310 es\n"
        "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
        "precision highp float;\n"
        "#else\n"
        "precision mediump float;\n"
        "#endif\n"
        "in vec4 fposition;\n"
        "in vec4 fcolor;\n"
        "in vec3 fshadow;\n"
        "in vec3 fnormal;\n"
        "out vec4 oColor;\n"
        "uniform sampler2D t;\n"
#ifdef BEST_SHADOW
        "\
const vec2 coord[16]=vec2[16](\
vec2(-0.6474742,0.6028621),\
vec2( 0.0939157, 0.6783564 ),\
vec2( -0.3371512, 0.04865054 ),\
vec2( -0.4010732, 0.914994 ),\
vec2( -0.2793565, 0.4456959 ),\
vec2( -0.6683437, -0.1396244 ),\
vec2( -0.6369296, -0.6966243 ),\
vec2( -0.2684143, -0.3756073 ),\
vec2( 0.1146429, -0.8692533 ),\
vec2( 0.0697926, 0.01110036 ),\
vec2( 0.4677842, 0.5375957 ),\
vec2( 0.377133, -0.3518053 ),\
vec2( 0.6722369, 0.03702459 ),\
vec2( 0.6890426, -0.5889201 ),\
vec2( -0.8208677, 0.2444565 ),\
vec2( 0.8431721, 0.3903837 ));\n"
#endif
        "void main(){\n"
        "  vec2 ss=(vec2(fposition)*0.5/fposition.w)+0.5;\n"
        "  float light=3.0/(2.0+dot(fshadow,fshadow*0.05));\n"
        "  vec3 dir=normalize(fshadow);\n"
        "  float dt=dot(dir,fnormal);\n"
        "  float lt=acos(dt)*0.35/3.14159;\n"
        "if(ss.x>=0.0 && ss.x<=1.0 && ss.y>=0.0 && ss.y<=1.0){"
#ifdef BEST_SHADOW
        "float theta=fract((dot(fshadow.xy,vec2(0.0,80.0)))*43.5453);\n"
        "float sh=0.0;\n"
        "theta=2.0*3.14159*theta-3.14159;\n"
        "vec2 q=1.0-ss*2.0;float ct=pow(min(1.0,dot(q,q)),2.0);"
        "for(int i=0;i<16;++i){\n"
        " float co=cos(theta);float si=sin(theta);\n"
        " vec2 "
        "sss=vec2(coord[i].x*co-coord[i].y*si,coord[i].x*si+coord[i].y*co);\n"
        " sh+=fshadow.z>texture(t, ss+sss*0.005).z?1.0:ct;\n"
        "}\n"
        "lt=sh*0.0625*max(0.0,-dt)*0.65+lt;//0.05882353;\n"
#else
        "  float s = texture(t,ss).z;\n"
        "  vec2 q=1.0-ss*2.0;float ct=pow(min(1.0,dot(q,q)),2.0);"
        "  lt=(s<fshadow.z?1.0:ct)*max(0.0,-dt)*0.65+lt;\n"
#endif
        "}else lt=max(0.0,-dt)*0.65+lt;\n"
        "oColor=lt*light*fcolor;\n"
        "}\n";
    m_programId = sCreateShaderProgram(vs, fs);
    if (!m_programId) return;
    m_projectionUniform = glGetUniformLocation(m_programId, "projectionMatrix");
    m_projectionShadowUniform =
        glGetUniformLocation(m_programId, "projectionShadowMatrix");
    m_modelUniform = glGetUniformLocation(m_programId, "modelMatrix");
    m_worldUniform = glGetUniformLocation(m_programId, "worldMatrix");
    m_shadowUniform = glGetUniformLocation(m_programId, "shadowMatrix");
    // m_deepUniform = glGetUniformLocation(m_programId, "deep");
#if 0
	glBindAttribLocation(m_programId, 0, "v_position");
	glBindAttribLocation(m_programId, 1, "v_color");
        m_vertexAttribute = 0;
		m_colorAttribute = 1;
#else
    m_vertexAttribute = /*0;*/ glGetAttribLocation(m_programId, "vposition");
    m_normalAttribute = /*1;*/ glGetAttribLocation(m_programId, "vnormal");
    m_colorAttribute = /*2;*/ glGetAttribLocation(m_programId, "vcolor");
#endif
    // Generate
    sCheckGLError();
    // Vertex buffer
    mTargetTexture =
        sCreateTargetTexture(mFramebufferWidth, mFramebufferHeight);
    mFramebuffer = sCreateFrameBuffer(mFramebufferWidth, mFramebufferHeight,
                                      mTargetTexture);
    m_count = 0;
  }
  void Destroy() {
    if (m_programId) {
      glDeleteProgram(m_programId);
      m_programId = 0;
    }
  }
  void Vertex(const float *m, const void *v, const void *n, const void *i,
              int size, const btVector3 &c) {
    if (m_count == e_maxVertices) Flush();
    auto &shape = m_vertices[m_count];
    shape.vert = v;
    shape.normal = n;
    shape.index = i;
    shape.size = size;
    shape.color[0] = c.x();
    shape.color[1] = c.y();
    shape.color[2] = c.z();
    shape.color[3] = 1.0f;
    for (int i = 0; i < 16; ++i) shape.mtx[i] = m[i];
    ++m_count;
  }
#define SHADOW 1
  void Flush() {
    if (m_count == 0) return;
    glDisable(GL_BLEND);
#if 1
    glUseProgram(s_programId);
    // glBindVertexArray(0);
    glUniformMatrix4fv(s_projectionUniform, 1, GL_FALSE, shadow_proj);
    glUniformMatrix4fv(s_worldUniform, 1, GL_FALSE, shadow);
    glEnableVertexAttribArray(s_vertexAttribute);
#if SHADOW
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glClearColor(0.5, 0.4, -1000.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, mFramebufferWidth, mFramebufferHeight);
#endif
    for (int i = 0; i < m_count; ++i) {
      auto &shape = m_vertices[i];
      glUniformMatrix4fv(s_modelUniform, 1, GL_FALSE, m_vertices[i].mtx);
      glVertexAttribPointer(s_vertexAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                            shape.vert);
      if (shape.index) {
        glDrawElements(GL_TRIANGLES, shape.size, GL_UNSIGNED_SHORT,
                       shape.index);
      } else {
        glDrawArrays(GL_TRIANGLES, 0, shape.size);
      }
    }
#if SHADOW
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.2, 0.3, 0.4, 1.0);
    // glViewport(0, 0, g_camera.m_width, g_camera.m_height);
#endif
#endif
#if 1
    glUseProgram(m_programId);
    // glBindVertexArray(0);
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, proj);
    glUniformMatrix4fv(m_projectionShadowUniform, 1, GL_FALSE, shadow_proj);
    glUniformMatrix4fv(m_shadowUniform, 1, GL_FALSE, shadow);
    // glUniform1f(m_deepUniform, rotate);
    glEnableVertexAttribArray(m_vertexAttribute);
    glEnableVertexAttribArray(m_normalAttribute);
    glEnableVertexAttribArray(m_colorAttribute);
    glVertexAttribDivisor(m_colorAttribute, 1);
    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    if (g_camera.vr) {
      glViewport(0, 0, g_camera.m_width / 2, g_camera.m_height);
      world[12] = eye;
      glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, world);
      for (int i = 0; i < m_count; ++i) {
        auto &shape = m_vertices[i];
        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, m_vertices[i].mtx);
        glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              shape.vert);
        glVertexAttribPointer(m_normalAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              shape.normal);
        glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0,
                              shape.color);
        if (shape.index) {
          glDrawElements(GL_TRIANGLES, shape.size, GL_UNSIGNED_SHORT,
                         shape.index);
        } else {
          glDrawArrays(GL_TRIANGLES, 0, shape.size);
        }
      }
      glViewport(g_camera.m_width / 2, 0, g_camera.m_width / 2,
                 g_camera.m_height);
      world[12] = -eye;
    } else
      glViewport(0, 0, g_camera.m_width, g_camera.m_height);
    {
      glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, world);
      for (int i = 0; i < m_count; ++i) {
        auto &shape = m_vertices[i];
        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, m_vertices[i].mtx);
        glVertexAttribPointer(m_vertexAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              shape.vert);
        glVertexAttribPointer(m_normalAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              shape.normal);
        glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0,
                              shape.color);
        if (shape.index) {
          glDrawElements(GL_TRIANGLES, shape.size, GL_UNSIGNED_SHORT,
                         shape.index);
        } else {
          glDrawArrays(GL_TRIANGLES, 0, shape.size);
        }
      }
    }
    glVertexAttribDivisor(m_colorAttribute, 0);
#else
    glUseProgram(m_programId2d);
    glBindTexture(GL_TEXTURE_2D, mTargetTexture);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, plane);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
    glUseProgram(0);
    // glDisableVertexAttribArray(m_normalAttribute);
    // glDisableVertexAttribArray(m_vertexAttribute);
    m_count = 0;
  }
  enum { e_maxVertices = 3 * 512 };
  tMesh m_vertices[e_maxVertices];
  int m_count;
  GLuint m_programId, s_programId, m_programId2d;
  GLint m_projectionUniform, m_projectionShadowUniform, m_modelUniform,
      m_worldUniform, m_shadowUniform;
  GLuint m_deepUniform;
  GLint s_projectionUniform, s_modelUniform, s_worldUniform;
  GLint m_vertexAttribute, m_normalAttribute, m_colorAttribute;
  GLint s_vertexAttribute;
  int mFramebuffer;
  int mFramebufferWidth = 512;
  int mFramebufferHeight = 512;
  GLuint mTargetTexture;
  GLfloat plane[12] = {-1, 1, 0, 1, 1, 0, 1, -1, 0, -1, -1, 0};
};
//
DebugDraw::DebugDraw() {
  m_points = NULL;
  m_lines = NULL;
  m_triangles = NULL;
  m_objects = NULL;
}
//
DebugDraw::~DebugDraw() {}
//
void DebugDraw::Create() {
  m_points = new GLRenderPoints;
  m_points->Create();
  m_lines = new GLRenderLines;
  m_lines->Create();
  m_triangles = new GLRenderTriangles;
  m_triangles->Create();
  m_objects = new GLRenderObjects;
  m_objects->Create();
}
//
void DebugDraw::Destroy() {
  m_points->Destroy();
  delete m_points;
  m_points = NULL;
  m_lines->Destroy();
  delete m_lines;
  m_lines = NULL;
  m_triangles->Destroy();
  delete m_triangles;
  m_triangles = NULL;
  m_objects->Destroy();
  delete m_objects;
  m_objects = NULL;
}
#ifdef DEMO
void DebugDraw::drawLine(const btVector3 &from, const btVector3 &to,
                         const btVector3 &color) {
  m_lines->Vertex(from, color);
  m_lines->Vertex(to, color);
}
void DebugDraw::drawContactPoint(const btVector3 &PointOnB,
                                 const btVector3 &normalOnB, btScalar distance,
                                 int lifeTime, const btVector3 &color) {}
void DebugDraw::reportErrorWarning(const char *warningString) {}
void DebugDraw::draw3dText(const btVector3 &location, const char *textString) {}
void DebugDraw::setDebugMode(int debugMode) {}
int DebugDraw::getDebugMode() const {
  return btIDebugDraw::DBG_DrawContactPoints | btIDebugDraw::DBG_DrawWireframe;
}
#endif
void DebugDraw::Flush() {
  m_objects->Flush();
  m_triangles->Flush();
  m_lines->Flush();
  m_points->Flush();
}
extern "C" void drawMesh(const float *m, const void *v, const void *n,
                         const void *i, int size, const btVector3 c) {
  g_debugDraw.m_objects->Vertex(m, v, n, i, size, c);
}
extern "C" void drawTest() {
  btVector4 c(1, 1, 1, 0.5);
  g_debugDraw.m_triangles->Vertex(btVector3(1, 0, 0.1), c);
  g_debugDraw.m_triangles->Vertex(btVector3(-1, 0, -0.1), c);
  g_debugDraw.m_triangles->Vertex(btVector3(0, 3, 0), c);
  g_debugDraw.m_triangles->Vertex(btVector3(-1, 0, -0.1), c);
  g_debugDraw.m_triangles->Vertex(btVector3(1, 0, 0.1), c);
  g_debugDraw.m_triangles->Vertex(btVector3(0, 3, 0), c);
}
extern "C" void drawLine(const float *m, const void *v, const void *n,
                         const int i, int size, const btVector3 c) {
  const float *mm = (const float *)v;
  for (int j = i + 51; j < size + i; ++j) {
    int p = (j % size) * 3;
    int pp = ((j + 1) % size) * 3;
    g_debugDraw.m_lines->Vertex(btVector3(mm[p + 0], mm[p + 1], mm[p + 2]), c);
    g_debugDraw.m_lines->Vertex(btVector3(mm[pp + 0], mm[pp + 1], mm[pp + 2]),
                                c);
  }
  // g_debugDraw.m_lines->Flush();
}
extern "C" void gluPerspectivef(GLfloat *m, GLfloat fovy, GLfloat aspect,
                                GLfloat zNear, GLfloat zFar);
void Camera::BuildProjectionMatrix(float *m, float zBias) {
  if (vr)
    gluPerspectivef(m, 40.00, m_width * 0.5f / m_height, zBias, zBias * 1000);
  else
    gluPerspectivef(m, 60.00, (float)m_width / m_height, zBias, zBias * 1000);
  /*float w = float(m_width);
  float h = float(m_height);
  float ratio = w / h;

  m[0] = 2.0f / (50*ratio);
  m[1] = 0.0f;
  m[2] = 0.0f;
  m[3] = 0.0f;
  m[4] = 0.0f;
  m[5] = 2.0f / (50);
  m[6] = 0.0f;
  m[7] = 0.0f;
  m[8] = 0.0f;
  m[9] = 0.0f;
  m[10] = 1.0f;
  m[11] = 0.0f;
  m[12] = 0;
  m[13] = 0;
  m[14] = zBias;
  m[15] = 1.0f;*/
}