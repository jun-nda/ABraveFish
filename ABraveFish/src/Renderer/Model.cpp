#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Model.h"

namespace ABraveFish {
Model::Model(const char* filename)
    : m_Verts()
    , m_Faces()
    , m_Norms()
    , m_UV()
    , m_Diffusemap() {
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
            m_Verts.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3 n;
            iss >> n.x >> n.y >> n.z;
            m_Norms.push_back(n);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2 uv;
            iss >> uv.x >> uv.y;
            m_UV.push_back(uv);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3> f;
            Vec3              tmp;
            iss >> trash;
            while (iss >> tmp.x >> trash >> tmp.y >> trash >> tmp.z) {
                // in wavefront obj all indices start at 1, not zero
                tmp.x--;
                tmp.y--;
                tmp.z--;
                f.push_back(tmp);
            }
            m_Faces.push_back(f);
        }
    }
    std::cerr << "# v# " << m_Verts.size() << " f# " << m_Faces.size() << " vt# " << m_UV.size() << " vn# "
              << m_Norms.size() << std::endl;
    load_texture(filename, "_diffuse.tga", m_Diffusemap);
}

Model::~Model() {}

int32_t Model::GetVertCount() { return (int32_t)m_Verts.size(); }

int32_t Model::GetFaceCount() { return (int32_t)m_Faces.size(); }

std::vector<int32_t> Model::Face(int32_t idx) {
    std::vector<int32_t> face;
    for (int32_t i = 0; i < (int32_t)m_Faces[idx].size(); i++)
        face.push_back(m_Faces[idx][i].x);
    return face;
}

Vec3 Model::Vert(int32_t i) { return m_Verts[i]; }

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

TGAColor Model::Diffuse(Vec2 uv) { return m_Diffusemap.get(uv.x, uv.y); }

Vec2 Model::UV(int32_t iface, int32_t nvert) {
    int32_t idx = m_Faces[iface][nvert].y;
    return Vec2({m_UV[idx].x * m_Diffusemap.get_width(), m_UV[idx].y * m_Diffusemap.get_height()});
}

Vec3 Model::Norm(int32_t iface, int32_t nvert) {
    int32_t idx = m_Faces[iface][nvert].z;
    return vec3_normalize(m_Norms[idx]);
}
} // namespace ABraveFish
