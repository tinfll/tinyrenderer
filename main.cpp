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
TGAImage image(width, height, TGAImage::RGB);

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



float viewtw(float a, float cx) {
    float scale = 400.0f;
    return a * scale + width / 2.0f;
}
float viewth(float a, float cy) {
    float scale = 400.0f;
    return (a + cy) * scale + height / 2.0f;
}

int main(int argc, char** argv) {
    Model qmhs("C:/Users/tinf/Documents/tinyRenderer/build/obj/qmhs/qmhs.obj");

    float rotation_angle = 90;

    float scale = std::min(width / qmhs.mw, height / qmhs.mh) * 0.9f;
  
    

    for(int i = 0; i < qmhs.faces_.size(); i++) {
		std::vector<int> fs = qmhs.faces_[i];
        for (int j = 0; j < fs.size(); j++) {
            int f = fs[j], k = (j + 1) % fs.size();
            line(viewtw(qmhs.verts_[f].y, qmhs.cy), viewth(-qmhs.verts_[f].x, qmhs.cx),
                    viewtw(qmhs.verts_[fs[k]].y, qmhs.cy), viewth(-qmhs.verts_[fs[k]].x, qmhs.cx),
                    image, blue);
        }
	}

    image.write_tga_file("image.tga");
    std::cout << "Render finished! output.tga saved." << std::endl;
    return 0;
}