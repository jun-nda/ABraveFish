#pragma once

#include "Core/Image.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "model.h"

namespace ABraveFish {
void      DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, RenderBuffer* rdBuffer, TGAColor color);
glm::mat4 lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
glm::mat4 perspective(float fovy, float aspect, float near, float far);
glm::vec3 viewport_transform(int width, int height, glm::vec3 ndc_coord);
bool      isBackFacing(glm::vec3 ndc_coords[]);
//float     interpolateDepth(float* screenDepth, glm::vec3 weights);
glm::vec3 Barycentric(glm::vec3* v, float x, float y);
glm::vec3 Barycentric(glm::vec3* pts, glm::vec2 P);
    void      interpolate_varyings(shader_struct_v2f* v2f, shader_struct_v2f* ret, int sizeof_varyings, glm::vec3 weights,
                               float recip_w[3]);
void      rasterization(DrawData* data, shader_struct_v2f* v2f, bool isSkyBox = false);
void      vertexProcessing(DrawData* drawData, shader_struct_a2v* a2v, bool isSkyBox = false);

int32_t homoClipping(HomogenousClip& clipData);
int32_t clipWithPlane(ClipPlane plane, int32_t numVertex, HomogenousClip& clipData);

bool  isInsidePlane(ClipPlane plane, glm::vec4& numVertex);
float getIntersectRatio(glm::vec4& preVertex, glm::vec4& curVertex, ClipPlane plane);
void  transformAttri(shader_struct_v2f* v2fs, HomogenousClip& clipData, int32_t* indexArray);

// for cube test
//glm::vec3 Barycentric(std::vector<glm::vec3> pts, glm::vec2 P);
//void      DrawTriangle(glm::vec3* pts, float* zbuffer, TGAImage* image, TGAColor color);

void PrintMatrix(glm::mat4 m);

} // namespace ABraveFish
