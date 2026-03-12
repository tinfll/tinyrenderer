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

extern qmhsV<float> zbuffer;

constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor white = { 255, 255, 255, 255 };
constexpr TGAColor Black = { 0, 0, 0, 255 };
const float MY_PI = 3.1415926535;
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
struct RandomShader : public IShader {
    const Model& model;
    Matrix4f uniform_M;
    TGAColor color = {};
    RandomShader(const Model& m, Matrix4f M) : model(m), uniform_M(M)  {}

    virtual Vec4f vertex(const int iface, int nthvert)  {
        Vec4f v = model.verts_[model.faces_[iface][nthvert]];
        Vec4f gl_Vertex = { v.x, v.y, v.z, 1.0f };
        Vec4f gl_Position = modelv * perspo * gl_Vertex;

        return uniform_M * gl_Vertex;
    }
    
    virtual std::pair<bool, TGAColor> fragment(const Vec3f bar) const {
        TGAColor c = qcolor();
        return { false, c };
    }

};





/*void line(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
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
}*/




int main(int argc, char** argv) {
    Model qmhs("unity.obj");
    tinfgl render;

    constexpr int width = 800;
    constexpr int height = 800;
     Vec3f eye = { -1, 0, 2 }; 
    Vec3f center = { 0, 0, 0 }; 
     Vec3f  up = { 0, 1, 0 }; 

    TGAImage image(width, height, TGAImage::RGB);
    bool a = false, b = false;
    TGAImage z(width, height, TGAImage::GRAYSCALE);

    float scale = 400.0f;
   

    render.lookat(eye, center, up);
    float Md = 0, md = 0;
    render.init_perspective(7);
    render.init_viewport(width, height, width, height);
    render.initZ(width, height);

    Matrix4f M = viewp * perspo * modelv;

    RandomShader shader(qmhs, M);

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
            render.rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[1]], qmhs.verts_[fs[2]], image, z, width, height, shader);
        if (fs.size() == 4) {
            render.rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[1]], qmhs.verts_[fs[2]], image, z, width, height, shader);
            render.rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[2]], qmhs.verts_[fs[3]], image, z, width, height, shader);
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