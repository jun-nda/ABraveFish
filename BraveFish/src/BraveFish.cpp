
#include "Core/Application.h"
#include "Core/EntryPoint.h"

#include "Core/Layer.h"
#include "Renderer/Image.h"

#include "Core/Math.h"
#include "Core/Timer.h"
#include "Renderer/Model.h"

#include <iostream>
#include "Core/Timer.h"
#include "include/GLFW/glfw3.h"

using namespace ABraveFish;
const int depth = 255;

Vec3 camera(0, 0, 3);
Vec3 light_dir(0, 0, -1); // define light_dir

Vec3 m2v(Mat4 m) { return Vec3(m.m[0][0] / m.m[3][0], m.m[1][0] / m.m[3][0], m.m[2][0] / m.m[3][0]); }

Mat4 v2m(Vec3 v) {
    Mat4 m = mat4_identity();
    m.m[0][0] = v.x;
    m.m[1][0] = v.y;
    m.m[2][0] = v.z;
    m.m[3][0] = 1.f;
    return m;
}

Mat4 viewport(int x, int y, int w, int h) {
    Mat4 m   = mat4_identity();
    m.m[0][3]  = x + w / 2.f;
    m.m[1][3] = y + h / 2.f;
    m.m[2][3]  = depth / 2.f;

    m.m[0][0] = w / 2.f;
    m.m[1][1] = h / 2.f;
    m.m[2][2] = depth / 2.f;
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
Vec3 Barycentric(Vec3* pts, Vec2 P) {
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

void DrawTriangle(Vec3* pts, float* zbuffer, Vec2* uv, TGAImage* image, Model* model, float intensity) {
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
    Vec3 P;

    for (P.x = bboxmin[0]; P.x <= bboxmax[0]; P.x++) {
        for (P.y = bboxmin[1]; P.y <= bboxmax[1]; P.y++) {
            Vec3 bc_screen = Barycentric(pts, Vec2(P.x, P.y));
            if (bc_screen.x < 0.f || bc_screen.y < 0.f || bc_screen.z < 0.f)
                continue;
            if (zbuffer[int(P.x + P.y * width)] < P.z) {
                zbuffer[int(P.x + P.y * width)] = P.z;

                // alpha beta gamma,for uv
                // Vec2 uvA = vec2_add(uv[0], vec2_mul(vec2_sub(uv[2], uv[0]), bc_screen.x));
                // Vec2 uvB = vec2_add(uv[1], vec2_mul(vec2_sub(uv[2], uv[1]), bc_screen.y));
                // Vec2 uvC = vec2_add(uv[2], vec2_mul(vec2_sub(uv[0], uv[2]), bc_screen.z));
                Vec2     uvP   = vec2_add(vec2_add(vec2_mul(uv[0], bc_screen.x), vec2_mul(uv[1], bc_screen.y)),
                                    vec2_mul(uv[2], bc_screen.z));
                TGAColor color = model->Diffuse(uvP);
                image->set(P.x, P.y, TGAColor(color.b * intensity, color.g * intensity, color.r * intensity, 255.f));
            }
        }
    }
}
void DrawTriangle(Vec2 t0, Vec2 t1, Vec2 t2, TGAImage* image, TGAColor color) {
    if (t0.y == t1.y && t0.y == t2.y)
        return; // i dont care about degenerate triangles
    if (t0.y > t1.y)
        std::swap(t0, t1);
    if (t0.y > t2.y)
        std::swap(t0, t2);
    if (t1.y > t2.y)
        std::swap(t1, t2);
    int32_t total_height = t2.y - t0.y;
    for (int32_t i = 0; i < total_height; i++) {
        bool    second_half    = i > t1.y - t0.y || t1.y == t0.y;
        int32_t segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        double  alpha          = (double)i / total_height;
        double  beta           = (double)(i - (second_half ? t1.y - t0.y : 0)) /
                      segment_height; // be careful: with above conditions no division by zero here
        Vec2 A = Vec2(t0.x + (t2.x - t0.x) * alpha, t0.y + (t2.y - t0.y) * alpha);
        Vec2 B = second_half ? Vec2(t1.x + (t2.x - t1.x) * beta, t1.y + (t2.y - t1.y) * beta)
                             : Vec2(t0.x + (t1.x - t0.x) * beta, t0.y + (t1.y - t0.y) * beta);
        if (A.x > B.x)
            std::swap(A, B);
        for (int32_t j = A.x; j <= B.x; j++) {
            image->set(j, t0.y + i, color); // attention, due to int32_t casts t0.y+i != A.y
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

            Render();
        }

        if (m_Image)
            ImGui::Image((ImTextureID)registeOpenGLTexture(m_Image->buffer(), m_ViewportWidth, m_ViewportHeight),
                         ImVec2(m_ViewportWidth, m_ViewportHeight));

        ImGui::End();

        m_Time = timer.ElapsedMillis();
    }

    void Render() {
        // DrawLine(80, 120, 300, 40, m_Image, TGAColor(255, 2, 1, 255));
        if (!m_Model) {
            m_Model = new Model("obj/african_head.obj");
        }

        // 线框模式
        // for (int32_t i = 0; i < m_Model->GetFaceCount(); i++) {
        //    std::vector<int32_t> face = m_Model->Face(i);
        //    for (int32_t j = 0; j < 3; j++) {
        //        Vec3    v0 = m_Model->Vert(face[j]);
        //        Vec3    v1 = m_Model->Vert(face[(j + 1) % 3]);
        //        int32_t x0 = (v0.x + 1.) * m_ViewportWidth / 2.;
        //        int32_t y0 = (v0.y + 1.) * m_ViewportHeight / 2.;
        //        int32_t x1 = (v1.x + 1.) * m_ViewportWidth / 2.;
        //        int32_t y1 = (v1.y + 1.) * m_ViewportHeight / 2.;
        //        DrawLine(x0, y0, x1, y1, m_Image, TGAColor(255, 255, 255, 255));
        //    }
        //}

        float* zbuffer = new float[m_ViewportWidth * m_ViewportHeight];
        for (int i = m_ViewportWidth * m_ViewportHeight; i--; zbuffer[i] = -std::numeric_limits<float>::max())
            ;

        Mat4 Projection   = mat4_identity();
        Mat4 ViewPort =
            viewport(m_ViewportWidth / 8, m_ViewportHeight / 8, m_ViewportWidth * 3 / 4, m_ViewportHeight * 3 / 4);
        Projection.m[3][2]  = -1.f / camera.z;

        for (int32_t i = 0; i < m_Model->GetFaceCount(); i++) {
            std::vector<int32_t> face = m_Model->Face(i);
            Vec3                 screen_coords[3];
            Vec3                 world_coords[3];
            for (int32_t j = 0; j < 3; j++) {
                Vec3 v           = m_Model->Vert(face[j]);
                screen_coords[j] = m2v(mat4_mul_mat4(mat4_mul_mat4(ViewPort, Projection), v2m(v)));
                world_coords[j]  = v;
            }

            Vec3 norm = vec3_normalize(
                vec3_cross(vec3_sub(world_coords[2], world_coords[1]), vec3_sub(world_coords[1], world_coords[0])));
            double intensity = vec3_dot(norm, light_dir);

            if (intensity > 0) {
                // texture
                Vec2 uv[3];
                for (int k = 0; k < 3; k++) {
                    uv[k] = m_Model->UV(i, k);
                }

                // DrawTriangle(screen_coords[0], screen_coords[1], screen_coords[2], m_Image,
                //             TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
                DrawTriangle(screen_coords, zbuffer, uv, m_Image, m_Model, intensity);
            }
        }

        m_Image->flip_vertically(); // 反转y轴
        m_Image->write_tga_file("out.tga");
    }

private:
    uint32_t  m_ViewportWidth = 0, m_ViewportHeight = 0;
    TGAImage* m_Image;
    Model*    m_Model = nullptr;

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