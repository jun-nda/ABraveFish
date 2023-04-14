#pragma once

#include <iostream>

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_opengl3_loader.h"

namespace ABraveFish {
// 不能在循环内执行， 会一直消耗内存 
unsigned int registeOpenGLTexture(uint32_t width, uint32_t height);
void refreshOpenGLTexture(unsigned char* buffer, uint32_t width, uint32_t height) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}

void loadTexture(std::string filename, const char* suffix, TGAImage& img) {
    std::string texfile(filename);
    size_t      dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed")
                  << std::endl;
        img.flip_vertically();
    }
}


} // namespace ABraveFish
