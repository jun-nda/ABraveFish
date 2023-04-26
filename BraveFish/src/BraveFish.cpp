/*
 * 空间变换部分的两个坑： 1. glm矩阵乘法是反着的。 2. games101只讲了mvp，之后还要进行透视除法。
 * 一定要进行裁剪，不然w为负值的时候会画出奇怪的东西，画天空盒的时候发现的
 *
 *
 *
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
#include "Renderer/Scene.h"

#include "Renderer/Util.h"
using namespace ABraveFish;

const int depth = 255;

class BraveFishLayer : public Layer {
public:
    BraveFishLayer()
        : m_Image(nullptr) {}

    virtual void OnAttach() override {
        for (int i = appWidth * appHeight; i--; m_Zbuffer[i] = 1.f)
            ;

        m_BuildScene[0] = buildQiyanaScene;
        m_BuildScene[1] = buldFuhuaScene;
        m_BuildScene[2] = buldHelmetScene;

        m_SceneIndex = rand() % 3;
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

        ImGui::Text("CameraPos: %.2f, %.2f, %.2f", m_Camera.eye.x, m_Camera.eye.y, m_Camera.eye.z);

        ImGui::Text("VertexNum: %d", m_VertexNum);
        ImGui::Text("FaceNum: %d", m_FaceNum);

        if (ImGui::Button("fuhua")) {
            m_SceneIndex = 1;
        }

        ImGui::End();


        ImGui::Begin("Render");

        //ImGui::ShowDemoWindow();


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
        if (!m_skyBox) {
            m_skyBox = new Model("obj/skybox4/box.obj", IS_SKYBOX);
        }

        if (m_Image) {
            m_Image->clear();
        }
        if (m_Zbuffer) {
            // std::fill(m_Zbuffer, m_Zbuffer + 2000 * 2000, 1.f);
            for (int i = appWidth * appHeight; i--; m_Zbuffer[i] = 1.f)
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

        m_DrawData._camera = &m_Camera;

        Transform transform(m_ModelM, m_View, m_Projection, m_ModelInv);


        m_BuildScene[m_SceneIndex](m_Models, &m_DrawData, &transform, m_Zbuffer, &a2v, m_modelNum, m_VertexNum,
                                   m_FaceNum);

        reset_camera();
        m_Image->setData(m_DrawData._rdBuffer->_colorBuffer);
        // delete m_DrawData._rdBuffer;
        m_Image->flip_vertically(); // i want to have the origin at the left bottom corner of the image
        // m_Image->write_tga_file("output.tga");
    }

private:
    uint32_t  m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image = nullptr;

    int32_t m_VertexNum = 0, m_FaceNum = 0;

    int32_t m_modelNum;
    Model*  m_Models[MAX_MODELNUM] = {nullptr};
    // 天空盒
    Model* m_skyBox = nullptr;

    float  m_Zbuffer[appWidth * appHeight];
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

    // bulid scene
    buildSceneFunc m_BuildScene[MAX_SCENENUM];
    int32_t m_SceneIndex = 0;
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