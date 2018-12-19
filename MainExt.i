extern "C" void start(int argc, char **argv) {

	FormatArgs(argc, argv, gArgs);
	InitDraw(argc, argv);
}
extern "C" void startLoop(int width, int height) {
	SetupDeepMimicCore();
	Keyboard('k', 0, 0);
	gWinWidth = width;
	gWinHeight = height;
	SetupDraw();
	DrawMainLoop();
}
extern "C" void use_arg(const char *path, int cmd) {
	if (cmd == 0)
		load_actor(path);
	else if (gArgs.size() > 1) {
		gArgs[1] = path;
		Reload();
	}
}
#if 0
#include <LinearMath/btVector3.h>
extern "C" void drawTest();
extern "C" void drawMesh(const float *m, const void *v, const void *n,
                         const void *i, int size, const btVector3 c);
extern "C" void drawPlane() {
  {
    const int PLANE_SIZE = 4;
    static float mtx[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 4};
    static float plane[18] = {
        -PLANE_SIZE, 0, -PLANE_SIZE, -PLANE_SIZE, 0, PLANE_SIZE,
        PLANE_SIZE,  0, PLANE_SIZE,  -PLANE_SIZE, 0, -PLANE_SIZE,
        PLANE_SIZE,  0, PLANE_SIZE,  PLANE_SIZE,  0, -PLANE_SIZE};
    static float normal[18] = {0, 1, 0, 0, 1, 0, 0, 1, 0,
                               0, 1, 0, 0, 1, 0, 0, 1, 0};
    static btVector3 color = {0.4, 0.86, 0.48};
    drawMesh(mtx, plane, normal, 0, 6, color);
  }
  {
    const int PLANE_SIZE = 10;
    static float mtx[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -2, 0, 4};
    static float plane[18] = {
        -PLANE_SIZE, 0, -PLANE_SIZE, -PLANE_SIZE, 0, PLANE_SIZE,
        PLANE_SIZE,  0, PLANE_SIZE,  -PLANE_SIZE, 0, -PLANE_SIZE,
        PLANE_SIZE,  0, PLANE_SIZE,  PLANE_SIZE,  0, -PLANE_SIZE};
    static float normal[18] = {0, 1, 0, 0, 1, 0, 0, 1, 0,
                               0, 1, 0, 0, 1, 0, 0, 1, 0};
    static btVector3 color = {0.4, 0.6, 0.8};
    drawMesh(mtx, plane, normal, 0, 6, color);
  }
  drawTest();
}
#endif