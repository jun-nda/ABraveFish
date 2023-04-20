#pragma once

#include "glm/glm.hpp"
#include "Core/Macros.h"
#include "Core/Color.h"

namespace ABraveFish{
float distributionGGX(float n_dot_h, float roughness) {
    float alpha  = roughness * roughness;
    float alpha2 = alpha * alpha;

    float n_dot_h_2 = n_dot_h * n_dot_h;
    float factor    = n_dot_h_2 * (alpha2 - 1) + 1;
    return alpha2 / (PI * factor * factor);
}

float SchlickGGX_geometry(float n_dot_v, float roughness) {
    float r = (1 + roughness);
    float k = r * r / 8.0;

    return n_dot_v / (n_dot_v * (1 - k) + k);
}

//几何遮挡计算的是物体宏观表面的影响，所以是宏观法向
float geometry_Smith(float n_dot_v, float n_dot_l, float roughness) {
    float g1 = SchlickGGX_geometry(n_dot_v, roughness);
    float g2 = SchlickGGX_geometry(n_dot_l, roughness);

    return g1 * g2;
}

//菲涅尔项是因观察角度与反射平面方向的夹角而引起的反射程度不同
//所以菲尼尔项计算的是微平面法向(真正有贡献的微平面)与观察方向的夹角
glm::vec3 fresenlschlick(float h_dot_v, glm::vec3& f0) {
    return f0 + (glm::vec3(1.0, 1.0, 1.0) - f0) * (float)pow(1 - h_dot_v, 5.0);
}

glm::vec3 fresenlschlick_roughness(float h_dot_v, glm::vec3& f0, float roughness) {
    float r1 = 1.0f - roughness;
    if (r1 < f0[0])
        r1 = f0[0];
    return f0 + (glm::vec3(r1, r1, r1) - f0) * pow(1 - h_dot_v, 5.0f);
}

// other utility functions
float float_clamp(float f, float min, float max) { return f < min ? min : (f > max ? max : f); }
float float_aces(float value) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    value   = (value * (a * value + b)) / (value * (c * value + d) + e);
    return float_clamp(value, 0, 1);
}

static Color Reinhard_mapping(Color& color) {
    int i;
    for (i = 0; i < 3; i++) {
        color[i] = float_aces(color[i]);
        // color[i] = color[i] / (color[i] + 0.5);
        color[i] = pow(color[i], 1.0 / 2.2);
    }
    return color;
}
}