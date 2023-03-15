#pragma once

#include "glm/glm.hpp"

namespace ABraveFish {
class Camera {
public:
    Camera();
    void update(float ts);
    void updateTransformMatrix(int32_t width, int32_t height);

    const glm::mat4 getWorldMatrix() const { return m_WorldMatrix; }
    const glm::mat4 getViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4 getPerspectiveMatrix() const { return m_PerspectiveMatrix; }

private:
    void initMouseCallBack();

private:
    glm::vec3 m_Pos    = glm::vec3(0.f, 0.f, 3.f);
    glm::vec3 m_Target = glm::vec3(0.f, 0.f, -1.f);

    int32_t m_Width = 0;
    int32_t m_Height = 0;

    glm::mat4 m_WorldMatrix       = glm::mat4(1.f);
    glm::mat4 m_ViewMatrix        = glm::mat4(1.f);
    glm::mat4 m_PerspectiveMatrix = glm::mat4(1.f);
};
} // namespace ABraveFish
