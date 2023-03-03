#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Model.h"

namespace ABraveFish {
Model::Model(const char* filename)
    : verts_()
    , faces_()
    , norms_()
    , uv_()
    , diffusemap_() {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
        return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char               trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3 v;
            iss >> v.x >> v.y >> v.z;
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3 n;
            iss >> n.x >> n.y >> n.z;
            norms_.push_back(n);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2 uv;
            iss >> uv.x >> uv.y;
            uv_.push_back(uv);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3> f;
            Vec3              tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i = 0; i < 3; i++)
                    tmp[i]--; // in wavefront obj all indices start at 1, not zero
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# "
              << norms_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
}

Model::~Model() {}

int Model::nverts() { return (int)verts_.size(); }

int Model::nfaces() { return (int)faces_.size(); }

std::vector<int> Model::face(int idx) {
    std::vector<int> face;
    for (int i = 0; i < (int)faces_[idx].size(); i++)
        face.push_back(faces_[idx][i][0]);
    return face;
}

Vec3 Model::vert(int i) { return verts_[i]; }

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
    std::string texfile(filename);
    size_t      dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed")
                  << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2 uv) { return diffusemap_.get(uv.x, uv.y); }

Vec2 Model::uv(int iface, int nvert) {
    int idx = faces_[iface][nvert][1];
    return Vec2(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}

Vec3 Model::norm(int iface, int nvert) {
    int idx = faces_[iface][nvert][2];
    return norms_[idx].normalize();
}
} // namespace ABraveFish
