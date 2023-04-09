#include "RenderBuffer.h"

#include <assert.h>

namespace ABraveFish {
RenderBuffer::RenderBuffer(int width, int height) {
    int32_t   colorBufferSize = width * height * 4;
    int32_t   depthBufferSize = sizeof(float) * width * height;
    Color defaultColor     = {0, 0, 0, 1};
    float defaultDepth     = 1;

    assert(width > 0 && height > 0);

    this->_width       = width;
    this->_height      = height;
    this->_colorBuffer = (unsigned char*)malloc(colorBufferSize);
    this->_depthBuffer = (float*)malloc(depthBufferSize);

    this->clearColor(defaultColor);
    this->clearDepth(defaultDepth);
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

void RenderBuffer::setColor(int x, int y, Color Color) {
    int index               = (y * _width + x) * 4;
    _colorBuffer[index + 0] = Color.r * 255;
    _colorBuffer[index + 1] = Color.g * 255;
    _colorBuffer[index + 2] = Color.b * 255;
}

Color RenderBuffer::getColor(int x, int y) {
    int index = (y * _width + x) * 4;
    return Color(_colorBuffer[index + 0] / 255.f, _colorBuffer[index + 1] / 255.f, _colorBuffer[index + 2] / 255.f);
}

void RenderBuffer::release() {
    free(_colorBuffer);
    free(_depthBuffer);
    //free(this);
}

void RenderBuffer::clearColor(Color Color) {
    int num_pixels = this->_width * this->_height;
    int i;
    for (i = 0; i < num_pixels; i++) {
        this->_colorBuffer[i * 4 + 0] = Color.r * 255;
        this->_colorBuffer[i * 4 + 1] = Color.g * 255;
        this->_colorBuffer[i * 4 + 2] = Color.b * 255;
        this->_colorBuffer[i * 4 + 3] = Color.a * 255;
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

