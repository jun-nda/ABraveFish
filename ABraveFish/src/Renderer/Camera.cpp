#include "camera.h"

#include "Core/Application.h"
#include "Core/Macros.h"
#include "Core/Window.h"

#include "RenderDevice.h"

#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

namespace ABraveFish {

const glm::vec3 UP(0.f, 1.f, 0.f);

const float NEAR = 0.1f;
const float FAR  = 1000.f;
const float FOV  = TO_RADIANS(60);

/* Arcbal */
int32_t last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int32_t arcball_on = false;

glm::vec3 get_arcball_vector(int x, int y, int width, int height);
void      onMouse(GLFWwindow* window, int button, int action, int mods);
void      onMotion(GLFWwindow* window, double x, double y);

/*
 * https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
 */

Camera::Camera() { initMouseCallBack(); }

void Camera::update(float ts) {
    /* onIdle() */
    if (cur_mx != last_mx || cur_my != last_my) {
        glm::vec3 va    = get_arcball_vector(last_mx, last_my, m_Width, m_Height);
        glm::vec3 vb    = get_arcball_vector(cur_mx, cur_my, m_Width, m_Height);
        float     angle = acos(std::min(1.0f, glm::dot(va, vb))) * 0.2;
        // std::cout << "onUpdate:" << angle << std::endl;
        glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
        glm::mat3 camera2object        = glm::inverse(glm::mat3(m_ViewMatrix) * glm::mat3(m_WorldMatrix));
        glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
        // std::cout << "axis:" << axis_in_camera_coord.x << " " << axis_in_camera_coord.y << " "
        //          << axis_in_camera_coord.z << std::endl;

        m_WorldMatrix = glm::rotate(m_WorldMatrix, glm::degrees(angle), axis_in_object_coord);
        last_mx       = cur_mx;
        last_my       = cur_my;
    }
}

void Camera::updateTransformMatrix(int32_t width, int32_t height) {
    m_Width  = width;
    m_Height = height;

    m_ViewMatrix        = lookat(m_Pos, m_Target, UP);
    m_PerspectiveMatrix = perspective(FOV, m_Width / m_Height, NEAR, FAR);
}

void Camera::initMouseCallBack() {
    GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
    glfwSetMouseButtonCallback(window, onMouse);
    glfwSetCursorPosCallback(window, onMotion);
}

/**
 * Get a normalized vector from the center of the virtual ball O to a
 * point P on the virtual ball surface, such that P is aligned on
 * screen's (X,Y) coordinates.  If (X,Y) is too far away from the
 * sphere, return the nearest point on the virtual ball surface.
 */
glm::vec3 get_arcball_vector(int x, int y, int width, int height) {
    // viewport reverse
    glm::vec3 P      = glm::vec3(1.0 * x / width * 2 - 1.0, 1.0 * y / height * 2 - 1.0, 0);
    P.y              = -P.y;
    float OP_squared = P.x * P.x + P.y * P.y;
    if (OP_squared <= 1 * 1)
        P.z = sqrt(1 * 1 - OP_squared); // Pythagoras
    else
        P = glm::normalize(P); // nearest point
    return P;
}

void onMouse(GLFWwindow* window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        arcball_on = true;
        last_mx = cur_mx = x;
        last_my = cur_my = y;
        // std::cout << "onMouse" << std::endl;
    } else {
        arcball_on = false;
    }
}

void onMotion(GLFWwindow* window, double x, double y) {
    if (arcball_on) { // if left button is pressed
        cur_mx = x;
        cur_my = y;
    }
}

} // namespace ABraveFish