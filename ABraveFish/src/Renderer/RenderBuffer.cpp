#include "RenderBuffer.h"

#include <assert.h>

namespace ABraveFish {
RenderBuffer::RenderBuffer(int width, int height) {
    int   color_buffer_size = width * height * 4;
    int   depth_buffer_size = sizeof(float) * width * height;
    TGAColor default_color     = {0, 0, 0, 1};
    float default_depth     = 1;

    assert(width > 0 && height > 0);

    this->_width       = width;
    this->_width       = height;
    this->_colorBuffer = (unsigned char*)malloc(color_buffer_size);
    this->_depthBuffer = (float*)malloc(depth_buffer_size);

    this->clearColor(default_color);
    this->clearDepth(default_depth);
}

RenderBuffer::~RenderBuffer() {
    free(_colorBuffer);
    free(_depthBuffer);
}

void RenderBuffer::setDepth(int x, int y, float depth) {
    int index           = y * _width + x;
    _depthBuffer[index] = depth;
}

float RenderBuffer::getDepth(int x, int y) {
    int index = y * _width + x;
    return _depthBuffer[index];
}

void RenderBuffer::setColor(int x, int y, TGAColor TGAColor) {
    int index               = (y * _width + x) * 4;
    _colorBuffer[index + 0] = TGAColor.r * 255;
    _colorBuffer[index + 1] = TGAColor.g * 255;
    _colorBuffer[index + 2] = TGAColor.b * 255;
}

TGAColor RenderBuffer::getColor(int x, int y) {
    int index = (y * _width + x) * 4;
    return TGAColor(_colorBuffer[index + 0] / 255.f, _colorBuffer[index + 1] / 255.f, _colorBuffer[index + 2] / 255.f);
}

void RenderBuffer::release() {
    free(_colorBuffer);
    free(_depthBuffer);
    //free(this);
}

void RenderBuffer::clearColor(TGAColor TGAColor) {
    int num_pixels = this->_width * this->_height;
    int i;
    for (i = 0; i < num_pixels; i++) {
        this->_colorBuffer[i * 4 + 0] = TGAColor.r * 255;
        this->_colorBuffer[i * 4 + 1] = TGAColor.g * 255;
        this->_colorBuffer[i * 4 + 2] = TGAColor.b * 255;
        this->_colorBuffer[i * 4 + 3] = TGAColor.a * 255;
    }
}

void RenderBuffer::clearDepth(float depth) {
    int num_pixels = this->_width * this->_height;
    int i;
    for (i = 0; i < num_pixels; i++) {
        this->_depthBuffer[i] = depth;
    }
}
}

