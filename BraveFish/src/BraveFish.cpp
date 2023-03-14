/*
 * 空间变换部分的两个坑： 1. glm矩阵乘法是反着的。 2. games101只讲了mvp，之后还要进行透视除法。
 */
#include "Core/Application.h"
#include "Core/EntryPoint.h"

#include <iostream>

#include "Core/Layer.h"
#include "Core/Timer.h"

#include "Core/Timer.h"
#include "GLFW/glfw3.h"

#define YUJUNDA_ORIGIN

#if defined YUJUNDA_ORIGIN
#include <glm/glm.hpp>
#include "Core/Macros.h"
#include "Core/Window.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderDevice.h"
#include "glm/gtc/matrix_transform.hpp"
#else
#include "Renderer/Camera.h"
#include "model.h"
#include "tgaimage.h"
#include "vector.h"

#endif

using namespace ABraveFish;

const int depth = 255;

#if defined YUJUNDA_ORIGIN

/* Global */
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int arcball_on = false;

void onMouse(GLFWwindow* window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        arcball_on = true;
        last_mx = cur_mx = x;
        last_my = cur_my = y;
        std::cout << "onMouse" << std::endl;
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

glm::vec3 light_dir(0, 0, -1); // define light_dir
glm::vec3 eye(1, 1, 3);
glm::vec3 center(0, 0, 0);

glm::vec3 Barycentric(std::vector<glm::vec3> pts, glm::vec2 P) {
    glm::vec3 u = glm::cross(glm::vec3({pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x}),
                             glm::vec3({pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y}));
    // 面积为0的退化三角形？
    if (std::abs(u.z) < 1)
        return glm::vec3({-1, 1, 1});
    return glm::vec3({1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z});
}

void DrawTriangle(std::vector<glm::vec3>& pts, int32_t* zbuffer, TGAImage* image, TGAColor color) {
    int32_t width  = image->get_width();
    int32_t height = image->get_height();

    // Attention: box must be int
    int32_t bboxmin[2] = {width - 1, height - 1};
    int32_t bboxmax[2] = {0, 0};
    int32_t clamp[2]   = {width - 1, image->get_height() - 1};
    for (int32_t i = 0; i < 3; i++) {
        bboxmin[0] = std::max(0, std::min(bboxmin[0], (int32_t)pts[i].x));
        bboxmin[1] = std::max(0, std::min(bboxmin[1], (int32_t)pts[i].y));

        bboxmax[0] = std::min(clamp[0], std::max(bboxmax[0], (int32_t)pts[i].x));
        bboxmax[1] = std::min(clamp[1], std::max(bboxmax[1], (int32_t)pts[i].y));
    }
    glm::vec3 P(1.f);

    for (P.x = bboxmin[0]; P.x <= bboxmax[0]; P.x++) {
        for (P.y = bboxmin[1]; P.y <= bboxmax[1]; P.y++) {
            glm::vec3 bc_screen = Barycentric(pts, glm::vec2(P.x, P.y));
            if (bc_screen.x < 0.f || bc_screen.y < 0.f || bc_screen.z < 0.f)
                continue;
            int32_t idx = P.x + P.y * width;
            if (zbuffer[idx] > P.z) {
                zbuffer[idx] = P.z;

                // glm::vec2 uvP   = uv[0] * bc_screen.x + uv[1] * bc_screen.y + uv[2] * bc_screen.z;
                // TGAColor  color = model->Diffuse(uvP);
                image->set(P.x, P.y, color);
                // image->set(P.x, P.y, TGAColor(255.f * intensity, 255.f * intensity, 255.f * intensity, 255.f));
            }
        }
    }
}

#else
Vector3f light_dir(0, 0, -1); // define light_dir
Vector3f eye(1, 1, 3);
Vector3f center(0, 0, 0);

void PrintMatrix(Matrix4x4 m) {
    std::cout << m[0] << std::endl;
    std::cout << m[1] << std::endl;
    std::cout << m[2] << std::endl;
    std::cout << m[3] << std::endl;
}

Vector3f barycentric(Vector2f* pts, Vector2f P) {
    Vector3f s[2];
    for (int i = 2; i--;) {
        s[i][0] = pts[2][i] - pts[0][i];
        s[i][1] = pts[1][i] - pts[0][i];
        s[i][2] = pts[0][i] - P[i];
    }
    Vector3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vector3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vector3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

float interpolate_depth(float* screen_depths, Vector3f weights) {
    Vector3f screen_depth;
    for (size_t i = 0; i < 3; i++) {
        screen_depth[i] = screen_depths[i];
    }

    return screen_depth * weights;
}

bool is_back_facing(Vector3f* ndc_coords) {
    Vector3f a           = ndc_coords[0];
    Vector3f b           = ndc_coords[1];
    Vector3f c           = ndc_coords[2];
    float    signed_area = a.x * b.y - a.y * b.x + b.x * c.y - b.y * c.x + c.x * a.y - c.y * a.x;
    return signed_area <= 0;
}

void triangle(Vector4f* clip_pos, float* itensity, Vector2f* uv, TGAImage* image, int* zbuffer, Model* model) {
    // 齐次除法 / 透视除法 (homogeneous division / perspective division)
    Vector3f ndc_coords[3];
    for (int i = 0; i < 3; i++)
        ndc_coords[i] = proj<3>(clip_pos[i] / clip_pos[i][3]);

    // 背面剔除
    if (is_back_facing(ndc_coords))
        return;

    int32_t width  = image->get_width();
    int32_t height = image->get_height();

    // 屏幕映射
    Vector2f screen_coords[3];
    float    screen_depth[3];
    for (int i = 0; i < 3; i++) {
        Vector3f win_coord = viewport_transform(width, height, ndc_coords[i]);
        screen_coords[i]   = Vector2f(win_coord.x, win_coord.y);
        screen_depth[i]    = win_coord.z;
    }

    Vector2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vector2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vector2f clamp(width - 1, height - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], screen_coords[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], screen_coords[i][j]));
        }
    }

    Vector2i P;

    for (P.x = bboxmin[0]; P.x <= bboxmax[0]; P.x++) {
        for (P.y = bboxmin[1]; P.y <= bboxmax[1]; P.y++) {
            // 重心坐标
            Vector3f barycentric_weights = barycentric(screen_coords, Vector2f(P.x, P.y));
            if (barycentric_weights.x < 0.f || barycentric_weights.y < 0.f || barycentric_weights.z < 0.f)
                continue;

            // 深度插值
            float frag_depth = interpolate_depth(screen_depth, barycentric_weights);

            // 深度测试
            // 近~远 0~1
            int32_t idx = P.x + P.y * width;
            if (zbuffer[idx] > frag_depth) {
                zbuffer[idx] = frag_depth;

                float intense = itensity[0] * barycentric_weights.x + itensity[1] * barycentric_weights.y +
                                itensity[2] * barycentric_weights.z;
                Vector2f uvP =
                    uv[0] * barycentric_weights.x + uv[1] * barycentric_weights.y + uv[2] * barycentric_weights.z;

                TGAColor color = model->diffuse(uvP);
                image->set(P.x, P.y, TGAColor(color[0], color[1], color[2], 255.f));
                // image->set(P.x, P.y, TGAColor(255.f * intensity, 255.f * intensity, 255.f * intensity, 255.f));
            }
        }
    }
}

void triangle(Vector4f* clip_pos, TGAImage* image, int* zbuffer, TGAColor color) {
    // 齐次除法 / 透视除法 (homogeneous division / perspective division)
    Vector3f ndc_coords[3];
    for (int i = 0; i < 3; i++)
        ndc_coords[i] = proj<3>(clip_pos[i] / clip_pos[i][3]);

    //背面剔除
    if (is_back_facing(ndc_coords))
        return;

    int32_t width  = image->get_width();
    int32_t height = image->get_height();

    // 屏幕映射
    Vector2f screen_coords[3];
    float    screen_depth[3];
    for (int i = 0; i < 3; i++) {
        Vector3f win_coord = viewport_transform(width, height, ndc_coords[i]);
        // std::cout << win_coord.x << " " << win_coord.y << std::endl;
        screen_coords[i] = Vector2f(win_coord.x, win_coord.y);
        screen_depth[i]  = win_coord.z;
    }

    float recip_w[3];
    /* reciprocals of w */
    for (int i = 0; i < 3; i++) {
        recip_w[i] = 1 / clip_pos[i][3];
    }

    Vector2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vector2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vector2f clamp(width - 1, height - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], screen_coords[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], screen_coords[i][j]));
        }
    }

    Vector2i P;

    for (P.x = bboxmin[0]; P.x <= bboxmax[0]; P.x++) {
        for (P.y = bboxmin[1]; P.y <= bboxmax[1]; P.y++) {
            // 重心坐标
            Vector3f barycentric_weights = barycentric(screen_coords, Vector2f(P.x, P.y));
            if (barycentric_weights.x < 0.f || barycentric_weights.y < 0.f || barycentric_weights.z < 0.f)
                continue;

            // 深度插值
            float frag_depth = interpolate_depth(screen_depth, barycentric_weights);

            // 深度测试
            int32_t idx = P.x + P.y * width;
            if (zbuffer[idx] > frag_depth) {
                zbuffer[idx] = frag_depth;

                image->set(P.x, P.y, color);
                // image->set(P.x, P.y, TGAColor(255.f * intensity, 255.f * intensity, 255.f * intensity, 255.f));
            }
        }
    }
}

#endif

unsigned int registeOpenGLTexture(unsigned char* buffer, uint32_t width, uint32_t height) {
    unsigned int texid;

    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    return texid;
}

// const glm::vec3 center(0.f, 0.f, 0.f);
// const glm::vec3 up(0.f, 1.f, 0.f);
// const float     radius       = 5.f;
// const float     minRadius    = 1.f;
// const float     azimuthAngle = TO_RADIANS(0.f);
// const float     polarAngle   = TO_RADIANS(0.f);

class BraveFishLayer : public Layer {
public:
    BraveFishLayer()
        : m_Image(nullptr)
        , m_Camera(TO_RADIANS(45), 0.1f, 1000.f) {}

    virtual void OnAttach() override {
        GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow()->GetWindowHandler();
        glfwSetMouseButtonCallback(window, onMouse);
        glfwSetCursorPosCallback(window, onMotion);
    }

    virtual void OnUpdate(float ts) override {
        if (m_Image) {
            delete m_Image;
            m_Image = new TGAImage(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA);
            m_Camera.OnUpdate(ts);
            Render();
        }

        // m_Renderer.ResetFrameIndex();
    }

    virtual void OnUIRender() {
        bool is = true;
        // ImGui::ShowDemoWindow(&is);
        Timer timer;

        ImGui::Begin("Settings");
        ImGui::Text("yujunda");
        ImGui::Text("Last render: %.3fms", m_Time);

        ImGui::End();

        ImGui::Begin("Render");

        m_ViewportWidth  = ImGui::GetContentRegionAvail().x;
        m_ViewportHeight = ImGui::GetContentRegionAvail().y;

        if (!m_Image || m_ViewportWidth != m_Image->get_width() || m_ViewportHeight != m_Image->get_height()) {
            delete m_Image;
            m_Image = new TGAImage(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA);
            m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
            Render();
        }

        if (m_Image)
            ImGui::Image((ImTextureID)registeOpenGLTexture(m_Image->buffer(), m_ViewportWidth, m_ViewportHeight),
                         ImVec2(m_ViewportWidth, m_ViewportHeight));

        ImGui::End();

        m_Time = timer.ElapsedMillis();
    }

    void Render() {
        if (!m_Model) {
            m_Model = new Model("obj/african_head/african_head.obj");
        }

        // if (!m_Zbuffer) {
        //    delete[] m_Zbuffer;
        //    m_Zbuffer = nullptr;
        m_Zbuffer = new int32_t[m_ViewportWidth * m_ViewportHeight];
        for (int i = m_ViewportWidth * m_ViewportHeight; i--; m_Zbuffer[i] = 10000.f)
            ;
            //}

#if defined YUJUNDA_ORIGIN
        glm::mat4 ModelView  = lookat(eye, center, glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 Projection = perspective(TO_RADIANS(60.f), m_ViewportWidth / m_ViewportWidth, 0.1, 100.f);
        // glm::mat4 ModelView  = m_Camera.GetView();
        // glm::mat4 Projection = m_Camera.GetProjection();

        /* onIdle() */
        if (cur_mx != last_mx || cur_my != last_my) {
            glm::vec3 va                   = get_arcball_vector(last_mx, last_my, m_ViewportWidth, m_ViewportWidth);
            glm::vec3 vb                   = get_arcball_vector(cur_mx, cur_my, m_ViewportWidth, m_ViewportWidth);
            float     angle                = acos(std::min(1.0f, glm::dot(va, vb))) * 0.2;
            std::cout << "onUpdate:" << angle << std::endl;
            glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
            glm::mat3 camera2object        = glm::inverse(glm::mat3(ModelView) * glm::mat3(m_World));
            glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
            m_World                        = glm::rotate(m_World, glm::degrees(angle), axis_in_object_coord);
            last_mx                        = cur_mx;
            last_my                        = cur_my;
        }

        bool isCube = false;
        if (isCube) {
            glm::vec3 vertices[] = {
                // Back face
                {-0.5f, -0.5f, -0.5f}, // Bottom-left
                {0.5f, 0.5f, -0.5f},   // top-right
                {0.5f, -0.5f, -0.5f},  // bottom-right
                {0.5f, 0.5f, -0.5f},   // top-right
                {-0.5f, -0.5f, -0.5f}, // bottom-left
                {-0.5f, 0.5f, -0.5f},  // top-left

                // Front face
                {0.5f, -0.5f, 0.5},   // bottom-left
                {0.5f, -0.5f, 0.5f},  // bottom-right
                {0.5f, 0.5f, 0.5f},   // top-right
                {0.5f, 0.5f, 0.5f},   // top-right
                {-0.5f, 0.5f, 0.5f},  // top-left
                {-0.5f, -0.5f, 0.5f}, // bottom-left

                // Left face
                {-0.5f, 0.5f, 0.5f},   // top-right
                {-0.5f, 0.5f, -0.5f},  // top-left
                {-0.5f, -0.5f, -0.5f}, // bottom-left
                {-0.5f, -0.5f, -0.5f}, // bottom-left
                {-0.5f, -0.5f, 0.5f},  // bottom-right
                {-0.5f, 0.5f, 0.5f},   // top-right

                // Right face
                {0.5f, 0.5f, 0.5f},   // top-left
                {0.5f, -0.5f, -0.5f}, // bottom-right
                {0.5f, 0.5f, -0.5f},  // top-right
                {0.5f, -0.5f, -0.5f}, // bottom-right
                {0.5f, 0.5f, 0.5f},   // top-left
                {0.5f, -0.5f, 0.5f},  // bottom-left

                // Bottom face
                {-0.5f, -0.5f, -0.5f}, // top-right
                {0.5f, -0.5f, -0.5f},  // top-left
                {0.5f, -0.5f, 0.5f},   // bottom-left
                {0.5f, -0.5f, 0.5f},   // bottom-left
                {-0.5f, -0.5f, 0.5f},  // bottom-right
                {-0.5f, -0.5f, -0.5f}, // top-right

                // Top face
                {-0.5f, 0.5f, -0.5f}, // top-left
                {0.5f, 0.5f, 0.5f},   // bottom-right
                {0.5f, 0.5f, -0.5f},  // top-right
                {0.5f, 0.5f, 0.5f},   // bottom-right
                {-0.5f, 0.5f, -0.5f}, // top-left
                {-0.5f, 0.5f, 0.5f},  // bottom-left
            };

            for (int32_t i = 0; i < 36; i += 3) {
                std::vector<glm::vec3> world_coords;
                std::vector<glm::vec3> screen_coords;

                for (int32_t j = 0; j < 3; j++) {
                    world_coords.emplace_back(vertices[i + j]);
                    glm::vec4 eye_pos =
                        glm::vec4(world_coords[j].x, world_coords[j].y, world_coords[j].z, 1.f) * ModelView;
                    glm::vec4 clip_pos = eye_pos * Projection;

                    PrintMatrix(Projection);
                    PrintMatrix(ModelView);
                    std::cout << world_coords[j].x << " " << world_coords[j].y << " " << world_coords[j].z << std::endl;
                    std::cout << eye_pos.x << " " << eye_pos.y << " " << eye_pos.z << " " << eye_pos.w << std::endl;
                    std::cout << clip_pos.x << " " << clip_pos.y << " " << clip_pos.z << " " << clip_pos.w << std::endl;

                    glm::vec3 ndc_pos =
                        glm::vec3(clip_pos.x / clip_pos.w, clip_pos.y / clip_pos.w, clip_pos.z / clip_pos.w);
                    glm::vec3 screen_coord = viewport_transform(m_ViewportWidth, m_ViewportHeight, ndc_pos);
                    screen_coords.emplace_back(screen_coord);
                }
                TGAColor color = TGAColor(i / 36.f * 255, i / 36.f * 255, i / 36.f * 255, 255.f);
                DrawTriangle(screen_coords, m_Zbuffer, m_Image, color);
            }
        } else {
            for (int32_t i = 0; i < m_Model->GetFaceCount(); i++) {
                std::vector<int32_t> face = m_Model->Face(i);
                glm::vec3            screen_coords[3];
                glm::vec3            world_coords[3];
                glm::vec3            ndc_coords[3];
                float                screen_depths[3];
                for (int32_t j = 0; j < 3; j++) {
                    glm::vec3 vert      = m_Model->Vert(face[j]);
                    glm::vec4 world_pos = m_World * glm::vec4(vert.x, vert.y, vert.z, 1.f);
                    glm::vec4 eye_pos   = world_pos * ModelView;
                    glm::vec4 clipPos   = eye_pos * Projection;

                    glm::vec4 ndcPos =
                        glm::vec4(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w, 1.0f);
                    ndc_coords[j]          = ndcPos;
                    glm::vec3 screen_coord = viewport_transform(m_ViewportWidth, m_ViewportHeight, ndcPos);
                    screen_coords[j]       = screen_coord;

                    world_coords[j]  = world_pos;
                    screen_depths[j] = screen_coord.z;
                }

                // 背面剔除
                if (isBackFacing(ndc_coords))
                    continue;

                glm::vec3 norm =
                    glm::normalize(glm::cross(world_coords[2] - world_coords[1], (world_coords[1] - world_coords[0])));
                double intensity = glm::dot(norm, light_dir);

                if (intensity > 0) {
                    // texture
                    glm::vec2 uv[3];
                    for (int k = 0; k < 3; k++) {
                        uv[k] = m_Model->UV(i, k);
                    }

                    DrawTriangle(screen_coords, m_Zbuffer, uv, m_Image, m_Model, intensity, screen_depths);
                }
            }
        }
#else
        Matrix4x4 ModelView  = lookat(eye, center, Vector3f(0, 1, 0));
        Matrix4x4 Projection = Matrix4x4::identity();
        Projection           = perspective(TO_RADIANS(60), m_ViewportWidth / m_ViewportHeight, 0.1f, 100.f);
        // PrintMatrix(ModelView);
        bool isCube = false;
        if (isCube) {
            Vector3f vertices[] = {
                // Back face
                {-0.5f, -0.5f, -0.5f}, // Bottom-left
                {0.5f, 0.5f, -0.5f},   // top-right
                {0.5f, -0.5f, -0.5f},  // bottom-right
                {0.5f, 0.5f, -0.5f},   // top-right
                {-0.5f, -0.5f, -0.5f}, // bottom-left
                {-0.5f, 0.5f, -0.5f},  // top-left

                // Front face
                {0.5f, -0.5f, 0.5},   // bottom-left
                {0.5f, -0.5f, 0.5f},  // bottom-right
                {0.5f, 0.5f, 0.5f},   // top-right
                {0.5f, 0.5f, 0.5f},   // top-right
                {-0.5f, 0.5f, 0.5f},  // top-left
                {-0.5f, -0.5f, 0.5f}, // bottom-left

                // Left face
                {-0.5f, 0.5f, 0.5f},   // top-right
                {-0.5f, 0.5f, -0.5f},  // top-left
                {-0.5f, -0.5f, -0.5f}, // bottom-left
                {-0.5f, -0.5f, -0.5f}, // bottom-left
                {-0.5f, -0.5f, 0.5f},  // bottom-right
                {-0.5f, 0.5f, 0.5f},   // top-right

                // Right face
                {0.5f, 0.5f, 0.5f},   // top-left
                {0.5f, -0.5f, -0.5f}, // bottom-right
                {0.5f, 0.5f, -0.5f},  // top-right
                {0.5f, -0.5f, -0.5f}, // bottom-right
                {0.5f, 0.5f, 0.5f},   // top-left
                {0.5f, -0.5f, 0.5f},  // bottom-left

                // Bottom face
                {-0.5f, -0.5f, -0.5f}, // top-right
                {0.5f, -0.5f, -0.5f},  // top-left
                {0.5f, -0.5f, 0.5f},   // bottom-left
                {0.5f, -0.5f, 0.5f},   // bottom-left
                {-0.5f, -0.5f, 0.5f},  // bottom-right
                {-0.5f, -0.5f, -0.5f}, // top-right

                // Top face
                {-0.5f, 0.5f, -0.5f}, // top-left
                {0.5f, 0.5f, 0.5f},   // bottom-right
                {0.5f, 0.5f, -0.5f},  // top-right
                {0.5f, 0.5f, 0.5f},   // bottom-right
                {-0.5f, 0.5f, -0.5f}, // top-left
                {-0.5f, 0.5f, 0.5f},  // bottom-left
            };

            for (int32_t i = 0; i < 36; i += 3) {
                Vector4f clip_pos[3];
                Vector3f world_coords[3];
                Vector4f eye_pos[3];
                for (int32_t j = 0; j < 3; j++) {
                    world_coords[j] = vertices[i + j];
                    eye_pos[j]      = ModelView * embed<4>(world_coords[j], 1.f);
                    clip_pos[j]     = Projection * eye_pos[j];
                    // PrintMatrix(Projection);
                    // PrintMatrix(ModelView);
                    // std::cout << embed<4>(world_coords[j], 1.f) << std::endl;
                    // std::cout << eye_pos[j] << std::endl;
                    // std::cout << clip_pos[j] << std::endl;
                }
                TGAColor color = TGAColor(i / 36.f * 255, i / 36.f * 255, i / 36.f * 255, 255.f);
                // if ()
                std::cout << (float)color[0] << " " << (float)color[1] << " " << (float)color[2] << " "
                          << (float)color[3] << std::endl;
                triangle(clip_pos, m_Image, m_Zbuffer, color);
            }
        } else {
            for (int i = 0; i < m_Model->nfaces(); i++) {
                std::vector<int32_t> face = m_Model->face(i);
                Vector4f             clip_pos[3];
                Vector3f             world_coords[3];
                float                intensity[3];
                for (int j = 0; j < 3; j++) {
                    Vector3f v      = m_Model->vert(face[j]);
                    clip_pos[j]     = Vector4f(Projection * ModelView * embed<4>(v));
                    world_coords[j] = v;
                    intensity[j]    = m_Model->normal(i, j) * light_dir;
                }
                Vector2f uv[3];
                for (int k = 0; k < 3; k++) {
                    uv[k] = m_Model->uv(i, k);
                }
                triangle(clip_pos, intensity, uv, m_Image, m_Zbuffer, m_Model);
            }
        }
#endif

        m_Image->flip_vertically(); // i want to have the origin at the left bottom corner of the image
        m_Image->write_tga_file("output.tga");
    }

private:
    uint32_t  m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image   = nullptr;
    Model*    m_Model   = nullptr;
    int32_t*  m_Zbuffer = nullptr;
    Camera    m_Camera;

    glm::mat4 m_World = glm::mat4(1.f);

    double m_Time = 0.f;
};

ABraveFish::Application* ABraveFish::CreateApplication() {
    ABraveFish::ApplicationSpecification spec;
    spec.Name                             = "ABraveFish Soft Renderer";
    ABraveFish::Application* app          = new ABraveFish::Application(spec);
    std::shared_ptr<Layer>   softRenderer = std::make_shared<BraveFishLayer>();
    app->PushLayer(softRenderer);

    //
    app->SetMenubarCallback([app]() {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                app->Close();
            }
            ImGui::EndMenu();
        }
    });

    return app;
}