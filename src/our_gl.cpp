#include "our_gl.h"


qmhsV<float> zbuffer;
qmhsV<float> zbuffer2;
Matrix4f modelv;
Matrix4f perspo;
Matrix4f viewp;
Matrix4f Lmodelv;
Matrix4f Lperspo;


void tinfgl::lookat(const Vec3f eye, const Vec3f center, const Vec3f up) {
    Vec3f n = (eye - center).normalize();
    Vec3f l = (cross(up, n)).normalize();
    Vec3f m = (cross(n, l)).normalize();
    modelv = Matrix4f{ l.x, l.y, l.z, 0.0f, 
                       m.x, m.y, m.z, 0.0f, 
                       n.x, n.y, n.z, 0.0f, 
                       0.0f,0.0f,0.0f,1.0f } *
                                             Matrix4f{1, 0, 0, -center.x,  
                                                      0, 1, 0, -center.y , 
                                                      0, 0, 1, -center.z , 
                                                      0, 0, 0, 1 };
}
void tinfgl::lookatL(const Vec3f eye, const Vec3f center, const Vec3f up) {
    Vec3f n = (eye - center).normalize();
    Vec3f l = (cross(up, n)).normalize();
    Vec3f m = (cross(n, l)).normalize();
    Lmodelv = Matrix4f{ l.x, l.y, l.z, 0.0f,
                       m.x, m.y, m.z, 0.0f,
                       n.x, n.y, n.z, 0.0f,
                       0.0f,0.0f,0.0f,1.0f } *
        Matrix4f{ 1, 0, 0, -center.x,
                 0, 1, 0, -center.y ,
                 0, 0, 1, -center.z ,
                 0, 0, 0, 1 };
}


void tinfgl::init_perspective(const float f) {
    perspo = { 1, 0, 0, 0,
               0, 1, 0, 0,
               0, 0, 1, 0,
               0, 0, -1/f, 1 };
}

void tinfgl::init_Lperspo(const float f) {
    Lperspo = { 1.0f / f, 0,    0,     0,
               0    , 1.0f / f,0,     0,
               0    , 0,    1.0f / f, 0,
               0    , 0,    0,     1 };
}

void tinfgl::init_viewport(const int x, const int y, const int width, const int height) {
    viewp = { width / 2.0f, 0,             0, x+width / 2.0f,
              0,            height / 2.0f, 0, y+height / 2.0f,
              0,            0,             1, 0,
              0,            0,             0, 1 };
}

/*void tinfgl::initProject(Vec3f v, float cx, float cy, float cz, float dx, float dy, float dz) {
    
    Vec3f n1 = v;
    Matrix4f O = { 1, 0, 0, -cx,
                      0, 1 / dy , 0, -cy,
                      0, 0, 1 / dz, -cz,
                      0, 0, 0, 1 };
    n1 = O * n1;
    n1 = viewp * n1;

    float f = 10000.0f;
    */
    
    //float c = - 1 / f;


void tinfgl::initZ(int width, int height) {
    zbuffer.clear();
    zbuffer.resize(width * height, -std::numeric_limits<float>::max());
    zbuffer2.clear();
    zbuffer2.resize(width * height, -std::numeric_limits<float>::max());
}

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


void tinfgl::rasterization(Vec4f a0, Vec4f a1, Vec4f a2, 
                            TGAImage& image, TGAImage& z, int width, int height, IShader& shader) {
    Vec4f ndc[3] = { a0/a0.w, a1/a1.w, a2/a2.w };
    Vec2f screeen[3] = { (a0).xy(), (a1).xy(), (a2).xy()};
    

    //std::cout << a0.x << "," << a0.y << " " << a1.x << "," << a1.y << " " << a2.x << "," << a2.y << std::endl;
    int minx_f = std::floor(std::min({ a0.x, a1.x, a2.x }));
    int maxx_f = std::ceil(std::max({ a0.x, a1.x, a2.x }));
    int miny_f = std::floor(std::min({ a0.y, a1.y, a2.y }));
    int maxy_f = std::ceil(std::max({ a0.y, a1.y, a2.y }));

    Matrix3f ABC = { a0.x, a1.x, a2.x,
                    a0.y, a1.y, a2.y,
                    1, 1, 1 };
    if (ABC.det() < 1) return;

    for (int i = std::max(0, minx_f); i < std::min(width - 1, maxx_f); i++) {
        for (int j = std::max(0, miny_f); j < std::min(height - 1, maxy_f); j++) {
            Vec3f a = ABC.invert() * Vec3f(static_cast<float>(i), static_cast<float>(j), 1.);
            
            if (a.x < -0.01 || a.y < -0.01 || a.z < -0.01) continue;
            float zb = a * Vec3f(ndc[0].z, ndc[1].z, ndc[2].z);
            int idx = i + j * width;
            if (zb >= zbuffer[idx]) {
                bool discard;
                TGAColor color;
                std::tie(discard, color) = shader.fragment(a);
                zbuffer[idx] = zb;
                if (!discard) {
                    zbuffer[idx] = zb;
                    image.set(i, j, color);
                }
            }
        }
    }
}

void tinfgl::rasterizationL(Vec4f a0, Vec4f a1, Vec4f a2, int width, int height) {
    Vec4f ndc[3] = { a0 / a0.w, a1 / a1.w, a2 / a2.w };
    int minx_f = std::floor(std::min({ a0.x, a1.x, a2.x }));
    int maxx_f = std::ceil(std::max({ a0.x, a1.x, a2.x }));
    int miny_f = std::floor(std::min({ a0.y, a1.y, a2.y }));
    int maxy_f = std::ceil(std::max({ a0.y, a1.y, a2.y }));
    Matrix3f ABC = { a0.x, a1.x, a2.x,
                   a0.y, a1.y, a2.y,
                   1, 1, 1 };
    if (ABC.det() < 1) return;

    for (int i = std::max(0, minx_f); i < std::min(width - 1, maxx_f); i++) {
        for (int j = std::max(0, miny_f); j < std::min(height - 1, maxy_f); j++) {
            Vec3f a = ABC.invert() * Vec3f(static_cast<float>(i), static_cast<float>(j), 1.);

            if (a.x < -0.01 || a.y < -0.01 || a.z < -0.01) continue;
            float zb = a * Vec3f(ndc[0].z, ndc[1].z, ndc[2].z);
            int idx = i + j * width;
            if (zb >= zbuffer2[idx]) {
                zbuffer2[idx] = zb;
                    zbuffer2[idx] = zb;
            }
        }
    }
}



void::tinfgl::drawZ(qmhsV<float>& zbuffer, int width, int height, TGAImage z) {
    
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

}

