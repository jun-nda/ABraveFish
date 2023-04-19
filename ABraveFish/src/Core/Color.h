#pragma once
#include <cassert>
#include "Image.h"
#include <glm/glm.hpp>

namespace ABraveFish {
class Color {
public:
    float r, g, b, a;
    Color();
    Color(float R, float G, float B, float A = 255);
    Color(const TGAColor& tga_color);
    Color(const glm::vec3& color);


    static float MAX_CHANNEL_VALUE;
    static Color White;
    static Color Red;
    static Color Green;
    static Color Blue;
    static Color Black;

    float& operator[](const size_t i);

    float operator[](const size_t i) const;

    Color operator+(const Color& color) const;

    Color operator*(float intensity) const;

    Color operator*(const Color& color) const;
};
}
