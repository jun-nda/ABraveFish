#include <iostream>
#include <unordered_map>

#include "Core/Image.h"
#include "RenderDevice.h"
#include "Shader.h"

namespace ABraveFish {

// std::unordered_map<int32_t, Color> = {
//    {1}
//}

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

/*
 * eye: the position of the eye point
 * target: the position of the target point
 * up: the direction of the up vector
 *
 * x_axis.x  x_axis.y  x_axis.z  -dot(x_axis,eye)
 * y_axis.x  y_axis.y  y_axis.z  -dot(y_axis,eye)
 * z_axis.x  z_axis.y  z_axis.z  -dot(z_axis,eye)
 *        0         0         0                 1
 *
 * z_axis: normalize(eye-target), the backward vector
 * x_axis: normalize(cross(up,z_axis)), the right vector
 * y_axis: cross(z_axis,x_axis), the up vector
 *
 * see http://www.songho.ca/opengl/gl_camera.html
 */
// 观察矩阵
glm::mat4 lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 z_axis = glm::normalize(eye - center);
    glm::vec3 x_axis = glm::normalize(cross(up, z_axis));
    glm::vec3 y_axis = cross(z_axis, x_axis);
    glm::mat4 m(1.f);

    m[0] = glm::vec4(x_axis, 0.f);
    m[1] = glm::vec4(y_axis, 0.f);
    m[2] = glm::vec4(z_axis, 0.f);

    m[0][3] = -glm::dot(x_axis, eye);
    m[1][3] = -glm::dot(y_axis, eye);
    m[2][3] = -glm::dot(z_axis, eye);

    return m;
}

/*
 * fovy: the field of view angle in the y direction, in radians
 * aspect: the aspect ratio, defined as width divided by height
 * near, far: the distances to the near and far depth clipping planes
 *
 * 1/(aspect*tan(fovy/2))              0             0           0
 *                      0  1/tan(fovy/2)             0           0
 *                      0              0  -(f+n)/(f-n)  -2fn/(f-n)
 *                      0              0            -1           0
 *
 * this is the same as
 *     float half_h = near * (float)tan(fovy / 2);
 *     float half_w = half_h * aspect;
 *     frustum(-half_w, half_w, -half_h, half_h, near, far);
 *
 * see http://www.songho.ca/opengl/gl_projectionmatrix.html
 */
// 透视投影矩阵
/*
 * Note that the eye coordinates are defined in the right-handed coordinate system,
 * but NDC uses the left-handed coordinate system.  That is, the camera at the origin
 * is looking along -Z axis in eye space, but it is looking along +Z axis in NDC.
 * Since glFrustum() accepts only positive values of near and far distances,
 * we need to negate them during the construction of GL_PROJECTION matrix.
 */
glm::mat4 perspective(float fovy, float aspect, float near, float far) {
    float     z_range = far - near;
    glm::mat4 m(1.f);
    assert(fovy > 0 && aspect > 0);
    assert(near > 0 && far > 0 && z_range > 0);
    m[1][1] = 1 / (float)tan(fovy / 2);
    m[0][0] = m[1][1] / aspect;
    m[2][2] = -(near + far) / z_range;
    m[2][3] = -2 * near * far / z_range;
    m[3][2] = -1;
    m[3][3] = 0;
    return m;
}

/*
 * for viewport transformation, see subsection 2.12.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
// 视口变换
glm::vec3 viewport_transform(int width, int height, glm::vec3 ndc_coord) {
    float x = (ndc_coord.x + 1) * 0.5f * (float)width;  /* [-1, 1] -> [0, w] */
    float y = (ndc_coord.y + 1) * 0.5f * (float)height; /* [-1, 1] -> [0, h] */
    float z = (ndc_coord.z + 1) * 0.5f;                 /* [-1, 1] -> [0, 1] */
    return glm::vec3(x, y, z);
}

bool isBackFacing(glm::vec3 ndc_coords[]) {
    glm::vec3 a           = ndc_coords[0];
    glm::vec3 b           = ndc_coords[1];
    glm::vec3 c           = ndc_coords[2];
    float     signed_area = a.x * b.y - a.y * b.x + b.x * c.y - b.y * c.x + c.x * a.y - c.y * a.x;
    return signed_area <= 0;
}

float interpolateDepth(float* screenDepth, glm::vec3 weights) {
    glm::vec3 screen_depth;
    for (size_t i = 0; i < 3; i++) {
        screen_depth[i] = screenDepth[i];
    }

    return glm::dot(screen_depth, weights);
}

// 重心坐标
glm::vec3 Barycentric(glm::vec3* v, float x, float y) {
    float c1 = (x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * y + v[1].x * v[2].y - v[2].x * v[1].y) /
               (v[0].x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * v[0].y + v[1].x * v[2].y - v[2].x * v[1].y);
    float c2 = (x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) /
               (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
    return glm::vec3(c1, c2, 1 - c1 - c2);
}

/*
 * for perspective correct interpolation, see
 * https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 *
 * equation 15 in reference 1 (page 2) is a simplified 2d version of
 * equation 3.5 in reference 2 (page 58) which uses barycentric coordinates
 */
void interpolate_varyings(shader_struct_v2f* v2f, shader_struct_v2f* ret, int sizeof_varyings, glm::vec3 weights,
                          float recip_w[3]) {
    int    num_floats = sizeof_varyings / sizeof(float);
    float* src0       = (float*)(&v2f[0]);
    float* src1       = (float*)(&v2f[1]);
    float* src2       = (float*)(&v2f[2]);
    float* dst        = (float*)ret;
    float  weight0    = recip_w[0] * weights.x;
    float  weight1    = recip_w[1] * weights.y;
    float  weight2    = recip_w[2] * weights.z;
    float  normalizer = 1 / (weight0 + weight1 + weight2);
    int    i;
    for (i = 0; i < num_floats; i++) {
        float sum = src0[i] * weight0 + src1[i] * weight1 + src2[i] * weight2;
        dst[i]    = sum * normalizer;
    }
}

void rasterization(DrawData* data, shader_struct_v2f* v2fs, bool isSkyBox, int32_t index) {
    auto rdBuffer = data->_rdBuffer;
    auto zbuffer  = data->_zBuffer;
    auto model    = data->_model;

    int32_t width  = rdBuffer->_width;
    int32_t height = rdBuffer->_height;

    glm::vec3 ndc_coords[3];

    for (int32_t i = 0; i < 3; i++) {
        // perspective division and viewport transform
        auto& clipPos = v2fs[i]._clipPos;
        if (clipPos.w < 0.f)
            return;
        ndc_coords[i] = glm::vec4(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w, 1.0f);
    }

    glm::vec3 screen_coords[3];
    glm::vec3 screenDepths;

    for (int32_t i = 0; i < 3; i++) {
        // perspective division and viewport transform
        auto&     clipPos      = v2fs[i]._clipPos;
        glm::vec4 ndcPos       = glm::vec4(clipPos.x / clipPos.w, clipPos.y / clipPos.w, clipPos.z / clipPos.w, 1.0f);
        glm::vec3 screen_coord = viewport_transform(width, height, ndcPos);
        screen_coords[i]       = screen_coord;
        screenDepths[i]        = screen_coord.z;
    }

    // for (int32_t i = 0; i < 3; i++) {
    //    // perspective division and viewport transform
    //    if (screen_coords[i].x < 0.f || screen_coords[i].y < 0.f || screen_coords[i].x > width ||
    //        screen_coords[i].y > height) {
    //        return;
    //    }
    //}

    // 背面剔除
    if (!isSkyBox) {
        if (isBackFacing(ndc_coords))
            return;
    }

    float recip_w[3];
    /* reciprocals of w */
    for (int i = 0; i < 3; i++) {
        recip_w[i] = 1 / v2fs[i]._clipPos[3];
    }

    // Attention: box must be int
    int32_t bboxmin[2] = {std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()};
    int32_t bboxmax[2] = {-std::numeric_limits<int32_t>::max(), -std::numeric_limits<int32_t>::max()};
    int32_t clamp[2]   = {width - 1, height - 1};
    for (int32_t i = 0; i < 3; i++) {
        bboxmin[0] = std::max(0, std::min(bboxmin[0], (int32_t)screen_coords[i].x));
        bboxmin[1] = std::max(0, std::min(bboxmin[1], (int32_t)screen_coords[i].y));

        bboxmax[0] = std::min(clamp[0], std::max(bboxmax[0], (int32_t)screen_coords[i].x));
        bboxmax[1] = std::min(clamp[1], std::max(bboxmax[1], (int32_t)screen_coords[i].y));
    }

    glm::vec3 P(1.f);

    // 未进行裁剪，当一个三角形有顶点在外面的时候，也进行光栅化，但只光栅了屏幕空间范围内的部分

    for (P.x = bboxmin[0]; P.x <= bboxmax[0]; P.x++) {
        for (P.y = bboxmin[1]; P.y <= bboxmax[1]; P.y++) {
            glm::vec3 bc_screen = Barycentric(screen_coords, P.x + 0.5f, P.y + 0.5f);
            if (bc_screen.x < 0.f || bc_screen.y < 0.f || bc_screen.z < 0.f)
                continue;
            int32_t idx = P.x + P.y * width;

            // 深度插值
            float frag_depth = glm::dot(screenDepths, bc_screen);

            if (zbuffer[idx] < frag_depth)
                continue;

            //变量插值
            shader_struct_v2f interpolate_v2f;
            interpolate_varyings(v2fs, &interpolate_v2f, sizeof(shader_struct_v2f), bc_screen, recip_w);

            Color color;
            bool  discard = data->_shader->fragment(&interpolate_v2f, color);

            if (!discard) {
                rdBuffer->setColor(P.x, P.y, TGAColor(color.r * 255.f, color.g * 255.f, color.b * 255.f));
                zbuffer[idx] = frag_depth;
            }
        }
    }
}

void vertexProcessing(DrawData* drawData, shader_struct_a2v* a2v, bool isSkyBox) {
    shader_struct_v2f v2fs[3];

    auto& model  = drawData->_model;
    auto& shader = drawData->_shader;

    for (int32_t i = 0; i < model->getFaceCount(); i++) {
        std::vector<int32_t> face = model->getFace(i);
        for (int32_t j = 0; j < 3; j++) {
            a2v->_objPos = model->getVert(face[j]);

            a2v->_objNormal = model->getNormal(i, j);
            a2v->_uv        = model->getUV(i, j);

            // for homogenous clip
            a2v->_vertIndex = j;
            // vertex shading
            v2fs[j] = shader->vertex(a2v);
        }

        // homogenous cliping
        int32_t num_vertex = homoClipping(shader->_homogenousClip);

        // triangle assembly and reaterize
        for (int i = 0; i < num_vertex - 2; i++) {
            int32_t indexArray[3];
            indexArray[0] = 0;
            indexArray[1] = i + 1;
            indexArray[2] = i + 2;
            // transform data to real vertex attri
            transformAttri(v2fs, shader->_homogenousClip, indexArray);
            rasterization(drawData, v2fs, isSkyBox, i);
        }
    }
}

void transformAttri(shader_struct_v2f* v2fs, HomogenousClip& clipData, int32_t* indexArray) { 
    for (int i = 0; i < 3; i++) {
        v2fs[i]._clipPos = clipData.out_clipcoord[indexArray[i]];
        v2fs[i]._worldPos = clipData.out_worldcoord[indexArray[i]];
        v2fs[i]._uv = clipData.out_uv[indexArray[i]];
        v2fs[i]._worldNormal = clipData.out_normal[indexArray[i]];
    }
}


int32_t homoClipping(HomogenousClip& clipData) {
    int32_t num_vertex = 3;
    num_vertex         = clipWithPlane(W_PLANE, num_vertex, clipData);
    num_vertex         = clipWithPlane(X_RIGHT, num_vertex, clipData);
    num_vertex         = clipWithPlane(X_LEFT, num_vertex, clipData);
    num_vertex         = clipWithPlane(Y_TOP, num_vertex, clipData);
    num_vertex         = clipWithPlane(Y_BOTTOM, num_vertex, clipData);
    num_vertex         = clipWithPlane(Z_NEAR, num_vertex, clipData);
    num_vertex         = clipWithPlane(Z_FAR, num_vertex, clipData);
    return num_vertex;
}
int32_t clipWithPlane(ClipPlane plane, int32_t numVertex, HomogenousClip& clipData) {
    int out_vert_num = 0;
    int previous_index, current_index;
    int is_odd = (plane + 1) % 2;

    // set the right in and out datas
    glm::vec4* in_clipcoord   = is_odd ? clipData.in_clipcoord : clipData.out_clipcoord;
    glm::vec3* in_worldcoord  = is_odd ? clipData.in_worldcoord : clipData.out_worldcoord;
    glm::vec3* in_normal      = is_odd ? clipData.in_normal : clipData.out_normal;
    glm::vec2* in_uv          = is_odd ? clipData.in_uv : clipData.out_uv;
    glm::vec4* out_clipcoord  = is_odd ? clipData.out_clipcoord : clipData.in_clipcoord;
    glm::vec3* out_worldcoord = is_odd ? clipData.out_worldcoord : clipData.in_worldcoord;
    glm::vec3* out_normal     = is_odd ? clipData.out_normal : clipData.in_normal;
    glm::vec2* out_uv         = is_odd ? clipData.out_uv : clipData.in_uv;

    // tranverse all the edges from first vertex
    for (int i = 0; i < numVertex; i++) {
        current_index        = i;
        previous_index       = (i - 1 + numVertex) % numVertex;
        glm::vec4 cur_vertex = in_clipcoord[current_index];
        glm::vec4 pre_vertex = in_clipcoord[previous_index];

        int is_cur_inside = isInsidePlane(plane, cur_vertex);
        int is_pre_inside = isInsidePlane(plane, pre_vertex);
        if (is_cur_inside != is_pre_inside) {
            float ratio = getIntersectRatio(pre_vertex, cur_vertex, plane);

            out_clipcoord[out_vert_num]  = glm::mix(pre_vertex, cur_vertex, ratio);
            out_worldcoord[out_vert_num] = glm::mix(in_worldcoord[previous_index], in_worldcoord[current_index], ratio);
            out_normal[out_vert_num]     = glm::mix(in_normal[previous_index], in_normal[current_index], ratio);
            out_uv[out_vert_num]         = glm::mix(in_uv[previous_index], in_uv[current_index], ratio);

            out_vert_num++;
        }

        if (is_cur_inside) {
            out_clipcoord[out_vert_num]  = cur_vertex;
            out_worldcoord[out_vert_num] = in_worldcoord[current_index];
            out_normal[out_vert_num]     = in_normal[current_index];
            out_uv[out_vert_num]         = in_uv[current_index];

            out_vert_num++;
        }
    }

    return out_vert_num;
}

bool isInsidePlane(ClipPlane plane, glm::vec4& vertex) {
    switch (plane) {
        case W_PLANE:
            return vertex.w >= EPSILON;
        case X_RIGHT:
            return vertex.x <= vertex.w;
        case X_LEFT:
            return vertex.x >= -vertex.w;
        case Y_TOP:
            return vertex.y <= vertex.w;
        case Y_BOTTOM:
            return vertex.y >= -vertex.w;
        case Z_NEAR:
            return vertex.z <= vertex.w;
        case Z_FAR:
            return vertex.z >= -vertex.w;
        default:
            return 0;
    }
}
float getIntersectRatio(glm::vec4& preVertex, glm::vec4& curVertex, ClipPlane plane) {
    switch (plane) {
        case W_PLANE:
            return (preVertex.w + EPSILON) / (preVertex.w - curVertex.w);
        case X_RIGHT:
            return (preVertex.w - preVertex.x) / ((preVertex.w - preVertex.x) - (curVertex.w - curVertex.x));
        case X_LEFT:
            return (preVertex.w + preVertex.x) / ((preVertex.w + preVertex.x) - (curVertex.w + curVertex.x));
        case Y_TOP:
            return (preVertex.w - preVertex.y) / ((preVertex.w - preVertex.y) - (curVertex.w - curVertex.y));
        case Y_BOTTOM:
            return (preVertex.w + preVertex.y) / ((preVertex.w + preVertex.y) - (curVertex.w + curVertex.y));
        case Z_NEAR:
            return (preVertex.w - preVertex.z) / ((preVertex.w - preVertex.z) - (curVertex.w - curVertex.z));
        case Z_FAR:
            return (preVertex.w + preVertex.z) / ((preVertex.w + preVertex.z) - (curVertex.w + curVertex.z));
        default:
            return 0;
    }
}

// for cube
glm::vec3 Barycentric(glm::vec3* pts, glm::vec2 P) {
    glm::vec3 u = glm::cross(glm::vec3({pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x}),
                             glm::vec3({pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y}));
    // 面积为0的退化三角形？
    if (std::abs(u.z) < 1)
        return glm::vec3({-1, 1, 1});
    return glm::vec3({1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z});
}

// for cube
void DrawTriangle(glm::vec3* pts, float* zbuffer, TGAImage* image, TGAColor color) {
    int32_t width  = image->get_width();
    int32_t height = image->get_height();

    // Attention: box must be int
    int32_t bboxmin[2] = {width, height};
    int32_t bboxmax[2] = {0, 0};
    int32_t clamp[2]   = {width, height};
    for (int32_t i = 0; i < 3; i++) {
        bboxmin[0] = std::max(0, std::min(bboxmin[0], (int32_t)pts[i].x));
        bboxmin[1] = std::max(0, std::min(bboxmin[1], (int32_t)pts[i].y));

        bboxmax[0] = std::min(clamp[0], std::max(bboxmax[0], (int32_t)pts[i].x));
        bboxmax[1] = std::min(clamp[1], std::max(bboxmax[1], (int32_t)pts[i].y));
    }
    glm::vec3 P(1.f);

    float depths[3];
    for (int32_t i = 0; i < 3; ++i) {
        depths[i] = pts[i].z;
    }
    for (P.x = bboxmin[0]; P.x <= bboxmax[0]; P.x++) {
        for (P.y = bboxmin[1]; P.y <= bboxmax[1]; P.y++) {
            glm::vec3 bc_screen = Barycentric(pts, glm::vec2(P.x, P.y));
            if (bc_screen.x < 0.f || bc_screen.y < 0.f || bc_screen.z < 0.f)
                continue;

            float   depth = interpolateDepth(depths, bc_screen);
            int32_t idx   = P.x + P.y * width;
            if (zbuffer[idx] > depth) {
                zbuffer[idx] = depth;
                image->set(P.x, P.y, color);
            }
        }
    }
}

// 打印矩阵
void PrintMatrix(glm::mat4 m) {
    std::cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << std::endl;
    std::cout << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << std::endl;
    std::cout << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << std::endl;
    std::cout << m[3][0] << " " << m[3][1] << " " << m[3][2] << " " << m[3][3] << std::endl;
}

} // namespace ABraveFish