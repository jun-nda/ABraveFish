//#include "Camera.h"
//
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>
//
//#include "Core/Macros.h"
//#include "Core/Input.h"
//
//namespace ABraveFish {
//
//const float FOVY = TO_RADIANS(60);
//const float NEAR = 0.1f;
//const float FAR = 1000.f;
//
//Camera::Camera(glm::vec3 position, glm::vec3 target, float aspect)
//    : m_Position(position)
//    , m_Target(target)
//    , m_Aspect(aspect)
//    , m_VerticalFOV(FOVY) {}
//
//bool Camera::OnUpdate(float ts) {
//    glm::vec2 mousePos  = Input::GetMousePosition();
//    glm::vec2 delta     = (mousePos - m_LastMousePosition) * 0.002f;
//    m_LastMousePosition = mousePos;
//
//    if (!Input::IsMouseButtonDown(MouseButton::Right)) {
//        Input::SetCursorMode(CursorMode::Normal);
//        return false;
//    }
//
//    Input::SetCursorMode(CursorMode::Locked);
//
//    bool moved = false;
//
//    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
//    glm::vec3           rightDirection = glm::cross(m_ForwardDirection, upDirection);
//
//    float speed = 5.0f;
//
//    // Movement
//    if (Input::IsKeyDown(KeyCode::W)) {
//        m_Position += m_ForwardDirection * speed * ts;
//        moved = true;
//        std::cout << m_Position.x << " " << m_Position.y << " " << m_Position.z << std::endl;
//    } else if (Input::IsKeyDown(KeyCode::S)) {
//        m_Position -= m_ForwardDirection * speed * ts;
//        moved = true;
//        std::cout << m_Position.x << " " << m_Position.y << " " << m_Position.z << std::endl;
//    }
//    if (Input::IsKeyDown(KeyCode::A)) {
//        m_Position -= rightDirection * speed * ts;
//        moved = true;
//    } else if (Input::IsKeyDown(KeyCode::D)) {
//        m_Position += rightDirection * speed * ts;
//        moved = true;
//    }
//    if (Input::IsKeyDown(KeyCode::Q)) {
//        m_Position -= upDirection * speed * ts;
//        moved = true;
//    } else if (Input::IsKeyDown(KeyCode::E)) {
//        m_Position += upDirection * speed * ts;
//        moved = true;
//    }
//
//    // Rotation
//    if (delta.x != 0.0f || delta.y != 0.0f) {
//        float pitchDelta = delta.y * GetRotationSpeed();
//        float yawDelta   = delta.x * GetRotationSpeed();
//
//        glm::quat q        = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
//                                                glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
//        m_ForwardDirection = glm::rotate(q, m_ForwardDirection);
//
//        moved = true;
//    }
//
//    if (moved) {
//        RecalculateView();
//        //RecalculateRayDirections();
//    }
//
//    return moved;
//}
//
//void Camera::OnResize(uint32_t width, uint32_t height) {
//    if (width == m_ViewportWidth && height == m_ViewportHeight)
//        return;
//
//    m_ViewportWidth  = width;
//    m_ViewportHeight = height;
//
//    RecalculateProjection();
//    //RecalculateRayDirections();
//}
//
//float Camera::GetRotationSpeed() { return 0.3f; }
//
//void Camera::RecalculateProjection() {
//    //m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight,
//    //                                   m_NearClip, m_FarClip);
//    float aspect        = (float)m_ViewportWidth / (float)m_ViewportHeight;
//    float     z_range = m_FarClip - m_NearClip;
//    assert(m_VerticalFOV > 0 && aspect > 0);
//    assert(m_NearClip > 0 && m_FarClip > 0 && z_range > 0);
//    m_Projection[1][1] = 1 / (float)tan(m_VerticalFOV / 2);
//    m_Projection[0][0] = m_Projection[1][1] / aspect;
//    m_Projection[2][2] = -(m_NearClip + m_FarClip) / z_range;
//    m_Projection[2][3] = -2 * m_NearClip * m_FarClip / z_range;
//    m_Projection[3][2] = -1;
//    m_Projection[3][3] = 0;
//    m_InverseProjection = glm::inverse(m_Projection);
//}
//
//void Camera::RecalculateView() {
//    m_View        = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
//    m_InverseView = glm::inverse(m_View);
//}
//
//}
