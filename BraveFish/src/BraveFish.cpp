
#include "Core/Application.h"
#include "Core/EntryPoint.h"

#include "Core/Layer.h"
#include "Renderer/Image.h"

#include "Core/Timer.h"
#include "Renderer/Camera.h"
#include "Renderer/Model.h"

#include <glm/glm.hpp>
#include <iostream>

#include "Core/Timer.h"
#include "include/GLFW/glfw3.h"

using namespace ABraveFish;
const int depth = 255;

glm::vec3 light_dir(0, 0, -1); // define light_dir

glm::mat4 viewport(int32_t w, int32_t h) {
    glm::mat4 m(1.0f);
    m[0][3] = w / 2.f;
    m[1][3] = h / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    return m;
}

void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, TGAImage* image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int32_t dx = x1 - x0;
    int32_t dy = y1 - y0;

    int32_t derror2 = std::abs(dy) * 2;
    int32_t error2  = 0;

    int32_t y = y0;
    for (int32_t x = x0; x <= x1; x++) {
        if (steep) {
            image->set(y, x, color);
        } else {
            image->set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

// TODO:
glm::vec3 Barycentric(glm::vec3* pts, glm::vec2 P) {
    glm::vec3 u = glm::cross(glm::vec3({pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x}),
                             glm::vec3({pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y}));
    // 面积为0的退化三角形？
    if (std::abs(u.z) < 1)
        return glm::vec3({-1, 1, 1});
    return glm::vec3({1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z});
}

void DrawTriangle(glm::vec3* pts, float* zbuffer, glm::vec2* uv, TGAImage* image, Model* model, float intensity) {
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
            if (zbuffer[int(P.x + P.y * width)] < P.z) {
                zbuffer[int(P.x + P.y * width)] = P.z;

                glm::vec2 uvP   = uv[0] * bc_screen.x + uv[1] * bc_screen.y + uv[2] * bc_screen.z;
                TGAColor  color = model->Diffuse(uvP);
                image->set(P.x, P.y, TGAColor(color.b * intensity, color.g * intensity, color.r * intensity, 255.f));
            }
        }
    }
}

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

class BraveFishLayer : public Layer {
public:
    BraveFishLayer()
        : m_Camera(45.0f, 0.1f, 100.0f)
        , m_Image(nullptr) {}

    virtual void OnUpdate(float ts) override {
        m_Camera.OnUpdate(ts);
        if (m_Image) {
            delete m_Image;
            m_Image = new TGAImage(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA);
            m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
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
            m_Model = new Model("obj/african_head.obj");
        }

        float* zbuffer = new float[m_ViewportWidth * m_ViewportHeight];
        for (int i = m_ViewportWidth * m_ViewportHeight; i--; zbuffer[i] = -std::numeric_limits<float>::max())
            ;

        glm::mat4 Cam        = m_Camera.GetView();
        glm::mat4 Projection = m_Camera.GetProjection();
        glm::mat4 ViewPort   = viewport(m_ViewportWidth, m_ViewportHeight);

        for (int32_t i = 0; i < m_Model->GetFaceCount(); i++) {
            std::vector<int32_t> face = m_Model->Face(i);
            glm::vec3            screen_coords[3];
            glm::vec3            world_coords[3];
            for (int32_t j = 0; j < 3; j++) {
                glm::vec3 v = m_Model->Vert(face[j]);
                glm::vec4 temp(Projection * Cam * glm::vec4(v.x, v.y, v.z, 1.f));
                glm::vec4 temp2  = ViewPort * temp;
                screen_coords[j] = glm::vec3(temp2.x, temp2.y, temp2.z);

                world_coords[j] = v;
            }

            glm::vec3 norm =
                glm::normalize(glm::cross(world_coords[2] - world_coords[1], (world_coords[1] - world_coords[0])));
            double intensity = glm::dot(norm, light_dir);

            if (intensity > 0) {
                // texture
                glm::vec2 uv[3];
                for (int k = 0; k < 3; k++) {
                    uv[k] = m_Model->UV(i, k);
                }

                DrawTriangle(screen_coords, zbuffer, uv, m_Image, m_Model, intensity);
            }
        }

        m_Image->flip_vertically(); // 反转y轴
        m_Image->write_tga_file("out.tga");
    }

private:
    uint32_t  m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image = nullptr;
    Model*    m_Model = nullptr;

    Camera m_Camera;

    double m_Time = 0.f;
};

ABraveFish::Application* ABraveFish::CreateApplication() {
    ABraveFish::ApplicationSpecification spec;
    spec.Name                    = "ABraveFish Soft Renderer";
    ABraveFish::Application* app = new ABraveFish::Application(spec);
    std::shared_ptr<Layer>   b   = std::make_shared<BraveFishLayer>();
    app->PushLayer(b);

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