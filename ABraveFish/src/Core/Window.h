#pragma once

#include <functional>
#include <string>

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "Core/Application.h"

namespace ABraveFish {
struct WindowData {
    std::string  Title;
    unsigned int Width, Height;
    bool         VSync;

    float curPosX, curPosY;
    float deltaX, deltaY;
    //// orbit
    // bool  IsOrbiting;
    // glm::vec2 OrbitPos;
    // glm::vec2 OrbitDelta;
    // glm::vec2 PressPos;

    // float ReleaseTime;
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
    inline GLFWwindow*       GetWindowHandler() { return m_WindowHandle; }
    inline const WindowData& GetWindowData() const { m_WindowData; }

private:
    ApplicationSpecification m_Specification;

    GLFWwindow* m_WindowHandle = nullptr;

    WindowData m_WindowData;
};
} // namespace ABraveFish