#include <fstream>
#include <sstream>
#include <model.h>

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
                qmhsV<int> fs;
                int f;
                std::string s;
                while (ss >> s) {
                    f = std::stoi(s);
                    fs.push_back(f - 1);

                }
                faces_.push_back(fs);
            }
            else {}
        }
    }

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

