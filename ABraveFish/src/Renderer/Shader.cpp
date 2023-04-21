#include "Shader.h"
#include "Shader/PBRFunc.h"

#include <iostream>

#define VEC4(pos) glm::vec4(pos, 1.f)
namespace ABraveFish {
static float     saturate(float f) { return f < 0 ? 0 : (f > 1 ? 1 : f); }
static glm::vec3 color2Vec3(Color color) { return glm::vec3(color.r, color.g, color.b); }

static Color AMBIENT = Color(54.f / 255, 58.f / 255, 66.f / 255);
void         Shader::setTransform(Transform* transform) { _transform = Transform(*transform); }

void Shader::setMaterial(const Material& material) { _material = material; }
void Shader::setSkyBox(CubeMap* cubeMap) {
    _material._diffuseMap  = nullptr;
    _material._normalMap   = nullptr;
    _material._specularMap = nullptr;

    _material._cubeMap = cubeMap;
}

glm::vec4 Shader::object2ClipPos(const glm::vec3& objPos) {
    glm::vec4 temp = VEC4(objPos) * _transform._model * _transform._view;
    // std::cout << temp.x << " " << temp.y << " " << temp.z << " " << temp.w << std::endl;
    return temp * _transform._projection;
}

glm::vec3 Shader::object2WorldPos(const glm::vec3& objPos) { return glm::vec3(VEC4(objPos) * _transform._model); }
glm::vec3 Shader::object2WorldNormal(const glm::vec3& objNormal) {
    return glm::vec3(objNormal * glm::mat3(glm::transpose(_transform._modelInv)));
}

shader_struct_v2f BlinnShader::vertex(shader_struct_a2v* a2v) {
    shader_struct_v2f v2f;
    v2f._clipPos     = object2ClipPos(a2v->_objPos);
    v2f._worldPos    = object2WorldPos(a2v->_objPos);
    v2f._worldNormal = object2WorldNormal(a2v->_objNormal);
    v2f._uv          = a2v->_uv;

    // for homogenous clipping
    _homogenousClip.in_clipcoord[a2v->_vertIndex]  = v2f._clipPos;
    _homogenousClip.in_worldcoord[a2v->_vertIndex] = v2f._worldPos;
    _homogenousClip.in_normal[a2v->_vertIndex]     = v2f._worldNormal;
    _homogenousClip.in_uv[a2v->_vertIndex]         = v2f._uv;
    return v2f;
}

bool BlinnShader::fragment(shader_struct_v2f* v2f, Color& color) {
    glm::vec3 worldNormalDir = glm::normalize(v2f->_worldNormal);
    Color     albedo         = diffuseSample(v2f->_uv) * _material.color;
    // Color     albedo         =  _material.color;

    Color     ambient = glm::dot(_material.color, albedo);
    float     n_dot_l = saturate(glm::dot(worldNormalDir, glm::normalize(_lightDir)));
    Color     diffuse = _ligthColor * albedo * n_dot_l;
    glm::vec3 viewDir = glm::normalize(worldSpaceViewDir(v2f->_worldPos));
    glm::vec3 halfDir = glm::normalize(viewDir + _lightDir);
    Color     spcular =
        _ligthColor * _material.specular * std::pow(saturate(glm::dot(worldNormalDir, halfDir)), _material.gloss);

    // glm::vec4 depth_pos = _lightVP * glm::vec4(v2f->_worldPos, 1.f);
    color = ambient + (diffuse + spcular);
    // color = diffuse + spcular;
    //color = albedo;

    // color               = Color(255, 255, 255);
    return false;
}

void      BlinnShader::setLightData(const glm::vec3& dir, const Color& color) { _lightDir = dir, _ligthColor = color; }
void      BlinnShader::setEyePos(const glm::vec3& eyePos) { _eyePos = eyePos; }
glm::vec3 BlinnShader::worldSpaceViewDir(glm::vec3 worldPos) { return _eyePos - worldPos; }

Color BlinnShader::diffuseSample(const glm::vec2& uv) { return _material._diffuseMap->get(uv.x, uv.y); }
Color BlinnShader::normalSample(const glm::vec2& uv) { return _material._normalMap->get(uv.x, uv.y); }

int32_t BlinnShader::isInShadow(glm::vec4 depthPos, float n_dot_l) {
    // if (shader_data->enable_shadow && shader_data->shadow_map) {
    //    float widht  = shader_data->shadow_map->width;
    //    float height = shader_data->shadow_map->height;

    //    Vector3f ndc_coords;
    //    ndc_coords          = proj<3>(depth_pos / depth_pos[3]);
    //    Vector3f pos        = viewport_transform(widht, height, ndc_coords);
    //    float    depth_bias = 0.05f * (1 - n_dot_l);
    //    if (depth_bias < 0.005f)
    //        depth_bias = 0.01f;
    //    float current_depth = depth_pos[2] - depth_bias;

    //    if (pos.x < 0 || pos.y < 0 || pos.x >= widht || pos.y >= height)
    //        return 1;

    //    float closest_depth = shader_data->shadow_map->get_color(pos.x, pos.y).r;
    //    return current_depth < closest_depth;
    //}

    return 1;
}

shader_struct_v2f SkyBoxShader::vertex(shader_struct_a2v* a2v) {
    shader_struct_v2f v2f;
    v2f._clipPos   = object2ClipPos(a2v->_objPos);
    v2f._clipPos.z = v2f._clipPos.w;
    // std::cout << v2f._clipPos.x << " " << v2f._clipPos.y << " " << v2f._clipPos.z << std::endl;
    v2f._worldPos = a2v->_objPos;

    // for homogenous clipping
    _homogenousClip.in_clipcoord[a2v->_vertIndex]  = v2f._clipPos;
    _homogenousClip.in_worldcoord[a2v->_vertIndex] = v2f._worldPos;

    return v2f;
}

bool SkyBoxShader::fragment(shader_struct_v2f* v2f, Color& color) {
    color = cubemapSampling(v2f->_worldPos, _material._cubeMap);
    // color = Color(1.f, 0.f, 0.f, 1.f);

    return false;
}

glm::vec3 PBRShader::cubemapSampling(const glm::vec3& direction, CubeMap* cubeMap) {
    glm::vec2 uv;
    Color     color;
    int32_t   face_index = calCubeMapUV(direction, uv);
    TGAImage& map        = _material._cubeMap->faces[face_index];
    color                = map.get(uv.x * map.get_width(), uv.y * map.get_height());

    return glm::vec3(color[0], color[1], color[2]);
}

// core cubemap algorithm
int32_t PBRShader::calCubeMapUV(const glm::vec3& direction, glm::vec2& uv) {
    int   face_index = -1;
    float ma = 0, sc = 0, tc = 0;
    float abs_x = fabs(direction[0]), abs_y = fabs(direction[1]), abs_z = fabs(direction[2]);

    if (abs_x > abs_y && abs_x > abs_z) /* major axis -> x */
    {
        ma = abs_x;
        if (direction.x > 0) /* positive x */
        {
            face_index = 0;
            sc         = +direction.z;
            tc         = +direction.y;
        } else /* negative x */
        {
            face_index = 1;
            sc         = -direction.z;
            tc         = +direction.y;
        }
    } else if (abs_y > abs_z) /* major axis -> y */
    {
        ma = abs_y;
        if (direction.y > 0) /* positive y */
        {
            face_index = 2;
            sc         = +direction.x;
            tc         = +direction.z;
        } else /* negative y */
        {
            face_index = 3;
            sc         = +direction.x;
            tc         = -direction.z;
        }
    } else /* major axis -> z */
    {
        ma = abs_z;
        if (direction.z > 0) /* positive z */
        {
            face_index = 4;
            sc         = -direction.x;
            tc         = +direction.y;
        } else /* negative z */
        {
            face_index = 5;
            sc         = +direction.x;
            tc         = +direction.y;
        }
    }

    uv[0] = (sc / ma + 1.0f) / 2.0f;
    uv[1] = (tc / ma + 1.0f) / 2.0f;

    return face_index;
}

// PBR shader
shader_struct_v2f PBRShader::vertex(shader_struct_a2v* a2v) {
    shader_struct_v2f v2f;
    v2f._clipPos     = object2ClipPos(a2v->_objPos);
    v2f._worldPos    = object2WorldPos(a2v->_objPos);
    v2f._worldNormal = object2WorldNormal(a2v->_objNormal);
    v2f._uv          = a2v->_uv;

    // for homogenous clipping
    _homogenousClip.in_clipcoord[a2v->_vertIndex]  = v2f._clipPos;
    _homogenousClip.in_worldcoord[a2v->_vertIndex] = v2f._worldPos;
    _homogenousClip.in_normal[a2v->_vertIndex]     = v2f._worldNormal;
    _homogenousClip.in_uv[a2v->_vertIndex]         = v2f._uv;

    // tangent
    _tangent = a2v->_tangent;
    _bitangent = a2v->_bitangent;

    return v2f;
}

glm::vec3 PBRShader::getNormalFromMap(shader_struct_v2f* v2f) {
    glm::vec3 N = normalize(v2f->_worldNormal);
    glm::vec3 T = normalize(object2WorldNormal(_tangent));
    T           = normalize(T - dot(T, N) * N);
    glm::vec3 B = cross(N, T);

    glm::mat3 TBN(T, B, N);

    Color normalColor = normalSample(v2f->_uv);
    glm::vec3 tangentNormal(normalColor[0] * 2.f - 1.f, normalColor[1] * 2.f - 1.f, normalColor[2] * 2.f - 1.f);
    
    return normalize(tangentNormal * TBN);
}


// Learn OpenGL°æ±¾
bool PBRShader::fragment(shader_struct_v2f* v2f, Color& color) {
    glm::vec3 CookTorranceBrdf;
    glm::vec3 lightPos(2.f, 1.5f, 5.f);
    glm::vec3 ligthtColor(1.f, 1.f, 1.f);

    const auto& uv       = v2f->_uv;
    const auto& worldpos = v2f->_worldPos;
    //const auto& normal   = getNormalFromMap(v2f);
    const auto& normal   = v2f->_worldNormal;


    glm::vec3 l = glm::normalize(lightPos - worldpos);
    glm::vec3 v = glm::normalize(worldSpaceViewDir(worldpos));
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 h = glm::normalize(l + v);

    float n_dot_l = std::max(dot(n, l), 0.f);
    if (n_dot_l > 0.f) {
        float n_dot_v = std::max(dot(n, v), 0.f);
        float n_dot_h = std::max(dot(n, h), 0.f);
        float h_dot_v = std::max(dot(h, v), 0.f);

        // get albedo
        Color     albed = diffuseSample(uv);
        glm::vec3 albedo(albed[0], albed[1], albed[2]);

        float roughness = roughnessSample(uv);
        float metalness = metalnessSample(uv);
        float occlusion = occlusionSample(uv);

        float distance    = (lightPos - worldpos).length();
        float attenuation = 1.0 / (distance * distance);

        glm::vec3 radiance = ligthtColor * attenuation;

        glm::vec3 temp(0.04, 0.04, 0.04);
        glm::vec3 f0 = glm::mix(temp, albedo, metalness);

        float     NDF = distributionGGX(n_dot_h, roughness);
        float     G   = geometry_Smith(n_dot_v, n_dot_l, roughness);
        glm::vec3 F   = fresenlschlick(h_dot_v, f0);

        glm::vec3 numerator   = NDF * G * F;
        float     denominator = 4.0 * n_dot_v * n_dot_l + 0.0001; // +0.0001 to prevent divide by zero
        glm::vec3 specular    = numerator / denominator;

        glm::vec3 KS = F;
        glm::vec3 kD = (glm::vec3(1.0f) - F) * (1 - metalness);

        glm::vec3 Lo      = (kD * albedo / PI + specular) * radiance * n_dot_l;
        glm::vec3 ambient = 0.05f * albedo * occlusion;
        color             = Lo + ambient;
        Reinhard_mapping(color);
    }

    return false;
}

// bool PBRShader::fragment(shader_struct_v2f* v2f, Color& color) {
//    glm::vec3 CookTorranceBrdf;
//    glm::vec3 lightPos(2.f, 1.5f, 5.f);
//    glm::vec3 radiance(3.f, 3.f, 3.f);
//
//    const auto& uv       = v2f->_uv;
//    const auto& worldpos = v2f->_worldPos;
//    const auto& normal   = v2f->_worldNormal;
//
//    glm::vec3 v = glm::normalize(worldSpaceViewDir(worldpos));
//    glm::vec3 n = glm::normalize(normal);
//
//    float n_dot_v = std::max(dot(n, v), 0.f);
//    if (n_dot_v > 0.f) {
//
//        float roughness = roughnessSample(uv);
//        float metalness = metalnessSample(uv);
//        float occlusion = occlusionSample(uv);
//        glm::vec3  emission  = emissionSample(uv);
//
//        // get albedo
//        Color     albed = diffuseSample(uv);
//        glm::vec3 albedo(albed[0], albed[1], albed[2]);
//
//        glm::vec3 temp(0.04, 0.04, 0.04);
//        glm::vec3 temp2 = glm::vec3(1.0f, 1.0f, 1.0f);
//        glm::vec3 f0 = glm::mix(temp, albedo, metalness);
//
//        glm::vec3 F  = fresenlschlick_roughness(n_dot_v, f0, roughness);
//        glm::vec3 kD = (glm::vec3(1.0, 1.0, 1.0) - F) * (1 - metalness);
//
//        // diffuse color
//        glm::vec3 irradiance = cubemapSampling(n, _material._cubeMap);
//        for (int i = 0; i < 3; i++)
//            irradiance[i] = pow(irradiance[i], 2.0f);
//        glm::vec3 diffuse = irradiance * kD * albedo;
//
//        // specular color
//        glm::vec r = glm::normalize(2.0f * glm::dot(v, n) * n - v);
//        glm::vec2 lut_uv = glm::vec2(n_dot_v, roughness);
//
//
//        CookTorranceBrdf = NDF * G * F / (float)(4.0 * n_dot_l * n_dot_v + 0.0001);
//
//        glm::vec3 Lo      = (kD * albedo / PI + CookTorranceBrdf) * radiance * n_dot_l;
//        glm::vec3 ambient = 0.05f * albedo;
//        color             = Lo + ambient;
//        Reinhard_mapping(color);
//    }
//
//    return false;
//}

float PBRShader::roughnessSample(const glm::vec2& uv) {
    Color ret = _material._roughnessMap->get(uv.x, uv.y);
    return ret[2];
}

float PBRShader::metalnessSample(const glm::vec2& uv) {
    Color ret = _material._metalnessMap->get(uv.x, uv.y);
    return ret[2];
}

float PBRShader::occlusionSample(const glm::vec2& uv) {
    Color ret = _material._metalnessMap->get(uv.x, uv.y);
    return ret[2];
}
glm::vec3 PBRShader::emissionSample(const glm::vec2& uv) {
    Color ret = _material._metalnessMap->get(uv.x, uv.y);
    return glm::vec3(ret[0], ret[1], ret[2]);
}

} // namespace ABraveFish