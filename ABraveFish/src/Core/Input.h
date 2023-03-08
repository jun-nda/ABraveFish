#pragma once

#include <glm/glm.hpp>

#include "KeyCodes.h"
#include "Math.h"

namespace ABraveFish {

class Input {
public:
    static bool IsKeyDown(KeyCode keycode);
    static bool IsMouseButtonDown(MouseButton button);

    static glm::vec2 GetMousePosition();

    static void SetMouseScrollCallback();
    static void SetCursorMode(CursorMode mode);

};

} // namespace ABraveFish