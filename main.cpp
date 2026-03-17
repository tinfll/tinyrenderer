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



struct PhongShader : IShader {
    const Model& model;
    Matrix4f uniform_M;
    Matrix4f modelM;
    Vec3f L;
    Vec3f cameraPos;
    //Vec3f N;
    PhongShader(const Model& m, Matrix4f M, Matrix4f modelM, Vec3f L, Vec3f cameraPos) : model(m), uniform_M(M), modelM(modelM), L(L), cameraPos(cameraPos){}
    
    mat<3, 3, float> PV;//worldPos
    mat<3, 3, float> NV;

    virtual Vec4f vertex(const int iface, int nthvert) override{
        Vec4f v = model.verts_[model.faces_[iface][nthvert].id];
        Vec4f vn = model.vertsN_[model.faces_[iface][nthvert].n];
        PV[0][nthvert] = (modelM * v).x;
        PV[1][nthvert] = (modelM * v).y;
        PV[2][nthvert] = (modelM * v).z;


        NV[0][nthvert] = (modelM * Vec4f(vn.x, vn.y, vn.z, 0.0f)).x;
        NV[1][nthvert] = (modelM * Vec4f(vn.x, vn.y, vn.z, 0.0f)).y;
        NV[2][nthvert] = (modelM * Vec4f(vn.x, vn.y, vn.z, 0.0f)).z;
        
        Vec4f gl_Vertex = { v.x, v.y, v.z, 1.0f };
        Vec4f gl_Position = modelv * perspo * gl_Vertex;
        return uniform_M * gl_Vertex;

    }
    virtual std::pair<bool, TGAColor> fragment(const Vec3f bar) const{
        Vec3f P = Vec3f(PV * bar);
        Vec3f N = Vec3f(NV * bar).normalize();
        Vec3f V = (P - cameraPos).normalize();
        float cosineA = dot(N, L);//diffuse
        float diffuse = cosineA;
        Vec3f R = 2 * N * cosineA - L;
        //float cosineB = dot(R, V) > 0 ? 0 : cosineB;
        float specular = std::pow(std::max(0.0f, dot(R, V)), 32.0f);

        float ambient = 0.1f;
        float i = ambient + diffuse * 0.6f + specular * 0.3f;
        TGAColor basecolor = qcolor();
        TGAColor specularColor = white;

        TGAColor finColor = {
            static_cast<unsigned char>(std::clamp(255.0f * i, 0.0f, 255.0f)),
            static_cast<unsigned char>(std::clamp(255.0f * i, 0.0f, 255.0f)),
            static_cast<unsigned char>(std::clamp(255.0f * i, 0.0f, 255.0f)),
            255
        };

        return { false, finColor };
    }
};

int main(int argc, char** argv) {
    Model qmhs("Adefault.obj");
    tinfgl render;

    constexpr int width = 800;
    constexpr int height = 800;
     Vec3f eye = { -3, 2, 5 }; 
     Vec3f center = { 0, 1, 0 }; 
     Vec3f  up = { 0, 0.5, 0 }; 

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage z(width, height, TGAImage::GRAYSCALE);
    bool a = false, b = false;
    float scale = 400.0f;
   
    render.lookat(eye, center, up);
    float Md = 0, md = 0;
    render.init_perspective((eye-center).norm());
    render.init_viewport(width/16, height/16, width*7/8, height*7/8);
    render.initZ(width, height);

    Matrix4f M = viewp * perspo * modelv;

    //RandomShader shader(qmhs, M);
    //directional Light
    Vec3f L = { 1.0, 1.0, 1.0 };
    L = L.normalize();
    PhongShader phong(qmhs, M, modelv, L, eye);


    //wirefreame rendering
    for (auto& face : qmhs.faces_) {
        qmhsV<Model::VertexData> fs = face;
        for (int j = 0; j < fs.size(); j++) {
            int f = fs[j].id, k = fs[(j + 1) % fs.size()].id;
            line(qmhs.verts_[f].x, qmhs.verts_[f].y,
                qmhs.verts_[k].x, qmhs.verts_[k].y,
                image, blue);
        }
    }
    //raster
    for (int i = 0; i < qmhs.faces_.size(); ++i) {
        TGAColor rnd = qcolor();
        qmhsV<Model::VertexData> fs = qmhs.faces_[i];
        Vec4f clip_coords[3];
        Vec4f clip_coordsn[3];
        for (int j = 0; j < 3; j++) 
            clip_coords[j] = phong.vertex(i, j);
            
        render.rasterization(clip_coords[0], clip_coords[1], clip_coords[2],
                            image, z, width, height, phong);
       
    }


    image.write_tga_file("image.tga");
    system("start image.tga");

    return 0;
}