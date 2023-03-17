#pragma once

namespace ABraveFish {
unsigned int registeOpenGLTexture(unsigned char* buffer, uint32_t width, uint32_t height) {
    unsigned int texid;

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    return texid;
}
} // namespace ABraveFish
