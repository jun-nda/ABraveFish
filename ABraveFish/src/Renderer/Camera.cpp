#include "camera.h"

#include "Core/Application.h"
#include "Core/Window.h"

#include "RenderDevice.h"

#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

namespace ABraveFish {

glm::vec2 orbit_pos;
glm::vec2 orbit_delta;
glm::vec2 fv_delta;
glm::vec2 fv_pos;

// for mouse wheel
float wheel_delta;

void onMouse(GLFWwindow* window, int button, int action, int mods);
void onMotion(GLFWwindow* window, double x, double y);
void onScroll(GLFWwindow* window, double x, double y);

Camera::Camera(glm::vec3 e, glm::vec3 t, glm::vec3 up)
    : eye(e)
    , target(t)
    , up(up) {
    GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();

    glfwSetMouseButtonCallback(window, onMouse);
    glfwSetCursorPosCallback(window, onMotion);
    glfwSetScrollCallback(window, onScroll);
}

Camera::~Camera() {}

void updata_camera_pos(Camera& camera) {
    GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();

    glm::vec3 from_target = camera.eye - camera.target; // vector point from target to camera's position
    float     radius      = glm::length(from_target);

    float phi = (float)atan2(from_target[0],
                             from_target[2]); // azimuth angle(·½Î»½Ç), angle between from_target and z-axis£¬[-pi, pi]
    float theta =
        (float)acos(from_target[1] / radius); // zenith angle(Ìì¶¥½Ç), angle between from_target and y-axis, [0, pi]


    float x_delta = orbit_delta[0] / camera.width;
    float y_delta = orbit_delta[1] / camera.height;

    // for mouse wheel
    radius *= (float)pow(0.95, wheel_delta);

    float factor = 1.5 * PI;
    // for mouse left button
    phi += x_delta * factor;
    theta += y_delta * factor;
    if (theta > PI)
        theta = PI - EPSILON * 100;
    if (theta < 0)
        theta = EPSILON * 100;

    camera.eye[0] = camera.target[0] + radius * sin(phi) * sin(theta);
    camera.eye[1] = camera.target[1] + radius * cos(theta);
    camera.eye[2] = camera.target[2] + radius * sin(theta) * cos(phi);

    // for mouse right button
    factor         = radius * (float)tan(60.0 / 360 * PI) * 2.2;
    x_delta        = fv_delta[0] / camera.width;
    y_delta        = fv_delta[1] / camera.height;
    glm::vec3 left = x_delta * factor * camera.x;
    glm::vec3 up   = y_delta * factor * camera.y;

    camera.eye += (left - up);
    camera.target += (left - up);
}

void handle_mouse_events(Camera& camera) {
    GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();

    if (glfwGetMouseButton(window, (int)GLFW_MOUSE_BUTTON_LEFT)) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        glm::vec2 cur_pos(x, y);
        //std::cout << orbit_pos.x <<  " " << orbit_pos.y << std::endl;
        orbit_delta = orbit_pos - cur_pos;
        orbit_pos   = cur_pos;
    }

    if (glfwGetMouseButton(window, (int)GLFW_MOUSE_BUTTON_RIGHT)) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        glm::vec2 cur_pos(x, y);

        fv_delta = fv_pos - cur_pos;
        fv_pos   = cur_pos;
    }

    updata_camera_pos(camera);
}

void handle_key_events(Camera& camera) {
    GLFWwindow* window   = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
    float       distance = glm::length(camera.target - camera.eye);

    glm::vec2 windowSize(camera.width, camera.height);
    if (isKeyDown(KeyCode::W)) {
        camera.eye += -10.0f / (camera.z * distance * (float)camera.width);
    }
    if (isKeyDown(KeyCode::S)) {
        camera.eye += 0.05f * camera.z;
    }
    if (isKeyDown(KeyCode::Up) || isKeyDown(KeyCode::Q)) {
        camera.eye += 0.05f * camera.y;
        camera.target += 0.05f * camera.y;
    }
    if (isKeyDown(KeyCode::Down) || isKeyDown(KeyCode::E)) {
        camera.eye += -0.05f * camera.y;
        camera.target += -0.05f * camera.y;
    }
    if (isKeyDown(KeyCode::Left) || isKeyDown(KeyCode::A)) {
        camera.eye += -0.05f * camera.x;
        camera.target += -0.05f * camera.x;
    }
    if (isKeyDown(KeyCode::Right) || isKeyDown(KeyCode::D)) {
        camera.eye += 0.05f * camera.x;
        camera.target += 0.05f * camera.x;
    }
}

void handle_events(Camera& camera) {
    // calculate camera axis
    camera.z = glm::normalize(camera.eye - camera.target);
    camera.x = glm::normalize(cross(camera.up, camera.z));
    camera.y = glm::normalize(cross(camera.z, camera.x));

    // mouse and keyboard events
    handle_mouse_events(camera);
    handle_key_events(camera);
}

void reset_camera() {
    // reset mouse information
    wheel_delta = 0;
    orbit_delta = glm::vec2(0, 0);
    fv_delta    = glm::vec2(0, 0);
}

void update_viewSize(Camera& camera, float width, float height) { 
    camera.width = width;
    camera.height = height;
}

bool isKeyDown(KeyCode keycode) {
    GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
    int         state  = glfwGetKey(window, (int)keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}


void onMouse(GLFWwindow* window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    orbit_pos = glm::vec2(x, y);
    fv_pos    = glm::vec2(x, y);
}

void onMotion(GLFWwindow* window, double x, double y) {
    //if (arcball_on) { // if left button is pressed
    //    cur_mx = x;
    //    cur_my = y;
    //}
}

void onScroll(GLFWwindow* window, double x, double y) {
    wheel_delta -= (float)y/6.f;
    //if (wheel_delta < 1.0f)
    //    wheel_delta = 1.0f;
    //if (wheel_delta > 89.0f)
    //    wheel_delta = 89.0f;
}

} // namespace ABraveFish