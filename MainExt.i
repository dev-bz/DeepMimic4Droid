#undef main
int main(int _argc, char **_argv) {
	char arg[3][128] = {"a.out", "--arg_file", "data/args/kin_char_args.txt"};
	char *argv[3] = {arg[1], arg[2]};
	int argc = 2;
	FormatArgs(argc, argv, gArgs);
	InitDraw(argc, argv);
	SetupDeepMimicCore();
	for (int i = 0; i < 1000; ++i) {
		if (i % 10 == 9) {
			printf("Keyboard\n");
			Keyboard('x', 1, 1);
		}
		Update(gAnimStep);
	}
	gArgs[1] = "data/args/run_humanoid3d_robot_args.txt";
	Reload();
	for (int i = 0; i < 1000; ++i) {
		if (i % 10 == 9) {
			printf("Keyboard\n");
			Keyboard('x', 1, 1);
		}
		Update(gAnimStep);
	}
}
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