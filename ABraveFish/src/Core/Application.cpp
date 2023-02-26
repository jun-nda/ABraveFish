#include "Application.h"

#include <GLFW/glfw3.h>

namespace ABraveFish {
Application::Application()
    : m_WindowHandle(nullptr) {
    init();
}

void Application::init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_WindowHandle =
        glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), NULL, NULL);

    glfwMakeContextCurrent(m_WindowHandle);

    //glfwSetFramebufferSizeCallback(m_WindowHandle, framebuffer_size_callback);

}

} // namespace ABraveFish
