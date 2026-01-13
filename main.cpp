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
constexpr TGAColor white = { 255, 255, 255, 255 };
constexpr TGAColor Black = { 0, 0, 0, 255 };


int width = 4096;
int height = 4096;
float scale = 2048.0f;
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

float S(Vec3f A, Vec3f B, Vec3f C);
Vec3f bayZ(Vec3f A, Vec3f B, Vec3f C, Vec3f P);


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

Vec3f project(Vec3f v, float cy, float cx, float cz) {
    float x = (v.x - cx) * scale + width / 2.0f ;
    float y = (v.y + cy) * scale + height / 2.0f;
    float z = (v.z + cz) * scale;
	if (!a) { std::cout << x << "," << y << std::endl; a = true; }
    return { x , y , z};
}


float cross(Vec2f a, Vec2f b) {
    return a.x * b.y - a.y * b.x;
}

void rasterization(Vec3f a0, Vec3f a1, Vec3f a2, TGAColor ccol) {

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
            Vec3f p(i + 0.5f, j + 0.5f, 0);
			Vec3f a = bayZ(a0, a1, a2, p);
            if (a.x < -0.01|| a.y < -0.01 || a.z < -0.01) continue;
			float zb = (a0.z * a.x + a1.z * a.y + a2.z * a.z);
			int idx = i + j * width;
                if (zb > zbuffer[idx]){
					zbuffer[idx] = zb;
                    image.set(p.x, p.y, ccol);
            }
        }
    }
}


float S(Vec3f A, Vec3f B, Vec3f C) {
    Vec2f AB = { B.x - A.x, B.y - A.y };
    Vec2f AC = { C.x - A.x, C.y - A.y };
    return cross(AB, AC);
}

Vec3f bayZ(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec2f AB = { B.x - A.x, B.y - A.y };
    Vec2f BC = { C.x - B.x, C.y - B.y };
    Vec2f CA = { A.x - C.x, A.y - C.y };
    Vec2f AC = { C.x - A.x, C.y - A.y };
    float S = cross(AB, AC);

    Vec2f AP = { P.x - A.x, P.y - A.y };
    Vec2f BP = { P.x - B.x, P.y - B.y };
    Vec2f CP = { P.x - C.x, P.y - C.y };

    float S1 = cross(AB, AP);
    float S2 = cross(BC, BP);
    float S3 = cross(CA, CP);

    float w = S1 / S; // C
    float u = S2 / S; // A
    float v = S3 / S;

    return Vec3f(u, v, w);
}


int main(int argc, char** argv) {
    Model qmhs("C:/Users/tinf/Documents/tinyRenderer/build/obj/qmhs/qmhs.obj");
    float Md = 0, md = 0;
    for (int i = 0; i < width * height; i++)
        zbuffer[i] = -std::numeric_limits<float>::max();

    //project vertices
    for (auto& verts_ : qmhs.verts_) {
        if (verts_.z < md) md = verts_.z;
        if (verts_.z > Md) Md = verts_.z;
        verts_ = project(verts_, qmhs.cy, qmhs.cx, qmhs.cz);
    }
	std::cout << "Z range: " << md << " to " << Md << std::endl;

	//wirefreame rendering
    for (auto& face : qmhs.faces_) {
		std::vector<int> fs = face;
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
        std::vector<int> fs = face;
        if (fs.size() == 3)
            rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[1]], qmhs.verts_[fs[2]], rnd);
        if (fs.size() == 4) {
            rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[1]], qmhs.verts_[fs[2]], rnd);
            rasterization(qmhs.verts_[fs[0]], qmhs.verts_[fs[2]], qmhs.verts_[fs[3]],rnd);
        }
   }
    
    image.rotate90();
    image.flip_vertically();
	image.flip_horizontally();
    image.write_tga_file("image.tga");

    
    float minz = 0, maxz = 0;

    float clear_value = -std::numeric_limits<float>::max();

    for (int i = 0; i < width * height; i++) {
        float v = zbuffer[i];
        if (std::abs(v) < 10000) {
            if (v < minz) minz = v;
            if (v > maxz) maxz = v;
        }
    }

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            float v = zbuffer[x + y * width];
            if (std::abs(v) >= 10000) 
                z.set(x, y, { 0, 0, 0, 255 });
    
                float normalized = (v - minz) / (maxz - minz);
                normalized = std::max(0.0f, std::min(1.0f, normalized));

                unsigned char gray = static_cast<unsigned char>(255.0f * normalized);
                z.set(x, y, { gray });
            }
        }

    std::cout << "Detected Z range for visualization: [" << minz << ", " << maxz << "]" << std::endl;


    z.rotate90();
    z.flip_vertically();
    z.flip_horizontally();
	z.write_tga_file("z.tga");
    std::cout << "Render finished! output.tga saved." << std::endl;
    system("start z.tga");
    system("start image.tga");
    return 0;
}