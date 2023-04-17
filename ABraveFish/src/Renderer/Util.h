#pragma once

#include <iostream>
#include "Core/Image.h"
#include "glad/glad.h"

#include "GLFW/glfw3.h"

/*
* Ϊɶ�ͻ��˺�����ͬʱ����ͬһ��.h���Ѷ��嶼д��h���ˣ��ͻ�������������أ�
* ������h���Ǿ�includeһ��ô���ѵ���ͬ��project����һ����Χ�ڣ�
*/

namespace ABraveFish {
// ������ѭ����ִ�У� ��һֱ�����ڴ�
unsigned int registeOpenGLTexture(uint32_t width, uint32_t height);

void refreshOpenGLTexture(unsigned char* buffer, uint32_t width, uint32_t height);

void loadTexture(std::string filename, const char* suffix, TGAImage& img);
} // namespace ABraveFish
