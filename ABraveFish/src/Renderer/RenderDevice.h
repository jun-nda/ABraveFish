#pragma once

#include "Core/Image.h"
#include "glm/glm.hpp"
#include "model.h"
#include "Shader.h"

namespace ABraveFish {
void      DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, TGAImage* image, TGAColor color);
glm::mat4 lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
glm::mat4 perspective(float fovy, float aspect, float near, float far);
glm::vec3 viewport_transform(int width, int height, glm::vec3 ndc_coord);
bool      isBackFacing(glm::vec3 ndc_coords[]);
float     interpolateDepth(float* screenDepth, glm::vec3 weights);
glm::vec3 Barycentric(glm::vec3* pts, glm::vec2 P);
void      interpolate_varyings(shader_struct_v2f* v2f, shader_struct_v2f* ret, int sizeof_varyings, glm::vec3 weights,
                               float recip_w[3]);
void      rasterization(DrawData* data, shader_struct_v2f* v2f);

// for cube test
glm::vec3 Barycentric(std::vector<glm::vec3> pts, glm::vec2 P);
void      DrawTriangle(glm::vec3* pts, float* zbuffer, TGAImage* image, TGAColor color);

void PrintMatrix(glm::mat4 m);

} // namespace ABraveFish
