#pragma once

//#include <GLES3/gl3.h>
#ifdef __cplusplus
#define GLUT_API extern "C"
#else
#define GLUT_API
#endif
#define GLUT_ELAPSED_TIME 0
#define GLUT_INIT_STATE 1
#define GLUT_FORWARD_COMPATIBLE 0
#define GLUT_CORE_PROFILE 0
#define GLUT_RGBA 0
#define GLUT_DOUBLE 0
#define GLUT_DEPTH 0


#define GLUT_LEFT_BUTTON 0
#define GLUT_RIGHT_BUTTON 1
#define GLUT_UP 0
#define GLUT_DOWN 1
#define GLUT_ACTIVE_ALT 0x10

typedef void (*TimerFunc)(int callback_val);
typedef void (*DrawFunc)(void);
typedef void (*ReshapeFunc)(int w,int h);
typedef void (*KeyboardFunc)(unsigned char key, int x, int y);
typedef void (*MouseClickFunc)(int button, int state, int x, int y);
typedef void (*MouseMoveFunc)(int x, int y);

GLUT_API void glutSwapBuffers();

GLUT_API void glutPostRedisplay();
GLUT_API int glutGet(int flag);
GLUT_API void glutTimerFunc(double timer_step, TimerFunc tf, int v);
GLUT_API void glutInit(int *argc, char **argv);

GLUT_API void glutInitContextVersion(int m, int n);
GLUT_API void glutInitContextFlags(int flags);
GLUT_API void glutInitContextProfile(int profile);

GLUT_API void glutInitDisplayMode(int mode);
GLUT_API void glutInitWindowSize(int width, int height);
GLUT_API void glutCreateWindow(const char *name);
GLUT_API void glutDisplayFunc(DrawFunc);
GLUT_API void glutReshapeFunc(ReshapeFunc);
GLUT_API void glutKeyboardFunc(KeyboardFunc);
GLUT_API void glutMouseFunc(MouseClickFunc);
GLUT_API void glutMotionFunc(MouseMoveFunc);
GLUT_API void glutMainLoop();

GLUT_API int glutGetModifiers();