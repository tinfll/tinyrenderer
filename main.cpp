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
#include "our_gl.h"
#include "geometry.h"
#include "myVector.h"


constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor white = { 255, 255, 255, 255 };
constexpr TGAColor Black = { 0, 0, 0, 255 };
const float MY_PI = 3.1415926535;


int width = 800;
int height = 800;
float scale = 400.f;
TGAImage image(width, height, TGAImage::RGB);
bool a = false, b = false;
TGAImage z(width, height, TGAImage::GRAYSCALE);
float* zbuffer = new float[width * height];


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

float S(Vec4f A, Vec4f B, Vec4f C);
Vec4f bayZ(Vec4f A, Vec4f B, Vec4f C, Vec4f P);


void line(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
    bool steep = std::abs(ax - bx) < std::abs(ay - by);
    if (steep) {
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
        if (steep)    framebuffer.set(y, x, color);
        else    framebuffer.set(x, y, color);
        ie += 2 * std::abs(by - ay);
        if (ie > bx - ax) {
            y += (by > ay) ? 1 : -1;
            ie -= 2 * (bx - ax);
        }
    }
}

Vec4f project(Vec4f v, float cy, float cx, float cz, float dy, float dx, float dz) {
    Matrix4f T = { 1, 0, 0, -v.x,
                      0, 1, 0, -v.y,
                      0, 0, 1, -v.z,
                      0, 0, 0, 1};
    //暂无matrix M视角问题
    Matrix4f O = { 1 / dx, 0, 0, -cx,
                   0, 1 / dy , 0, -cy,
                   0, 0, 1 / dz, -cz,
                   0, 0, 0, 1 };

    Matrix4f O2 = { 1, 0, 0, -cx,
                   0, 1 , 0, -cy,
                   0, 0, 1, -cz,
                   0, 0, 1, 1 };

    Matrix4f vp = { width / 2.0f, 0, 0, 0,
        0, height / 2.0f, 0,  0,
        0, 0, scale, 0,
        0, 0, 0, 1 };
    float f = 3.0f;
    float c = 1 / (1 - v.z / f);
    Matrix4f perspo = { 1, 0 ,0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, c, 1 };

    Vec4f n = perspo * vp * O * T *  v;
    if (!a) { std::cout << n.x << "," << n.y << std::endl; a = true; }
    return n;
}



void rasterization(Vec4f a0, Vec4f a1, Vec4f a2, TGAColor ccol) {

    TGAColor rnd;
    //std::cout << a0.x << "," << a0.y << " " << a1.x << "," << a1.y << " " << a2.x << "," << a2.y << std::endl;
    int minx_f = std::floor(std::min({ a0.x, a1.x, a2.x }));
    int maxx_f = std::ceil(std::max({ a0.x, a1.x, a2.x }));
    int miny_f = std::floor(std::min({ a0.y, a1.y, a2.y }));
    int maxy_f = std::ceil(std::max({ a0.y, a1.y, a2.y }));

    int minx = std::max(0, minx_f);
    int maxx = std::min(width - 1, maxx_f);
    int miny = std::max(0, miny_f);
    int maxy = std::min(height - 1, maxy_f);


    float s = S(a0, a1, a2);
    if (s < 0) return;

    for (int i = minx; i < maxx; i++) {
        for (int j = miny; j < maxy; j++) {
            Vec4f p(i + 0.5f, j + 0.5f, 0, 1);
            Vec4f a = bayZ(a0, a1, a2, p);
            if (a.x < -0.01 || a.y < -0.01 || a.z < -0.01) continue;
            float zb = (a0.z * a.x + a1.z * a.y + a2.z * a.z);
            int idx = i + j * width;
            if (zb >= zbuffer[idx]) {
                zbuffer[idx] = zb;
                image.set(p.x, p.y, ccol);
            }
        }
    }
}


float S(Vec4f A, Vec4f B, Vec4f C) {
    Vec2f AB(B.x - A.x, B.y - A.y);
    Vec2f AC(C.x - A.x, C.y - A.y);
    return cross(AB, AC);
}

Vec4f bayZ(Vec4f A, Vec4f B, Vec4f C, Vec4f P) {
    Vec2f AB(B.x - A.x, B.y - A.y);
    Vec2f BC(C.x - B.x, C.y - B.y);
    Vec2f CA(A.x - C.x, A.y - C.y);
    Vec2f AC(C.x - A.x, C.y - A.y);
    float S = cross(AB, AC);

    Vec2f AP(P.x - A.x, P.y - A.y);
    Vec2f BP(P.x - B.x, P.y - B.y);
    Vec2f CP(P.x - C.x, P.y - C.y);

    float S1 = cross(AB, AP);
    float S2 = cross(BC, BP);
    float S3 = cross(CA, CP);

    float w = S1 / S; // C
    float u = S2 / S; // A
    float v = S3 / S;

    return Vec4f(u, v, w, 1);
}


int main(int argc, char** argv) {
    Model qmhs("unity.obj");
    //Model qmhs("../obj/qmhs/qmhs.obj");

    float Md = 0, md = 0;
    for (int i = 0; i < width * height; i++)
        zbuffer[i] = -std::numeric_limits<float>::max();

    //project vertices
    for (auto& verts_ : qmhs.verts_) {
        
        verts_ = project(verts_, qmhs.cy, qmhs.cx, qmhs.cz, qmhs.dy, qmhs.dx, qmhs.cz);
        verts_.x = verts_.x / verts_.w;
        verts_.y = verts_.y / verts_.w;
        verts_.z = verts_.z / verts_.w;
    }   

    //wirefreame rendering
    for (auto& face : qmhs.faces_) {
        qmhsV<int> fs = face;
        for (int j = 0; j < fs.size(); j++) {
            int f = fs[j], k = fs[(j + 1) % fs.size()];
            line(qmhs.verts_[f].x, qmhs.verts_[f].y,
                qmhs.verts_[k].x, qmhs.verts_[k].y,
                image, blue);
        }
    }
    //raster
    for (auto& face : qmhs.faces_) {
        TGAColor rnd = qcolor();
        qmhsV<int> fs = face;
        if (fs.size() == 3)
            rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[1]], qmhs.verts_[fs[2]], rnd);
        if (fs.size() == 4) {
            rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[1]], qmhs.verts_[fs[2]], rnd);
            rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[2]], qmhs.verts_[fs[3]], rnd);
        }
    }


    image.write_tga_file("image.tga");
    system("start image.tga");

    float minz = std::numeric_limits<float>::max(), maxz = -std::numeric_limits<float>::max();


    for (int i = 0; i < width * height; i++) {
        float v = zbuffer[i];
        if (std::abs(v) < 1e5) {
            if (v < minz) minz = v;
            if (v > maxz) maxz = v;
        }
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            float v = zbuffer[x + y * width];
            if (std::abs(v) >= 1)
                z.set(x, y, { 0, 0, 0, 255 });

            float normalized = (v - minz) / (maxz - minz);
            normalized = std::max(0.0f, std::min(1.0f, normalized));

            unsigned char gray = static_cast<unsigned char>(255.0f * normalized);
            z.set(x, y, { gray });
        }
    }

    std::cout << "Detected Z range for visualization: [" << minz << ", " << maxz << "]" << std::endl;


    z.write_tga_file("z.tga");
    std::cout << "Render finished! output.tga saved." << std::endl;
    system("start z.tga");


    return 0;
}