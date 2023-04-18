/*
 * 空间变换部分的两个坑： 1. glm矩阵乘法是反着的。 2. games101只讲了mvp，之后还要进行透视除法。
 */
/*
 * TODO:
 * 1. tgaimage to renderbuffer
 *
 */

#include "Core/Application.h"
#include "Core/EntryPoint.h"

#include <iostream>
#include <memory>

#include "Core/Layer.h"
#include "Core/Timer.h"

#include "Core/Timer.h"

#include <glm/glm.hpp>
#include "Core/Macros.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBuffer.h"
#include "Renderer/RenderDevice.h"
#include "Renderer/Shader.h"
#include "Renderer/Util.h"
using namespace ABraveFish;

const int depth = 255;

glm::vec3 light_dir(2, 2, 2); // define light_dir

class BraveFishLayer : public Layer {
public:
    BraveFishLayer()
        : m_Image(nullptr) {}

    virtual void OnAttach() override {
        for (int i = 1600 * 900; i--; m_Zbuffer[i] = 1.f)
            ;
        m_Models[0] = new Model("obj/fuhua/fuhuabody.obj");
        m_Models[1] = new Model("obj/fuhua/fuhuahair.obj");
        m_Models[2] = new Model("obj/fuhua/fuhuaface.obj");
        m_Models[3] = new Model("obj/fuhua/fuhuacloak.obj");
    }

    virtual void OnUpdate(float ts) override {
        if (m_Image) {
            Timer timer;

            Render();
            m_Time = timer.ElapsedMillis();
        }
    }

    virtual void OnUIRender() {
        bool is = true;

        ImGui::Begin("Settings");
        ImGui::Text("yujunda");
        ImGui::Text("Last render: %.3fms", m_Time);
        ImGui::Text("FPS: %.f", 1000.f / m_Time);

        ImGui::Text("CameraPos: %.3f, %.3f, %.3f", m_Camera.eye.x, m_Camera.eye.y, m_Camera.eye.z);

        ImGui::End();

        ImGui::Begin("Render");

        m_ViewportWidth  = ImGui::GetContentRegionAvail().x;
        m_ViewportHeight = ImGui::GetContentRegionAvail().y;

        if (!m_Image || m_ViewportWidth != m_Image->get_width() || m_ViewportHeight != m_Image->get_height()) {
            delete m_Image;
            m_Image = new TGAImage(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA);
        }

        if (!m_Texid)
            m_Texid = registeOpenGLTexture(m_ViewportWidth, m_ViewportHeight);

        if (m_Image) {
            refreshOpenGLTexture(m_Image->buffer(), m_ViewportWidth, m_ViewportHeight);
            ImGui::Image((ImTextureID)m_Texid, ImVec2(m_ViewportWidth, m_ViewportHeight));
        }

        ImGui::End();
    }

    void Render() {
        if (!m_Models) {

        }

        if (!m_skyBox) {
            m_skyBox = new Model("obj/skybox4/box.obj", IS_SKYBOX);
        }

        if (m_Image) {
            m_Image->clear();
        }
        if (m_Zbuffer) {
            // std::fill(m_Zbuffer, m_Zbuffer + 2000 * 2000, 1.f);
            for (int i = 1600 * 900; i--; m_Zbuffer[i] = 1.f)
                ;
        }

        shader_struct_a2v a2v;

        // handle events and update view, perspective matrix
        update_viewSize(m_Camera, m_ViewportWidth, m_ViewportHeight);
        handle_events(m_Camera);
        // set mvp matrix
        m_ModelM     = glm::mat4(1.f);
        m_View       = lookat(m_Camera.eye, m_Camera.target, m_Camera.up);
        m_Projection = perspective(TO_RADIANS(60.f), (float)(m_ViewportWidth) / m_ViewportHeight, 0.1f, 1000.f);
        m_ModelInv   = glm::inverse(m_ModelM);

        a2v._model      = m_ModelM;
        a2v._view       = m_View;
        a2v._projection = m_Projection;
        // m_View          = glm::mat4(glm::mat3(m_View)); // 去掉translate
        if (!m_DrawData._rdBuffer)
            m_DrawData._rdBuffer = new RenderBuffer(m_ViewportWidth, m_ViewportHeight);
        else
            m_DrawData._rdBuffer->clearColor(Color(0, 0.2, 0.3f, 1));
        for (int32_t i = 0; i < 4; ++i) {
            shaderType            = ShaderType::BlinnShader;
            m_DrawData._shader    = Create();
            m_DrawData._transform = {m_ModelM, m_View, m_Projection, m_ModelInv};

            m_DrawData._shader->setTransform(m_ModelM, m_View, m_Projection, m_ModelInv);
            m_DrawData._shader->setMaterial({&m_Models[i]->_diffuseMap, &m_Models[i]->_normalMap,
                                             &m_Models[i]->_specularMap, Color(0.8f, 0.8f, 0.8f), Color(1.f, 1.f, 1.f),
                                             50});
            std::dynamic_pointer_cast<BlinnShader>(m_DrawData._shader)->setLightData(light_dir, Color(1.f, 1.f, 1.f));
            m_DrawData._model = m_Models[i];


            m_DrawData._zBuffer = m_Zbuffer;

            // draw call entrypoint
            vertexProcessing(&m_DrawData, &a2v);
        }

        // skybox
        //m_DrawData._model  = m_skyBox;
        //shaderType         = ShaderType::SkyBoxShader;
        //m_DrawData._shader = Create();
        //m_View             = glm::mat4(glm::mat3(m_View)); // 去掉translate
        ////m_View[3][0] = 0;
        ////m_View[3][1] = 0;
        ////m_View[3][2] = 0;
        //m_DrawData._shader->setTransform(m_ModelM, m_View, m_Projection, m_ModelInv);
        //m_DrawData._shader->setSkyBox(&m_skyBox->_enviromentMap);

        //vertexProcessing(&m_DrawData, &a2v, IS_SKYBOX);

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
                    world_coords[j]    = m_ModelM * glm::vec4(vert, 1.f);
                    glm::vec4 eye_pos  = world_coords[j] * m_View;
                    glm::vec4 clip_pos = eye_pos * m_Projection;

                    glm::vec3 ndc_pos =
                        glm::vec3(clip_pos.x / clip_pos.w, clip_pos.y / clip_pos.w, clip_pos.z / clip_pos.w);
                    glm::vec3 screen_coord = viewport_transform(m_ViewportWidth, m_ViewportHeight, ndc_pos);
                    screen_coords[j]       = screen_coord;
                }
                TGAColor color = TGAColor(i / 36.f * 255, i / 36.f * 255, i / 36.f * 255, 255.f);
                DrawTriangle(screen_coords, m_Zbuffer, m_Image, color);
            }
        } else {
        }

        reset_camera();
        m_Image->setData(m_DrawData._rdBuffer->_colorBuffer);
        // delete m_DrawData._rdBuffer;
        m_Image->flip_vertically(); // i want to have the origin at the left bottom corner of the image
        // m_Image->write_tga_file("output.tga");
    }

private:
    uint32_t  m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image = nullptr;
    Model*    m_Models[4];
    // 天空盒
    Model* m_skyBox = nullptr;

    float  m_Zbuffer[2000 * 2000];
    Camera m_Camera;

    float m_Time = 0.f;

    // shader
    DrawData m_DrawData;

    // imgui
    uint32_t m_Texid = 0;

    glm::mat4 m_ModelM;
    glm::mat4 m_View;
    glm::mat4 m_Projection;
    glm::mat4 m_ModelInv;
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