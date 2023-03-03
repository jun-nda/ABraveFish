#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "Image.h"
#include "Math.h"

namespace ABraveFish {
class Model {
private:
    std::vector<Vec3>              verts_;
    std::vector<std::vector<Vec3>> faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3>              norms_;
    std::vector<Vec2>              uv_;
    TGAImage                       diffusemap_;
    void                           load_texture(std::string filename, const char* suffix, TGAImage& img);

public:
    Model(const char* filename);
    ~Model();
    int              nverts();
    int              nfaces();
    Vec3           norm(int iface, int nvert);
    Vec3            vert(int i);
    Vec2            uv(int iface, int nvert);
    TGAColor         diffuse(Vec2 uv);
    std::vector<int> face(int idx);
};
} // namespace ABraveFish

#endif //__MODEL_H__