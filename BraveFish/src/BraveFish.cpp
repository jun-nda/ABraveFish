/*
 * 空间变换部分的两个坑： 1. glm矩阵乘法是反着的。 2. games101只讲了mvp，之后还要进行透视除法。
 */
#include "Core/Application.h"
#include "Core/EntryPoint.h"

#include <iostream>

#include "Core/Layer.h"
#include "Core/Timer.h"

#include "Core/Timer.h"
#include "Core/Window.h"

#include <glm/glm.hpp>
#include "Core/Macros.h"
#include "Renderer/RenderDevice.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBuffer.h"
#include "Renderer/Util.h"

using namespace ABraveFish;

const int depth = 255;

glm::vec3 light_dir(0, 0, -1); // define light_dir

struct DrawData {
    Model* _model;
    RenderBuffer* _renderbuffer;
    //Ishader*      shader;
};

void drawTriangle(DrawData ) {

}


class BraveFishLayer : public Layer {
public:
    BraveFishLayer()
        : m_Image(nullptr)
         {}

    virtual void OnAttach() override {
        for (int i = 2000 * 2000; i--; m_Zbuffer[i] = 1.f)
            ;
    }

    virtual void OnUpdate(float ts) override {
        if (m_Image) {
            delete m_Image;
            m_Image = new TGAImage(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA);
            m_Camera.update(ts);
            Render();
        }
    }

    virtual void OnUIRender() {
        bool is = true;
        Timer timer;

        ImGui::Begin("Settings");
        ImGui::Text("yujunda");
        ImGui::Text("Last render: %.3fms", m_Time);
        //ImGui::Text();

        ImGui::End();

        ImGui::Begin("Render");

        m_ViewportWidth  = ImGui::GetContentRegionAvail().x;
        m_ViewportHeight = ImGui::GetContentRegionAvail().y;

        if (!m_Image || m_ViewportWidth != m_Image->get_width() || m_ViewportHeight != m_Image->get_height()) {
            delete m_Image;
            m_Image = new TGAImage(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA);
            m_Camera.updateTransformMatrix(m_ViewportWidth, m_ViewportHeight);
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

        if (m_Zbuffer) {
            std::fill(m_Zbuffer, m_Zbuffer + 2000 * 2000, 1.f);
        }

        glm::mat4 World = m_Camera.getWorldMatrix();
        glm::mat4 ModelView  = m_Camera.getViewMatrix();
        glm::mat4 Projection = m_Camera.getPerspectiveMatrix();

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
                {-0.5f, -0.5f, 0.5f}, // bottom-left
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
                glm::vec4 world_coords[3];
                glm::vec3 screen_coords[3];

                for (int32_t j = 0; j < 3; j++) {
                    glm::vec3 vert     = vertices[i + j];
                    world_coords[j]    = World * glm::vec4(vert, 1.f);
                    glm::vec4 eye_pos  = world_coords[j] * ModelView;
                    glm::vec4 clip_pos = eye_pos * Projection;

                    glm::vec3 ndc_pos =
                        glm::vec3(clip_pos.x / clip_pos.w, clip_pos.y / clip_pos.w, clip_pos.z / clip_pos.w);
                    glm::vec3 screen_coord = viewport_transform(m_ViewportWidth, m_ViewportHeight, ndc_pos);
                    screen_coords[j]       = screen_coord;
                }
                TGAColor color = TGAColor(i / 36.f * 255, i / 36.f * 255, i / 36.f * 255, 255.f);
                DrawTriangle(screen_coords, m_Zbuffer, m_Image, color);
            }
        } else {
            for (int32_t i = 0; i < m_Model->getFaceCount(); i++) {
                std::vector<int32_t> face = m_Model->getFace(i);
                glm::vec3            screen_coords[3];
                glm::vec3            world_coords[3];
                glm::vec3            ndc_coords[3];

                float intensity[3];
                float screen_depths[3];
                for (int32_t j = 0; j < 3; j++) {
                    glm::vec3 vert      = m_Model->getVert(face[j]);
                    glm::vec4 world_pos = World * glm::vec4(vert, 1.f);
                    glm::vec4 eye_pos   = world_pos * ModelView;
                    glm::vec4 clipPos   = eye_pos * Projection;

                    glm::vec4 ndcPos =
                        glm::vec4(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w, 1.0f);
                    ndc_coords[j]          = ndcPos;
                    glm::vec3 screen_coord = viewport_transform(m_ViewportWidth, m_ViewportHeight, ndcPos);
                    screen_coords[j]       = screen_coord;

                    world_coords[j]  = world_pos;
                    screen_depths[j] = screen_coord.z;
                    intensity[j]     = glm::dot(m_Model->getNormal(i, j), light_dir);
                }

                // 背面剔除
                if (isBackFacing(ndc_coords))
                    continue;

                // texture
                glm::vec2 uv[3];
                for (int k = 0; k < 3; k++) {
                    uv[k] = m_Model->getUV(i, k);
                }

                DrawTriangle(screen_coords, m_Zbuffer, uv, m_Image, m_Model, intensity, screen_depths);
            }
        }

        m_Image->flip_vertically(); // i want to have the origin at the left bottom corner of the image
        m_Image->write_tga_file("output.tga");
    }

private:
    uint32_t  m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image = nullptr;
    Model*    m_Model = nullptr;
    float     m_Zbuffer[2000 * 2000];
    Camera    m_Camera;

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