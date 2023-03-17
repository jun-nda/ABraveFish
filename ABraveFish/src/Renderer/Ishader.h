//#pragma once
//
//#include "Core/Image.h"
//#include "RenderBuffer.h"
//
//#include "glm/glm.hpp"
//
//namespace ABraveFish {
//
//struct Material {
//    TGAImage* diffuse_map;
//    TGAImage* normal_map;
//    TGAImage* specular_map;
//    TGAColor  color;
//    TGAColor  specular;
//    float     gloss;
//    float     bump_scale;
//};
//
//struct ShaderData {
//    Material*     matrial;
//    RenderBuffer* targetBuffer;
//    RenderBuffer* shadow_map;
//    bool          enable_shadow;
//    glm::vec3     view_Pos;
//    glm::vec3     light_dir;
//    TGAColor      light_color;
//    TGAColor      ambient;
//    glm::mat4     model_matrix;
//    glm::mat4     model_matrix_I;
//    glm::mat4     view_matrix;
//    glm::mat4     projection_matrix;
//    glm::mat4     light_vp_matrix;
//    glm::mat4     camera_vp_matrix;
//};
//
//struct shader_struct_a2v {
//    glm::vec3 obj_pos;
//    glm::vec3 obj_normal;
//    glm::vec2 uv;
//};
//
//struct shader_struct_v2f {
//    glm::vec4 clip_pos;
//    glm::vec3 world_pos;
//    glm::vec3 world_normal;
//    glm::vec2 uv;
//    float     intensity;
//};
//
//class IShader {
//    ShaderData* shader_data;
//
//    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v)                    = 0;
//    virtual bool              fragment(shader_struct_v2f* v2f, TGAColor& color) = 0;
//};
//} // namespace ABraveFish