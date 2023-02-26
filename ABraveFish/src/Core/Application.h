#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "imgui.h"

struct GLFWwindow;

namespace ABraveFish {
struct ApplicationSpecification {
    std::string Name   = "ABraveFish App";
    uint32_t    Width  = 1600;
    uint32_t    Height = 900;
};

class Application {
public:
    Application( );

    void init();
    inline void* GetWindowHandler() const { return m_WindowHandle; }
    // virtual ~Application( );
    // void Run( );
private:
    ApplicationSpecification m_Specification;
    GLFWwindow* m_WindowHandle = nullptr;
};

// To be defined in CLIENT
Application* CreateApplication();
} // namespace ABraveFish