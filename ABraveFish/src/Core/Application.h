#pragma once

#include "Layer.h"

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
    Application();
    Application(ApplicationSpecification spec);
    virtual ~Application();

    static Application& Get();

    void         init();
    inline void* GetWindowHandler() const { return m_WindowHandle; }
    void         SetMenubarCallback(std::function<void()>&& callback) { m_MenubarCallback = callback; }

    void PushLayer(const std::shared_ptr<Layer>& layer) {
        m_LayerStack.emplace_back(layer);
        layer->OnAttach();
    }

    float GetTime();

    void Run();
    void Shutdown();
    void Close() {}

private:
    ApplicationSpecification m_Specification;
    GLFWwindow*              m_WindowHandle = nullptr;

    std::vector<std::shared_ptr<Layer>> m_LayerStack;

    std::function<void()> m_MenubarCallback;

private:
    float m_TimeStep      = 0.0f;
    float m_FrameTime     = 0.0f;
    float m_LastFrameTime = 0.0f;
};

// To be defined in CLIENT
Application* CreateApplication();
} // namespace ABraveFish