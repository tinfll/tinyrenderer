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

bool is_inside(Vec2f A, Vec2f B, Vec2f C, Vec2f P);

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
    return { std::round((v.x + cx) * scale + width / 2) , std::round((v.y + cy) * scale + height / 2)};
}

float viewtw(float a, float cx) {
    return (a + cx) * scale + width / 2.0f;
}
float viewth(float a, float cy) {
    return (a + cy) * scale + height / 2.0f;
}

float cross_product(Vec2f a, Vec2f b) {
    return a.x * b.y - a.y * b.x;
}

void rasterization(Vec2f a0, Vec2f a1, Vec2f a2) {
    int minx = std::min({ a0.x, a1.x, a2.x });
    int maxx = std::max({ a0.x, a1.x, a2.x });
    int miny = std::min({a0.y, a1.y, a2.y});
    int maxy = std::max({ a0.y, a1.y, a2.y });

    for (int i = minx; i < maxx; i++) {
        for (int j = miny; j < maxy; j++) {
            Vec2f p(i, j);
            if (is_inside(a0, a1, a2, p))
                image.set(p.y, -p.x, blue);

        }

    }
}

bool is_inside(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec2f AB = { B.x - A.x, B.y - A.y };
    Vec2f BC = { C.x - B.x, C.y - B.y };
    Vec2f CA = { A.x - C.x, A.y - C.y };

    Vec2f AP = { P.x - A.x, P.y - A.y };
    Vec2f BP = { P.x - B.x, P.y - B.y };
    Vec2f CP = { P.x - C.x, P.y - C.y };

    float c1 = cross_product(AB, AP);
    float c2 = cross_product(BC, BP);
    float c3 = cross_product(CA, CP);

    return (c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0);
}


int main(int argc, char** argv) {
    Model qmhs("C:/Users/tinf/Documents/tinyRenderer/build/obj/qmhs/qmhs.obj");
  
	//wirefreame rendering
  /*  for (auto& face : qmhs.faces_) {
		std::vector<int> fs = face;
        for (int j = 0; j < fs.size(); j++) {
            int f = fs[j], k = (j + 1) % fs.size();
            line(viewtw(qmhs.verts_[f].y, qmhs.cy), viewth(-qmhs.verts_[f].x, qmhs.cx),
                    viewtw(qmhs.verts_[fs[k]].y, qmhs.cy), viewth(-qmhs.verts_[fs[k]].x, qmhs.cx),
                    image, blue);
        }
	}*/
    for (auto& face : qmhs.faces_) {
        std::vector<int> fs = face;
        if (fs.size() == 3)
            rasterization(project(qmhs.verts_[fs[0]], qmhs.cy, qmhs.cx), project(qmhs.verts_[fs[1]], qmhs.cy, qmhs.cx), project(qmhs.verts_[fs[2]], qmhs.cy, qmhs.cx));
        if (fs.size() == 4) {
            rasterization(project(qmhs.verts_[fs[0]], qmhs.cy, qmhs.cx), project(qmhs.verts_[fs[1]],qmhs.cy, qmhs.cx), project(qmhs.verts_[fs[2]],qmhs.cy, qmhs.cx));
            rasterization(project(qmhs.verts_[fs[0]], qmhs.cy, qmhs.cx), project(qmhs.verts_[fs[2]], qmhs.cy, qmhs.cx), project( qmhs.verts_[fs[3]], qmhs.cy, qmhs.cx));
        }
        }

    image.write_tga_file("image.tga");
    std::cout << "Render finished! output.tga saved." << std::endl;
    return 0;
}