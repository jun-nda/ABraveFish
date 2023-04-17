#pragma once

#include "Core/Image.h"
#include "Core/Color.h"

namespace ABraveFish {
class RenderBuffer {
public:
    RenderBuffer(int width, int height);
    ~RenderBuffer();


    void     setDepth(int x, int y, float depth);
    float    getDepth(int x, int y);
    void     setColor(int x, int y, const Color& color);
    Color getColor(int x, int y);

    void release();
    void clearColor(const Color& color);
    void clearDepth(float depth);

public:
    int32_t        _width, _height;
    unsigned char* _colorBuffer;
    float*         _depthBuffer;
};
} // namespace ABraveFish
