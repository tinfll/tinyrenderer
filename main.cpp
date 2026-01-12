#include <cmath>
#include<iostream>
#include<fstream>
#include<sstream>
#include <ctime>
#include <cstdlib>
#include <chrono> // 用于精确计时
#include "tgaimage.h"
#include "model.h"

struct Vec3f {
    double x, y, z;
};

struct Vec2f {
    double x, y;
};

std::vector<Vec3f> verts_;
std::vector<std::vector<int>> faces_;

void load(const char* filename);

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
    float y = ay;
    int ie = 0;
    for (int x = ax; x <= bx; x++) {
	    if(steep)
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ie += 2 * std::abs(by - ay);
        y += (by - ay) / static_cast<float>(bx - ax);
        ie -= 2 * (bx - ax) * (ie > bx - ax);
    }
}


void load(const char* filename) {
	std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) {
        std::cout << "File not found!" << std::endl;
        return;
    }
    else std::cout << "success" << std::endl;
    
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::stringstream ss(line);
        char trash;
        std::string a;
        
        while (ss >> a) {
            if (a == "v") {
                ss >> trash;
                Vec3f v;
                ss >> v.x >> v.y >> v.z;
                verts_.push_back(v);
                std::cout << "v: " << v.x << " " << v.y << " " << v.z << std::endl;
            }
            else if (a == 'f') {
                ss >> trash;
                std::vector<int> fs;
                int f;
                ss >> f;
                fs.push_back(f);
                faces_.push_back(fs);
                std::cout << "f: " << f << std::endl;
            }
            else {}
        }
    }
     
}


int main(int argc, char** argv) {
    load("C:/Users/tinf/Documents/tinyRenderer/build/obj/qmhs/qmhs.obj");

    int width = 800;
    int height = 800;
    TGAImage image(width, height, TGAImage::RGB);



    return 0;
}