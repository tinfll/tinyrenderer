#pragma once

#include <vector>
#include <string>
#include <geometry.h>

struct Vec3f {
	float x, y, z;
};

struct Vec2f {
	float x, y;
};

class Model {

public:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int>> faces_;
	float min_x = 1e9, max_x = -1e9;
	float min_y = 1e9, max_y = -1e9;
	float min_z = 1e9, max_z = -1e9;
	float cx = 0, cy = 0, cz = 0;
	float mw = 1, mh = 1;

	Model(const char* filename);
	
	Vec2f viewt(int i, int screen_w, int screen_h, float scale, float rotation_deg);
};


