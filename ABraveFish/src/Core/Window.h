#pragma once

#include <functional>
#include <string>

#include "GLFW/glfw3.h"

#include "Core/Application.h"

namespace ABraveFish {
struct WindowData {
    std::string  Title;
    unsigned int Width, Height;
    bool         VSync;
    float        offsetX, offsetY;
};

class Window {
public:
    Window(){};
    Window(ApplicationSpecification spec);
    void Init();
    bool IsWindowShouldClose();
    void SwapBuffers();
    void PollEvents();
public:
    inline GLFWwindow* GetWindowHandler() { return m_WindowHandle; }

private:
    ApplicationSpecification m_Specification;

    GLFWwindow* m_WindowHandle = nullptr;

    WindowData m_WindowData;
};
} // namespace ABraveFish