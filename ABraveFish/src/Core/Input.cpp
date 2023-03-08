#include "Input.h"

#include "Application.h"
#include "Window.h"
#include <GLFW/glfw3.h>

namespace ABraveFish {

bool Input::IsKeyDown(KeyCode keycode) {
    GLFWwindow* windowHandle = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
    int         state        = glfwGetKey(windowHandle, (int)keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonDown(MouseButton button) {
    GLFWwindow* windowHandle = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
    int         state        = glfwGetMouseButton(windowHandle, (int)button);
    return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePosition() {
    GLFWwindow* windowHandle = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();

    double x, y;
    glfwGetCursorPos(windowHandle, &x, &y);
    return {(float)x, (float)y};
}

void Input::SetMouseScrollCallback() {
    GLFWwindow* windowHandle = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
    
    glfwSetScrollCallback(windowHandle, [](GLFWwindow* window, double xoffset, double yoffset) { 
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        data.offsetX = xoffset;
        data.offsetY = yoffset;
        });
}


void Input::SetCursorMode(CursorMode mode) {
    GLFWwindow* windowHandle = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
    glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
}

} // namespace Walnut