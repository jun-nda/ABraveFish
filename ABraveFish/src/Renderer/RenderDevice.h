#pragma once

#include <iostream>

#include "Image.h"

namespace ABraveFish {
static void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, TGAImage* image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int32_t dx = x1 - x0;
    int32_t dy = y1 - y0;

    int32_t derror2 = std::abs(dy) * 2;
    int32_t error2  = 0;

    int32_t y = y0;
    for (int32_t x = x0; x <= x1; x++) {
        if (steep) {
            image->set(y, x, color);
        } else {
            image->set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}
}

