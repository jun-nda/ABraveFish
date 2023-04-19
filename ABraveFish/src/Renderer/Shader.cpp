#include "Shader.h"
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
    //Color     albedo         =  _material.color;


    Color     ambient = glm::dot(_material.color, albedo);
    float     n_dot_l = saturate(glm::dot(worldNormalDir, _lightDir));
    Color     diffuse = _ligthColor * albedo * n_dot_l;
    glm::vec3 viewDir = glm::normalize(worldSpaceViewDir(v2f->_worldPos));
    glm::vec3 halfDir = glm::normalize(viewDir + _lightDir);
    Color     spcular =
        _ligthColor * _material.specular * std::pow(saturate(glm::dot(worldNormalDir, halfDir)), _material.gloss);

    // glm::vec4 depth_pos = _lightVP * glm::vec4(v2f->_worldPos, 1.f);
    color = ambient + (diffuse + spcular);
    //color = diffuse + spcular; 
    color = albedo;

     //color               = Color(255, 255, 255);
    return false;
}

void BlinnShader::setLightData(const glm::vec3& dir, const Color& color) { _lightDir = dir, _ligthColor = color; }

glm::vec3 BlinnShader::worldSpaceViewDir(glm::vec3 worldPos) { return _eyePos - worldPos; }

Color BlinnShader::diffuseSample(const glm::vec2& uv) { return _material._diffuseMap->get(uv.x, uv.y); }

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

Color SkyBoxShader::cubemapSampling(const glm::vec3& direction, CubeMap* cubeMap) {
    glm::vec2 uv;
    Color     color;
    int32_t   face_index = calCubeMapUV(direction, uv);
    TGAImage& map        = _material._cubeMap->faces[face_index];
    color                = map.get(uv.x * map.get_width(), uv.y * map.get_height());

    return color;
}

// core cubemap algorithm
int32_t SkyBoxShader::calCubeMapUV(const glm::vec3& direction, glm::vec2& uv) {
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
shader_struct_v2f PBRShader::vertex( shader_struct_a2v* a2v ) {
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

bool PBRShader::fragment(shader_struct_v2f* v2f, Color& color) { 
    glm::vec3 CookTorranceBrdf;
    glm::vec3 lightPos(2.f, 1.5f, 5.f);
    glm::vec3 radiance(3.f, 3.f, 3.f);
}

} // namespace ABraveFish