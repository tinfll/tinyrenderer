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
    Matrix4f viewp;
    Vec3f L;
    Vec3f cameraPos;
    TGAImage Albedo;
    TGAImage NM;
    float width;
    float height;
    qmhsV<float> zL;

    Matrix4f modelL; //shadowmap

    PhongShader(const Model& m, Matrix4f M, Matrix4f modelM, Matrix4f modelL, Matrix4f viewp, 
                Vec3f L, Vec3f cameraPos, TGAImage tex, TGAImage NM, qmhsV<float> zL, 
                float width, float height) : model(m), uniform_M(M), modelM(modelM), viewp(viewp), modelL(modelL), L(L), cameraPos(cameraPos), Albedo(tex), NM(NM), zL(zL), width(width), height(height){}
    
    mat<3, 3, float> PV;//worldPos
    mat<3, 3, float> PL;
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

        PL[0][nthvert] = (modelL * v).x / (modelL * v).w;
        PL[1][nthvert] = (modelL * v).y / (modelL * v).w;
        PL[2][nthvert] = (modelL * v).z / (modelL * v).w;

        //LV[3 * iface + nthvert - 3] = (Lmodelv * v).z;
        //what are you doing?

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
        mat<3, 2, float> e;
        mat<2, 2, float> u;
        mat<3, 2, float> tb;
        mat<3, 3, float> TBN;
        Vec3f P1 = Vec3f(PV[1][0] - PV[0][0], PV[1][1] - PV[0][1], PV[1][2] - PV[0][2]);
        Vec3f P2 = Vec3f(PV[2][0] - PV[0][0], PV[2][1] - PV[0][1], PV[2][2] - PV[0][2]);
        e = { P1.x, P2.x, 
              P1.y, P2.y, 
              P1.z, P2.z };

        Vec2f U1 = Vec2f(UV[0][1] - UV[0][0], UV[1][1] - UV[1][0]);
        Vec2f U2 = Vec2f(UV[0][2] - UV[0][0], UV[1][2] - UV[1][0]);
        u = { U1.x, U2.x, 
              U1.y, U2.y };

        // ....manually invert the 2x2 UV matrix
        float det = U1.x * U2.y - U2.x * U1.y;
        mat<2, 2, float> u_inv;
        u_inv[0][0] = U2.y / det;
        u_inv[0][1] = -U2.x / det;
        u_inv[1][0] = -U1.y / det;
        u_inv[1][1] = U1.x / det;

        tb = e * u_inv;//through some linear algebra transformation

        Vec3f P = Vec3f(PV * bar);
        Vec3f pL = Vec3f(PL * bar);
        
        float shadow = 1.0f;
        int minx = std::floor(pL.x);
        int maxx = std::ceil(pL.x);
        int miny = std::floor(pL.y);
        int maxy = std::ceil(pL.y);
        
        int sx = std::max(0, std::min(static_cast<int>(pL.x), static_cast<int>(width)));
        int sy = std::max(0, std::min(static_cast<int>(pL.y), static_cast<int>(height)));
        float z_stored = zL[sx + sy * width];
        if (pL.z < z_stored - 0.02f)  
                shadow = 0.3f;

        Vec3f N = Vec3f(NV * bar).normalize();
        Vec2f uv = UV * bar;

        //sample the normal
        int NM_x = std::max(0, std::min(NM.width() - 1, static_cast<int>(uv.x * NM.width())));
        int NM_y = std::max(0, std::min(NM.height() - 1, static_cast<int>((1.0f - uv.y) * NM.height())));
        Vec3f n = { NM.get(NM_x, NM_y).bgra[2] / 255.f * 2.f - 1.f, 
                    NM.get(NM_x, NM_y).bgra[1] / 255.f * 2.f - 1.f, 
                    NM.get(NM_x, NM_y).bgra[0] / 255.f * 2.f - 1.f };


        Vec3f T = Vec3f(tb[0][0], tb[1][0], tb[2][0]).normalize();
        Vec3f B = Vec3f(tb[0][1], tb[1][1], tb[2][1]).normalize();
        TBN = { T.x, B.x, N.x,
                T.y, B.y, N.y,
                T.z, B.z, N.z };

        N = (TBN * n).normalize();

        //sample the albedo
        int tex_x = std::max(0, std::min(Albedo.width() - 1, static_cast<int>(uv.x * Albedo.width())));
        int tex_y = std::max(0, std::min(Albedo.height() - 1, static_cast<int>((1.0f - uv.y) * Albedo.height())));
        //vroid hub textures uv.y / obj

        Vec3f V = ( P - cameraPos).normalize();
        float cosineA = dot(N, L);//diffuse
        float diffuse = cosineA;
        Vec3f R = 2 * N * cosineA - L;
        float specular = std::pow(std::max(0.0f, dot(R, V)), 32.0f);

        float ambient = 0.1f;
        float diffuseTerm = 0.6f;
        float i = (ambient + diffuse * 0.6f + specular * 0.3f) * shadow;
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

struct shadowmap : IShader {
    const Model& model;
    Matrix4f uniform_LM;
    shadowmap(const Model& m, Matrix4f M) : model(m), uniform_LM(M){}

    virtual Vec4f vertex(const int iface, int nthvert) override {
        Model::VertexData vd = model.faces_[iface][nthvert];
        Vec4f v = (vd.id >= 0 && vd.id < model.verts_.size())
            ? model.verts_[vd.id]
            : Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
        Vec4f gl_Vertex = { v.x, v.y, v.z, 1.0f };
        return uniform_LM * gl_Vertex;
    }
    virtual std::pair<bool, TGAColor> fragment(const Vec3f bar) const{
        TGAColor fincolor = white;
        return{ true, fincolor };
    }
};

int main(int argc, char** argv) {
    Model Aface("Adefaultface.obj");
    Model Abody("Adefaultbody.obj");

    tinfgl render;

    constexpr int width = 800;
    constexpr int height = 800;
     Vec3f eye = { 1, 2, 5 }; 
     Vec3f center = { 0, 1, 0 }; 
     Vec3f  up = { 0, 1, 0 }; 

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage z(width, height, TGAImage::GRAYSCALE);
    TGAImage face;
    TGAImage body;
    TGAImage NMf;
    TGAImage NMb;
    NMf.read_tga_file("Image_2.tga");
    NMb.read_tga_file("Image_5.tga");
    face.read_tga_file("Image_0.tga");
    body.read_tga_file("Image_3.tga");

    TGAColor bg = { 30, 30, 40, 255 };
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
            image.set(x, y, white);


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
    Vec3f Leye = center + L * 5.0f;
    render.lookatL(Leye, center, up);
    render.init_Lperspo(3.0f);
    Matrix4f LM = Lperspo * viewp * Lmodelv;

    shadowmap s(Abody, LM);
    for (int i = 0; i < Abody.faces_.size(); ++i) {
        qmhsV<Model::VertexData> fs = Abody.faces_[i];
        Vec4f clip_coords[3];
        for (int j = 0; j < 3; ++j)
            clip_coords[j] = s.vertex(i, j);
        render.rasterizationL(clip_coords[0], clip_coords[1], clip_coords[2], width, height);
    }
  
    shadowmap s1(Aface, LM);
    for (int i = 0; i < Aface.faces_.size(); ++i) {
        qmhsV<Model::VertexData> fs = Aface.faces_[i];
        Vec4f clip_coords[3];
        for (int j = 0; j < 3; ++j)
            clip_coords[j] = s1.vertex(i, j);
        render.rasterizationL(clip_coords[0], clip_coords[1], clip_coords[2], width, height);
    }

    PhongShader phong(Abody, M, modelv, LM, viewp, L, eye, body, NMb, zbuffer2, width, height);
    PhongShader phong1(Aface, M, modelv, LM, viewp, L, eye, face, NMf, zbuffer2, width, height);

    //raster
    for (int i = 0; i < Abody.faces_.size(); ++i) {
        qmhsV<Model::VertexData> fs = Abody.faces_[i];
        Vec4f clip_coords[3];
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
        for (int j = 0; j < 3; ++j)
            clip_coords[j] = phong1.vertex(i, j);
     
        render.rasterization(clip_coords[0], clip_coords[1], clip_coords[2],
            image, z, width, height, phong1);
    }


    image.write_tga_file("image.tga");
    system("start image.tga");

    return 0;
}