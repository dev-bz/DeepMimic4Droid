#include "ui/imgui.h"
#include <LinearMath/btVector3.h>
#include <dirent.h>
#include <stdio.h>
#include <string>
#include <vector>
void Keyboard(unsigned char key, int x, int y);
static int state_scroll = 0, save_scroll = 0;
static bool state_show = false;
extern int running;
extern double gUpdatesPerSec;
bool over = false;
#define USE_UI
namespace ui {
static const int BUTTON_HEIGHT = 80;
static const int SLIDER_HEIGHT = 80;
static const int SLIDER_MARKER_WIDTH = 40;
static const int CHECK_SIZE = 32;
static const int DEFAULT_SPACING = 16;
static const int TEXT_HEIGHT = 32;
static const int SCROLL_AREA_PADDING = 24;
static const int INDENT_SIZE = 64;
static const int AREA_HEADER = 112;
} // namespace ui
int stateSize = 0, inputSize = 0;
int settings = 0;
extern int caffe_color, no_shadow;
static std::vector<std::string> layer_names;
static std::vector<std::string> model_files;
static std::vector<std::string> model_names;
extern "C" void layer_name(const char *name) {
	if (name)
		layer_names.push_back(name);
	else
		layer_names.clear();
}
extern "C" void dirent_name(const char *d_name) {
	std::string n(d_name);
	int p = n.find("run_");
	if (p == 0) {
		model_files.push_back("data/args/" + n);
		n.replace(n.size() - 9, 9, "");
		n.replace(0, 15, "");
		model_names.push_back(n);
	}
}
int dirent_file(const struct dirent *d) {

	if ((d->d_type & DT_DIR) == 0) {
		dirent_name(d->d_name);
	}
	return 0;
}
extern "C" void use_arg(const char *path, int cmd);
extern "C" void box2d_ui(int width, int height, int mx, int my,
												 unsigned char mbut, int scroll) {

	if (mbut) {
		imguiBeginFrame(mx, height - my, mbut, scroll);
	} else {
		imguiBeginFrame(0, 0, 0, scroll);
	}
#ifdef USE_UI
	int size = (ui::AREA_HEADER + ui::BUTTON_HEIGHT + ui::SCROLL_AREA_PADDING);
	size += (ui::BUTTON_HEIGHT)*4;
	size += (ui::DEFAULT_SPACING)*4;
	if (state_show)
		size += ui::BUTTON_HEIGHT * 8 +
						ui::DEFAULT_SPACING * 7; // btMax(width, height) * 3 / 8;
	char title[64];
	sprintf(title, "DeepMimic FPS:%4.1f", gUpdatesPerSec);
	// sprintf(title, "%4.2f,%4.2f,%4.2f,%4.2f", gyro[0], gyro[1], gyro[2],
	// gyro[3]);
	over |= imguiBeginScrollArea(
			title, width - btMin(width, height) * 2 / 3 - (width > height ? 120 : 0),
			height - size - 60, btMin(width, height) * 2 / 3 - 10, size,
			&state_scroll);
	if (settings) {
		char info[64];
		if (imguiButton("Show Kin", true)) {
			Keyboard('k', 1, 1);
		}
		if (imguiButton("Change Style", true)) {
			caffe_color = !caffe_color;
			Keyboard('m', 1, 1);
		}
		if (imguiButton("Change Shsdow", true)) {
			no_shadow = (no_shadow+1)%3;
			Keyboard('m', 1, 1);
		}
		if (imguiButton("Back", true)) {
			settings = !settings;
		}
	} else {
		if (imguiButton("Reset", true)) {
			Keyboard('r', 1, 1);
		}
		if (imguiButton("Pause", true)) {
			running = !running;
		}
		if (imguiButton("Projectile", true)) {
			Keyboard('x', 1, 1);
		}
		if (imguiButton("Settings", true)) {
			settings = !settings;
		}

		if (layer_names.size() > 0) {
			for (int i = 0; i < layer_names.size(); ++i) {
				imguiItem(layer_names[i].c_str(), true);
			}
		}
	}
	// imguiLabel("Label");
	if (imguiCollapse("Items", 0, state_show, true)) {
		if (state_show) {
			save_scroll = state_scroll;
			state_scroll = 0;
		} else {
			state_scroll = save_scroll;
		}
		state_show = !state_show;
	}

	if (state_show) {
		if (model_files.size() == 0) {
			struct dirent **d;
			scandir("data/args", &d, dirent_file, 0);
			if (model_files.size() == 0) {
				struct dirent **d;
				scandir("apkoverlay/assets/data/args", &d, dirent_file, 0);
			}
		}

		for (int i = 0; i < model_files.size(); ++i) {
			if (imguiItem(model_names[i].c_str(), true)) {
				use_arg(model_files[i].c_str(), 1);
			}
		}
	}

	imguiEndScrollArea();
	imguiEndFrame();
#endif
	/*point = b2Vec2(mx, my);
	if (touch == 1) {
					if (!over) MouseDown(point);
	} else if (ctrl == 1) {
					MouseUp(point);
	} else {
		MouseMove(point);
	}*/
}