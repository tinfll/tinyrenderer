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
    TGAImage Albedo;
    PhongShader(const Model& m, Matrix4f M, Matrix4f modelM, Vec3f L, Vec3f cameraPos, TGAImage tex) : model(m), uniform_M(M), modelM(modelM), L(L), cameraPos(cameraPos), Albedo(tex){}
    
    mat<3, 3, float> PV;//worldPos
    mat<3, 3, float> NV;
    mat<2, 3, float> UV;

    virtual Vec4f vertex(const int iface, int nthvert) override{
        Model::VertexData vd = model.faces_[iface][nthvert];

        Vec4f v = (vd.id >= 0 && vd.id < model.verts_.size())
            ? model.verts_[vd.id]
            : Vec4f(0.0f, 0.0f, 0.0f, 1.0f);

        Vec4f vn = (vd.n >= 0 && vd.id < model.vertsN_.size())
            ? model.vertsN_[vd.n]
            : Vec4f(0.0f, 0.0f, 1.0f, 0.0f);

        Vec4f uv = (vd.t >= 0 && vd.t < model.vertsT_.size())
            ? model.vertsT_[vd.t]
            : Vec4f(0.0f, 0.0f, 0.0f, 0.0f);
        // no better solutions so.....

        PV[0][nthvert] = (modelM * v).x;
        PV[1][nthvert] = (modelM * v).y;
        PV[2][nthvert] = (modelM * v).z;


        NV[0][nthvert] = (modelM.invert_transpose() * Vec4f(vn.x, vn.y, vn.z, 0.0f)).x;
        NV[1][nthvert] = (modelM.invert_transpose() * Vec4f(vn.x, vn.y, vn.z, 0.0f)).y;
        NV[2][nthvert] = (modelM.invert_transpose() * Vec4f(vn.x, vn.y, vn.z, 0.0f)).z;
        
        UV[0][nthvert] = uv.x;
        UV[1][nthvert] = uv.y;

        Vec4f gl_Vertex = { v.x, v.y, v.z, 1.0f };
        Vec4f gl_Position = modelv * perspo * gl_Vertex;
        return uniform_M * gl_Vertex;

    }
    virtual std::pair<bool, TGAColor> fragment(const Vec3f bar) const{
        Vec3f P = Vec3f(PV * bar);
        Vec3f N = Vec3f(NV * bar).normalize();
        Vec2f uv = UV * bar;

        int tex_x = std::max(0, std::min(Albedo.width() - 1, static_cast<int>(uv.x * Albedo.width())));
        int tex_y = std::max(0, std::min(Albedo.height() - 1,
            static_cast<int>((1.0f - uv.y) * Albedo.height())));

        Vec3f V = ( P - cameraPos).normalize();
        float cosineA = dot(N, L);//diffuse
        float diffuse = cosineA;
        Vec3f R = 2 * N * cosineA - L;
        float specular = std::pow(std::max(0.0f, dot(R, V)), 32.0f);

        float ambient = 0.1f;
        float diffuseTerm = 0.6f;
        float i = ambient + diffuse * 0.6f + specular * 0.3f;
        TGAColor basecolor = Albedo.get(tex_x, tex_y);
        TGAColor specularColor = white;

        TGAColor finColor = {
            static_cast<unsigned char>(std::clamp(basecolor[0] * i, 0.0f, 255.0f)),
            static_cast<unsigned char>(std::clamp(basecolor[1] * i, 0.0f, 255.0f)),
            static_cast<unsigned char>(std::clamp(basecolor[2] * i, 0.0f, 255.0f)),
            255
        };

        return { false, finColor };
    }
};

int main(int argc, char** argv) {
    Model Aface("Adefaultface.obj");
    Model Abody("Adefaultbody.obj");

    tinfgl render;

    constexpr int width = 800;
    constexpr int height = 800;
     Vec3f eye = { -3, 2, 5 }; 
     Vec3f center = { 0, 1, 0 }; 
     Vec3f  up = { 0, 0.5, 0 }; 

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage z(width, height, TGAImage::GRAYSCALE);
    TGAImage face;
    TGAImage body;
    face.read_tga_file("Image_0.tga");
    body.read_tga_file("Image_3.tga");


    bool a = false, b = false;
    float scale = 400.0f;
   
    render.lookat(eye, center, up);
    float Md = 0, md = 0;
    render.init_perspective((eye-center).norm());
    render.init_viewport(width/16, height/16, width*7/8, height*7/8);
    render.initZ(width, height);

    Matrix4f M = viewp * perspo * modelv;

    //directional Light
    Vec3f L = { 1.0, 1.0, 1.0 };
    L = L.normalize();
    PhongShader phong(Abody, M, modelv, L, eye, body);
    PhongShader phong1(Aface, M, modelv, L, eye, face);

    //raster
    for (int i = 0; i < Abody.faces_.size(); ++i) {
        qmhsV<Model::VertexData> fs = Abody.faces_[i];
        Vec4f clip_coords[3];
        Vec4f clip_coordsn[3];
        for (int j = 0; j < 3; ++j) 
            clip_coords[j] = phong.vertex(i, j);
            
        render.rasterization(clip_coords[0], clip_coords[1], clip_coords[2],
                            image, z, width, height, phong);
       
    } 
    
    //raster
    for (int i = 0; i < Aface.faces_.size(); ++i) {
        qmhsV<Model::VertexData> fs = Aface.faces_[i];
        Vec4f clip_coords[3];
        Vec4f clip_coordsn[3];
        for (int j = 0; j < 3; j++)
            clip_coords[j] = phong1.vertex(i, j);
     
        render.rasterization(clip_coords[0], clip_coords[1], clip_coords[2],
            image, z, width, height, phong1);

    }


    image.write_tga_file("image.tga");
    system("start image.tga");

    return 0;
}