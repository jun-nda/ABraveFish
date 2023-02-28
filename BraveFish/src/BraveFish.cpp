
#include "Core/Application.h"
#include "Core/EntryPoint.h"

#include "Core/Layer.h"
#include "renderer/Image.h"

#include "include/GLFW/glfw3.h"

#include <iostream>

using namespace ABraveFish;

void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
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
    int dx = x1 - x0;
    int dy = y1 - y0;

    int derror2 = std::abs(dy) * 2;
    int error2  = 0;

    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
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
    virtual void OnUIRender() {
        bool is = true;
        //ImGui::ShowDemoWindow(&is);

        m_ViewportWidth  = ImGui::GetContentRegionAvail().x;
        m_ViewportHeight = ImGui::GetContentRegionAvail().y;

        ImGui::Begin("Settings");
        ImGui::Text("yujunda");
        ImGui::End();

        ImGui::Begin("Render");

        TGAImage image(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA);
        //DrawLine(80, 120, 300, 40, image, TGAColor(255, 2, 1, 255));

        //ImGui::Image(
        //    (ImTextureID)registeOpenGLTexture(image.buffer(), image.get_width(), image.get_height()),
        //             ImVec2(m_ViewportWidth, m_ViewportHeight)
        //);
        ImGui::End();
    }

private:
    uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
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