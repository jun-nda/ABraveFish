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

void vertexProcessing(DrawData* drawData, shader_struct_a2v* a2v, bool isSkyBox = false) {
    shader_struct_v2f v2fs[3];

    auto model  = drawData->_model;
    auto shader = drawData->_shader;

    for (int32_t i = 0; i < model->getFaceCount(); i++) {
        std::vector<int32_t> face = model->getFace(i);
        for (int32_t j = 0; j < 3; j++) {
            a2v->_objPos    = model->getVert(face[j]);
            
            a2v->_objNormal = model->getNormal(i, j);
            a2v->_uv        = model->getUV(i, j);
            // vertex shading
            v2fs[j] = shader->vertex(a2v);
        }

        rasterization(drawData, v2fs, isSkyBox);
    }
}

class BraveFishLayer : public Layer {
public:
    BraveFishLayer()
        : m_Image(nullptr) {}

    virtual void OnAttach() override {
        for (int i = 1600 * 900; i--; m_Zbuffer[i] = 2.f)
            ;
    }

    virtual void OnUpdate(float ts) override {
        if (m_Image) {
            Timer timer;

            // m_Camera.update(ts);
            // m_Camera.updateTransformMatrix(m_ViewportWidth, m_ViewportHeight);
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

        // ImGui::Text();

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
        if (!m_Model) {
            m_Model = new Model("obj/yayi/yayibody.obj");
        }
        if (!m_skyBox) {
            m_skyBox = new Model("obj/skybox2/box.obj", IS_SKYBOX);
        }

        if (m_Image) {
            m_Image->clear();
        }
        if (m_Zbuffer) {
            // std::fill(m_Zbuffer, m_Zbuffer + 2000 * 2000, 1.f);
            for (int i = 1600 * 900; i--; m_Zbuffer[i] = 2.f)
                ;
        }

        shader_struct_a2v a2v;

        // handle events and update view, perspective matrix
        handle_events(m_Camera);
        // update_matrix(m_Camera, view, projection, shader_model, shader_skybox);
        // set mvp matrix
        m_ModelM     = glm::mat4(1.f);
        m_View       = lookat(m_Camera.eye, m_Camera.target, m_Camera.up);
        m_Projection = perspective(TO_RADIANS(60.f), (float)(m_ViewportWidth) / m_ViewportHeight, 0.1f, 1000.f);
        m_ModelInv   = glm::inverse(m_ModelM);

        a2v._model      = m_ModelM;
        a2v._view       = m_View;
        a2v._projection = m_Projection;

        shaderType            = ShaderType::BlinnShader;
        m_DrawData._shader    = Create();
        m_DrawData._transform = {m_ModelM, m_View, m_Projection, m_ModelInv};

        m_DrawData._shader->setTransform(m_ModelM, m_View, m_Projection, m_ModelInv);
        m_DrawData._shader->setMaterial({&m_Model->_diffuseMap, &m_Model->_normalMap, &m_Model->_specularMap,
                                         Color(0.5f, 0.5f, 0.5f), Color(1.f, 1.f, 1.f), 50});
        std::dynamic_pointer_cast<BlinnShader>(m_DrawData._shader)->setLightData(light_dir, Color(1.f, 1.f, 1.f));
        m_DrawData._model = m_Model;
        if (!m_DrawData._rdBuffer)
            m_DrawData._rdBuffer = new RenderBuffer(m_ViewportWidth, m_ViewportHeight);
        else
            m_DrawData._rdBuffer->clearColor(Color(0, 0.2, 0.3f, 1));

        m_DrawData._zBuffer = m_Zbuffer;

        // draw call entrypoint
        vertexProcessing(&m_DrawData, &a2v);

        // skybox
        //m_DrawData._model  = m_skyBox;
        //shaderType         = ShaderType::SkyBoxShader;
        //m_DrawData._shader = Create();
        //m_View             = glm::mat4(glm::mat3(m_View)); // 去掉translate
        //m_View[0][3]      = 0;
        //m_View[1][3]      = 0;
        //m_View[2][3]      = 0;
        //m_DrawData._shader->setTransform(m_ModelM, m_View, m_Projection, m_ModelInv);
        //m_DrawData._shader->setSkyBox(&m_skyBox->_enviromentMap);

        //vertexProcessing(&m_DrawData, &a2v, IS_SKYBOX);

        reset_camera();
        m_Image->setData(m_DrawData._rdBuffer->_colorBuffer);
        // delete m_DrawData._rdBuffer;
        m_Image->flip_vertically(); // i want to have the origin at the left bottom corner of the image
        // m_Image->write_tga_file("output.tga");
    }

private:
    uint32_t  m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image = nullptr;
    Model*    m_Model = nullptr;
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