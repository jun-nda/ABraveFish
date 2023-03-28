#pragma once

#include "Core/Image.h"
#include "RenderBuffer.h"

#include "glm/glm.hpp"

namespace ABraveFish {

struct Material {
    TGAImage* _diffuseMap;
    TGAImage* _normalMap;
    TGAImage* _specularMap;
    //TGAColor  color;
    //TGAColor  specular;
    //float     gloss;
    //float     bump_scale;
};

struct shader_struct_a2v {
    glm::vec3 obj_pos;
    glm::vec3 obj_normal;
    glm::vec2 uv;
};

struct shader_struct_v2f {
    glm::vec4 clip_pos;
    glm::vec3 world_pos;
    glm::vec3 world_normal;
    glm::vec2 uv;
    float     intensity;
};

class IShader {
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v)                    = 0;
    virtual bool              fragment(shader_struct_v2f* v2f, TGAColor& color) = 0;
};


} // namespace ABraveFish