#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Model.h"
#include "Util.h"

namespace ABraveFish {
Model::Model(const char* filename, bool isSkyBox, bool vReverse)
    : _verts()
    , _faces()
    , _norms()
    , _uv()
    , _diffuseMap() {
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
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            _verts.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            _norms.push_back(n);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            if (vReverse) {
                uv.y = 2 - uv.y;
            }
            _uv.push_back(uv);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<glm::vec3> f;
            glm::vec3              tmp;
            iss >> trash;
            while (iss >> tmp.x >> trash >> tmp.y >> trash >> tmp.z) {
                // in wavefront obj all indices start at 1, not zero
                tmp.x--;
                tmp.y--;
                tmp.z--;
                f.push_back(tmp);
            }
            _faces.push_back(f);
        }
    }
    std::cerr << "# v# " << _verts.size() << " f# " << _faces.size() << " vt# " << _uv.size() << " vn# "
              << _norms.size() << std::endl;

    if (isSkyBox) {
        loadCubeMap(filename);
    } else {
        loadTexture(filename, "_diffuse.tga", _diffuseMap);
        //loadTexture(filename, "_nm_tangent.tga", _normalMap);
        loadTexture(filename, "_metalness.tga", _metalnessMap);
        loadTexture(filename, "_normal.tga", _normalMap);
        loadTexture(filename, "_occlusion.tga", _occlusionMap);
        loadTexture(filename, "_roughness.tga", _roughnessMap);
        //loadTexture(filename, "_spec.tga", _specularMap);

    } 
}

Model::~Model() {}

int32_t Model::getVertCount() { return (int32_t)_verts.size(); }

int32_t Model::getFaceCount() { return (int32_t)_faces.size(); }

std::vector<int32_t> Model::getFace(int32_t idx) {
    std::vector<int32_t> face;
    for (int32_t i = 0; i < (int32_t)_faces[idx].size(); i++)
        face.push_back(_faces[idx][i].x);
    return face;
}

glm::vec3 Model::getVert(int32_t i) { return _verts[i]; }

glm::vec2 Model::getUV(int32_t iface, int32_t nvert) {
    int32_t idx = _faces[iface][nvert].y;
    //std::cout << _uv[idx].x << " " << _uv[idx].y << std::endl;
    return glm::vec2({_uv[idx].x * _diffuseMap.get_width(), _uv[idx].y * _diffuseMap.get_height()});
}

glm::vec3 Model::getNormal(int32_t iface, int32_t nvert) {
    int32_t idx = _faces[iface][nvert].z;
    return glm::normalize(_norms[idx]);
}

TGAColor Model::diffuseSample(glm::vec2 uv) { return _diffuseMap.get(uv.x, uv.y); }
TGAColor Model::normalSample(glm::vec2 uv) { return _normalMap.get(uv.x, uv.y); }
TGAColor Model::specularSample(glm::vec2 uv) { return _specularMap.get(uv.x, uv.y); }

void Model::loadCubeMap(const char* filename) {
    loadTexture(filename, "_right.tga", _enviromentMap.faces[0]);
    loadTexture(filename, "_left.tga", _enviromentMap.faces[1]);
    loadTexture(filename, "_top.tga", _enviromentMap.faces[2]);
    loadTexture(filename, "_bottom.tga", _enviromentMap.faces[3]);
    loadTexture(filename, "_back.tga", _enviromentMap.faces[4]);
    loadTexture(filename, "_front.tga", _enviromentMap.faces[5]);
    for (int i = 0; i < 6; i++) {
        _enviromentMap.faces[i].flip_vertically();
    }
}

} // namespace ABraveFish
