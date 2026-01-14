#pragma once

#include <myVector.h>
#include <string>
#include <geometry.h>



class Model {

public:
	qmhsV<Vec4f> verts_;
	qmhsV<qmhsV<int>> faces_;
	float min_x = 1e9, max_x = -1e9;
	float min_y = 1e9, max_y = -1e9;
	float min_z = 1e9, max_z = -1e9;
	float cx = 0, cy = 0, cz = 0, dx = 0, dy = 0, dz = 0;
	float mw = 1, mh = 1;

	Model(const char* filename);
	
	Vec2f viewt(int i, int screen_w, int screen_h, float scale, float rotation_deg);
};


