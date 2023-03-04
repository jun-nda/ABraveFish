
#include "Core/Application.h"
#include "Core/EntryPoint.h"

#include "Core/Layer.h"
#include "renderer/Image.h"

#include "Core/Math.h"
#include "Renderer/Model.h"
#include "Core/Timer.h"

#include "include/GLFW/glfw3.h"
#include "Core/Timer.h"
#include <iostream>

using namespace ABraveFish;

void DrawLine(int x0, int y0, int x1, int y1, TGAImage* image, TGAColor color) {
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

Vec3 barycentric(Vec2* pts, Vec2 P) {
    Vec3 u = vec3_cross(Vec3({pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x}),
                        Vec3({pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y}));
    /* `pts` and `P` has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means
       triangle is degenerate, in this case return something with negative coordinates */
    // 面积为0的退化三角形？
    if (std::abs(u.z) < 1)
        return Vec3({-1, 1, 1});
    return Vec3({1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z});
}

void triangle(Vec2* pts, TGAImage* image, TGAColor color) {
    // 
    Vec2 bboxmin(image->get_width() - 1, image->get_height() - 1);
    Vec2 bboxmax(0.f, 0.f);
    Vec2 clamp(image->get_width() - 1, image->get_height() - 1);
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec2 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3 bc_screen = barycentric(pts, Vec2(P.x + 0.5, P.y + 0.5));
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            image->set(P.x, P.y, color);
        }
    }
} 
void triangle(Vec2 t0, Vec2 t1, Vec2 t2, TGAImage* image, TGAColor color) {
    if (t0.y == t1.y && t0.y == t2.y)
        return; // i dont care about degenerate triangles
    if (t0.y > t1.y)
        std::swap(t0, t1);
    if (t0.y > t2.y)
        std::swap(t0, t2);
    if (t1.y > t2.y)
        std::swap(t1, t2);
    int total_height = t2.y - t0.y;
    for (int i = 0; i < total_height; i++) {
        bool  second_half    = i > t1.y - t0.y || t1.y == t0.y;
        int   segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha          = (float)i / total_height;
        float beta           = (float)(i - (second_half ? t1.y - t0.y : 0)) /
                     segment_height; // be careful: with above conditions no division by zero here
        Vec2 A = Vec2(t0.x + (t2.x - t0.x) * alpha, t0.y + (t2.y - t0.y) * alpha);
        Vec2 B = second_half ? Vec2(t1.x + (t2.x - t1.x) * beta, t1.y + (t2.y - t1.y) * beta) 
                             : Vec2(t0.x + (t1.x - t0.x) * beta, t0.y + (t1.y - t0.y) * beta);
        if (A.x > B.x)
            std::swap(A, B);
        for (int j = A.x; j <= B.x; j++) {
            image->set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y
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
        Timer timer;

        ImGui::Begin("Settings");
        ImGui::Text("yujunda");
        ImGui::End();

        ImGui::Begin("Render");

        m_ViewportWidth  = ImGui::GetContentRegionAvail().x;
        m_ViewportHeight = ImGui::GetContentRegionAvail().y;

        std::cout << "imagebegin:" << timer.ElapsedMillis() << std::endl;
        //if (m_Image == nullptr) {
        m_Image = new TGAImage(m_ViewportWidth, m_ViewportHeight, TGAImage::RGBA); 
        //}
        std::cout << "imageend:" << timer.ElapsedMillis() << std::endl;

        //DrawLine(80, 120, 300, 40, m_Image, TGAColor(255, 2, 1, 255));
        if (m_Model == nullptr) {
            m_Model = new Model("obj/african_head.obj");
        }

        for (int i = 0; i < m_Model->GetFaceCount(); i++) {
            std::vector<int> face = m_Model->Face(i);
            Vec2            screen_coords[3];
            for (int j = 0; j < 3; j++) {
                Vec3 world_coords = m_Model->Vert(face[j]);
                screen_coords[j] =
                    Vec2((world_coords.x + 1.) * m_ViewportWidth / 2., (world_coords.y + 1.) * m_ViewportHeight / 2.);
            }
            //triangle(screen_coords[0], screen_coords[1], screen_coords[2], m_Image,
            //         TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
            triangle(screen_coords, m_Image,
                     TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
        }

        m_Image->flip_vertically(); // 反转y轴

        ImGui::Image((ImTextureID)registeOpenGLTexture(m_Image->buffer(), m_ViewportWidth, m_ViewportHeight),
                     ImVec2(m_ViewportWidth, m_ViewportHeight)
        );
        ImGui::End();
        std::cout << "imageend2:" << timer.ElapsedMillis() << std::endl;
    }

private:
    uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image = nullptr;
    Model*    m_Model = nullptr;
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