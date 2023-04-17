#pragma once

#include <iostream>
#include "Core/Image.h"
#include "glad/glad.h"

#include "GLFW/glfw3.h"

/*
* 为啥客户端和引擎同时引用同一个.h（把定义都写在h里了）就会出现链接问题呢？
* 理论上h不是就include一次么，难道不同的project不在一个范围内？
*/

namespace ABraveFish {
// 不能在循环内执行， 会一直消耗内存
unsigned int registeOpenGLTexture(uint32_t width, uint32_t height);

void refreshOpenGLTexture(unsigned char* buffer, uint32_t width, uint32_t height);

void loadTexture(std::string filename, const char* suffix, TGAImage& img);
} // namespace ABraveFish
