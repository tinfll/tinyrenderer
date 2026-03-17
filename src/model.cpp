#include <fstream>
#include <sstream>
#include <model.h>
#include <stdio.h>

Model::Model(const char* filename) {
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
                Vec4f v;
                ss >> v.x >> v.y >> v.z;
                v.w = 1.0f;
                verts_.push_back(v);
            }
            else if (a == "f") {
                qmhsV<VertexData> fs;
                std::string s;

                while (ss >> s) {
                    int v, t, n;
                    VertexData vi = { -1, -1, -1 };
                    if (sscanf(s.c_str(), "%d/%d/%d", &v, &t, &n) == 3)//my obj just invovled these.
                    {
                        vi.id = v - 1;
                        vi.t = t - 1;
                        vi.n = n - 1;
                    }                        
                    fs.push_back(vi);

                }
                if (fs.size() == 4) {
                    qmhsV<VertexData> tri1, tri2;
                    tri1.push_back(fs[0]); tri1.push_back(fs[1]); tri1.push_back(fs[2]);
                    tri2.push_back(fs[0]); tri2.push_back(fs[2]); tri2.push_back(fs[3]);
                    faces_.push_back(tri1);
                    faces_.push_back(tri2);
                }
                else if (fs.size() == 3) {
                    faces_.push_back(fs);
                }
            }
            else if (a == "vn") {
                Vec4f v;
                ss >> v.x >> v.y >> v.z;
                v.w = 1.0f;
                vertsN_.push_back(v);
            }
            else if (a == "vt") {
                Vec4f v;
                ss >> v.x >> v.y >> v.z;
                v.w = 1.0f;
                vertsT_.push_back(v);
            }
        }
    }
    
    //NORMALS SMOOTH
    //vertsN_.resize(verts_.size());
    //for (int i = 0; i < vertsN_.size(); ++i)
    //    vertsN_[i] = { 0.0f, 0.0f, 0.0f };
    //for (int i = 0; i < faces_.size(); ++i) {
      /*  qmhsV<VertexData> f = faces_[i];

        if (f.size() == 3) {
            Vec4f v0 = verts_[f[0]];
            Vec4f v1 = verts_[f[1]];
            Vec4f v2 = verts_[f[2]];

            Vec3f AB = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
            Vec3f AC = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
            Vec3f N0 = cross(AB, AC).normalize();

            vertsN_[f[0]] = vertsN_[f[0]] + N0;
            vertsN_[f[1]] = vertsN_[f[1]] + N0;
            vertsN_[f[2]] = vertsN_[f[2]] + N0;
        }
    }
    for (int i = 0; i < vertsN_.size(); ++i)
        vertsN_[i] = vertsN_[i].normalize();
    std::cout << "has normal successfully" << std::endl;
    */
    for (int i = 0; i < verts_.size(); i++) {
		Vec4f v = verts_[i];
        if (v.x < min_x) min_x = v.x;
        if (v.x > max_x) max_x = v.x;
        if (v.y < min_y) min_y = v.y;
        if (v.y > max_y) max_y = v.y;
        if (v.z < min_z) min_z = v.z;
        if (v.z > max_z) max_z = v.z;
    }

    cx = (min_x + max_x) / 2.0f;
    cy = (min_y + max_y) / 2.0f;
    cz = (min_z + max_z) / 2.0f;

    dx = (-min_x + max_x) / 2.0f;
    dy = (-min_y + max_y) / 2.0f;
    dz = (-min_z + max_z) / 2.0f;

    mw = max_x - min_x;
    mh = max_y - min_y;
    std::cout << "Model loaded. Center: " << cx << ", " << cy << std::endl;
    std::cout << "Model loaded. Center: " << mw << ", " << mh << std::endl;
}

