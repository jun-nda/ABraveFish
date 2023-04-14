#pragma once

#include "SkyBox.h"
#include "Core/Image.h"
#include "Util.h"

namespace ABraveFish {
void SkyBox::loadCubeMap(const char* filename) {
    loadTexture(filename, "_right.tga", _enviromentMap.faces[0]);
    loadTexture(filename, "_left.tga", _enviromentMap.faces[1]);
    loadTexture(filename, "_top.tga", _enviromentMap.faces[2]);
    loadTexture(filename, "_bottom.tga", _enviromentMap.faces[3]);
    loadTexture(filename, "_back.tga", _enviromentMap.faces[4]);
    loadTexture(filename, "_front.tga", _enviromentMap.faces[5]);
}
}