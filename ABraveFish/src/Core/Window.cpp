
#include "Window.h"

namespace ABraveFish {
Window::Window(ApplicationSpecification spec)
    : m_Specification(spec) {}
void Window::Init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_WindowHandle =
        glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), NULL, NULL);

    glfwMakeContextCurrent(m_WindowHandle);

    glfwSetWindowUserPointer(m_WindowHandle, &m_WindowData);
    // glfwSetFramebufferSizeCallback(m_WindowHandle, framebuffer_size_callback);
}

bool Window::IsWindowShouldClose() { return glfwWindowShouldClose(m_WindowHandle); }

void Window::SwapBuffers() { glfwSwapBuffers(m_WindowHandle); }

void Window::PollEvents() { glfwPollEvents(); }

} // namespace ABraveFish