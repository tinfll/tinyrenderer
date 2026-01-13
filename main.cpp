#include <cmath>
#include<iostream>
#include<fstream>
#include<sstream>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"

constexpr TGAColor blue = { 255, 128,  64, 255 };

int width = 800;
int height = 800;
float scale = 400.0f;
TGAImage image(width, height, TGAImage::RGB);
bool a = false, b = false;


TGAColor qcolor() {
    int base = 180;
    int b = (base + 40) + (std::rand() % 36); if (b > 255) b = 255;
    int g = (base + 20) + (std::rand() % 36); if (g > 255) g = 255;
    int r = base + (std::rand() % 36);        if (r > 255) r = 255;

    TGAColor color;
    color.bgra[0] = b;
    color.bgra[1] = g;
    color.bgra[2] = r;
    color.bgra[3] = 255;
    color.bytespp = 4; 

    return color;
}

bool is_inside(Vec2f A, Vec2f B, Vec2f C, Vec2f P);
float S(Vec2f A, Vec2f B, Vec2f C);
Vec3f bayZ(Vec2f A, Vec2f B, Vec2f C, Vec2f P);


void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax - bx) < std::abs(ay - by);
    if(steep) {
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax > bx) {
        std::swap(ax, bx);
		std::swap(ay, by);
    }
    int y = ay;
    int ie = 0;
    for (int x = ax; x <= bx; x++) {
	    if(steep)    framebuffer.set(y, x, color);
        else    framebuffer.set(x, y, color);
        ie += 2 * std::abs(by - ay);
        if (ie > bx - ax) {
            y += (by > ay) ? 1 : -1;
            ie -= 2 * (bx - ax);
        }
    }
}

Vec2f project(Vec3f v, float cy, float cx) {
    float x = (v.x - cx) * scale + width / 2.0f ;
    float y = (v.y + cy) * scale + height / 2.0f;
	if (!a) { std::cout << x << "," << y << std::endl; a = true; }
    return { x , y };
}

float viewtw(float a, float cx) {
	float x = (a - cx) * scale + width / 2.0f;
    return x;
}
float viewth(float a, float cy) {
    float y = (a + cy) * scale + width / 2.0f;
    if (!b) { std::cout << y << std::endl;    b = true; }
    return y;
}

float cross_product(Vec2f a, Vec2f b) {
    return a.x * b.y - a.y * b.x;
}

void rasterization(Vec2f a0, Vec2f a1, Vec2f a2, TGAColor ccol) {

    TGAColor rnd;
	//std::cout << a0.x << "," << a0.y << " " << a1.x << "," << a1.y << " " << a2.x << "," << a2.y << std::endl;
    int minx = std::min({ a0.x, a1.x, a2.x });
    int maxx = std::max({ a0.x, a1.x, a2.x });
    int miny = std::min({ a0.y, a1.y, a2.y });
    int maxy = std::max({ a0.y, a1.y, a2.y });

    float s = S(a0, a1, a2);
    if (s < 0) return;

    for (int i = minx; i < maxx; i++) {
        for (int j = miny; j < maxy; j++) {

            Vec2f p(i + 0.5f, j + 0.5f);
			Vec3f a = bayZ(a0, a1, a2, p);
			if (a.x < 0 || a.y < 0 || a.z < 0) continue;
            if ((a.x > 0 && a.y > 0 && a.z > 0) || (a.x < 0 && a.y < 0 && a.z < 0))
                image.set(p.x, p.y, ccol);
        }

    }
}


float S(Vec2f A, Vec2f B, Vec2f C) {
    Vec2f AB = { B.x - A.x, B.y - A.y };
    Vec2f AC = { C.x - A.x, C.y - A.y };
    return cross_product(AB, AC);
}

Vec3f bayZ(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec2f AB = { B.x - A.x, B.y - A.y };
    Vec2f BC = { C.x - B.x, C.y - B.y };
    Vec2f CA = { A.x - C.x, A.y - C.y };
    Vec2f AC = { C.x - A.x, C.y - A.y };
    float S = cross_product(AB, AC);

    Vec2f AP = { P.x - A.x, P.y - A.y };
    Vec2f BP = { P.x - B.x, P.y - B.y };
    Vec2f CP = { P.x - C.x, P.y - C.y };

    float c1 = cross_product(AB, AP);
    float c2 = cross_product(BC, BP);
    float c3 = cross_product(CA, CP);

    float w = c1 / S; // Weight for C
    float u = c2 / S; // Weight for A
    float v = c3 / S;
    //(c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0)
    return Vec3f(w, u, v);
}


int main(int argc, char** argv) {
    Model qmhs("C:/Users/tinf/Documents/tinyRenderer/build/obj/qmhs/qmhs.obj");
  
	//wirefreame rendering
for (auto& face : qmhs.faces_) {
		std::vector<int> fs = face;
        for (int j = 0; j < fs.size(); j++) {
            int f = fs[j], k = fs[(j + 1) % fs.size()];
            line(viewtw(qmhs.verts_[f].x, qmhs.cx), viewth(qmhs.verts_[f].y, qmhs.cy),
                    viewtw(qmhs.verts_[k].x, qmhs.cx), viewth(qmhs.verts_[k].y, qmhs.cy),
                    image, blue);
        }
	}
for (auto& face : qmhs.faces_) {
        TGAColor rnd = qcolor();
        std::vector<int> fs = face;
        if (fs.size() == 3)
            rasterization(project(qmhs.verts_[fs[0]], qmhs.cy, qmhs.cx), 
                          project(qmhs.verts_[fs[1]], qmhs.cy, qmhs.cx), 
                          project(qmhs.verts_[fs[2]], qmhs.cy, qmhs.cx), rnd);
        if (fs.size() == 4) {
            rasterization(project(qmhs.verts_[fs[0]], qmhs.cy, qmhs.cx), 
                project(qmhs.verts_[fs[1]],qmhs.cy, qmhs.cx), 
                project(qmhs.verts_[fs[2]],qmhs.cy, qmhs.cx), rnd);
            rasterization(project(qmhs.verts_[fs[0]], qmhs.cy, qmhs.cx), 
                project(qmhs.verts_[fs[2]], qmhs.cy, qmhs.cx), 
                project( qmhs.verts_[fs[3]], qmhs.cy, qmhs.cx),rnd);
        }
   }
    
    image.rotate90();
    image.flip_vertically();
	image.flip_horizontally();
    image.write_tga_file("image.tga");
    std::cout << "Render finished! output.tga saved." << std::endl;
    system("start image.tga");
    return 0;
}