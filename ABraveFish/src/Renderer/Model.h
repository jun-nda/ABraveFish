#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "Core/Image.h"
#include "glm/glm.hpp"
#include "Shader.h"

namespace ABraveFish {
class Model {
public:
    Model(const char* filename);
    ~Model();
    int32_t              getVertCount();
    int32_t              getFaceCount();
    glm::vec3            getNormal(int32_t iface, int32_t nvert);
    glm::vec3            getVert(int32_t i);
    glm::vec2            getUV(int32_t iface, int32_t nvert);
    std::vector<int32_t> getFace(int32_t idx); // 拿到第idx个面的三个顶点数据vert的索引

public:
    TGAColor diffuseSample(glm::vec2 uv);
    TGAColor normalSample(glm::vec2 uv);
    TGAColor specularSample(glm::vec2 uv);

public:
    // textures
    TGAImage _diffuseMap;
    TGAImage _normalMap;
    TGAImage _specularMap;

private:
    std::vector<glm::vec3>              _verts;
    std::vector<std::vector<glm::vec3>> _faces; // attention, this Vec3 means vertex/uv/normal
    std::vector<glm::vec3>              _norms;
    std::vector<glm::vec2>              _uv;
};
} // namespace ABraveFish

#endif //__MODEL_H_