#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "Image.h"
#include "glm/glm.hpp"

namespace ABraveFish {
class Model {
private:
    std::vector<glm::vec3>              m_Verts;
    std::vector<std::vector<glm::vec3>> m_Faces; // attention, this Vec3 means vertex/uv/normal
    std::vector<glm::vec3>              m_Norms;
    std::vector<glm::vec2>             m_UV;
    TGAImage                       m_Diffusemap;
    void                           load_texture(std::string filename, const char* suffix, TGAImage& img);

public:
    Model(const char* filename);
    ~Model();
    int32_t              GetVertCount();
    int32_t              GetFaceCount();
    glm::vec3            Norm(int32_t iface, int32_t nvert);
    glm::vec3            Vert(int32_t i);
    glm::vec2            UV(int32_t iface, int32_t nvert);
    TGAColor             Diffuse(glm::vec2 uv);
    std::vector<int32_t> Face(int32_t idx); // 拿到第idx个面的三个顶点数据vert的索引
};
} // namespace ABraveFish

#endif //__MODEL_H__