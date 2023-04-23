#pragma once

#include "Core/Color.h"
#include "Core/Image.h"

#include "RenderBuffer.h"

#include "Core/Macros.h"
#include "Model.h"
#include "Camera.h"
#include "glm/glm.hpp"

namespace ABraveFish {

struct Transform {
    glm::mat4 _model;
    glm::mat4 _view;
    glm::mat4 _projection;
    glm::mat4 _modelInv;
    Transform() {}
    Transform(glm::mat4 model, glm::mat4 view, glm::mat4 projection, glm::mat4 modelInv)
        : _model(model)
        , _view(view)
        , _projection(projection)
        , _modelInv(modelInv) {}
    Transform(const Transform& transform) {
        _model      = transform._model;
        _view       = transform._view;
        _projection = transform._projection;
        _modelInv   = transform._modelInv;
    }
};

struct Material {
    TGAImage* _diffuseMap;
    TGAImage* _normalMap;
    TGAImage* _specularMap;
    TGAImage* _roughnessMap;
    TGAImage* _metalnessMap;
    TGAImage* _occlusionMap;
    TGAImage* _emissionMap;

    Color color; // 暂时不知道用来干啥的
    Color specular;
    float gloss;

    CubeMap* _cubeMap;
    // float     bump_scale;
};

// 齐次裁剪所用到的数据
struct HomogenousClip {
    glm::vec3 in_normal[MAX_VERTEX];
    glm::vec2 in_uv[MAX_VERTEX];
    glm::vec3 in_worldcoord[MAX_VERTEX];
    glm::vec4 in_clipcoord[MAX_VERTEX];

    glm::vec3 out_normal[MAX_VERTEX];
    glm::vec2 out_uv[MAX_VERTEX];
    glm::vec3 out_worldcoord[MAX_VERTEX];
    glm::vec4 out_clipcoord[MAX_VERTEX];
};

typedef enum { W_PLANE, X_RIGHT, X_LEFT, Y_TOP, Y_BOTTOM, Z_NEAR, Z_FAR } ClipPlane;

struct shader_struct_a2v {
    glm::vec3 _objPos;
    glm::vec3 _objNormal;
    glm::vec2 _uv;

    glm::mat4 _model;
    glm::mat4 _view;
    glm::mat4 _projection;

    int32_t _vertIndex;

    // for tangent space
    glm::vec3 _tangent;
    glm::vec3 _bitangent;
};

struct shader_struct_v2f {
    glm::vec4 _clipPos;
    glm::vec3 _worldPos;
    glm::vec3 _worldNormal;
    glm::vec2 _uv;
    
    // for tangent space
    glm::vec3 _tangent;
    glm::vec3 _bitangent;

    // float     _screenDepth;
};

enum class ShaderType { None = 0, BlinnShader = 1, SkyBoxShader = 2, PBRShader = 3 };
static ShaderType shaderType = ShaderType::BlinnShader;

class Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v)                 = 0;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) = 0;

    void setTransform(Transform* transform);
    void setMaterial(const Material& material);
    void setSkyBox(CubeMap* cubeMap);

    glm::vec4 object2ClipPos(const glm::vec3& objPos);
    glm::vec3 object2WorldPos(const glm::vec3& objPos);
    glm::vec3 object2WorldNormal(const glm::vec3& objNormal);

public:
    HomogenousClip _homogenousClip;

protected:
    Transform _transform;
    Material  _material;
    // TODO: 用户配置
};

struct DrawData {
    Model*        _model;
    Camera*       _camera;
    float*        _zBuffer;
    RenderBuffer* _rdBuffer = nullptr;
    Ref<Shader>   _shader;
    IBLMap*       _iblMap = nullptr;
};

class BlinnShader : public Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) override;

    void      setLightData(const glm::vec3& dir, const Color& color);
    void      setEyePos(const glm::vec3& eyePos);
    glm::vec3 worldSpaceViewDir(glm::vec3 worldPos);

    Color diffuseSample(const glm::vec2& uv);
    Color normalSample(const glm::vec2& uv);

    int32_t isInShadow(glm::vec4 depthPos, float n_dot_l);

private:
    glm::vec3 _lightDir;
    Color     _ligthColor;
    glm::vec3 _eyePos;

    glm::mat4 _lightVP;
};

class ShadowShader : public Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) override;
};

class PBRShader : public BlinnShader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) override;

protected:
    float     roughnessSample(const glm::vec2& uv);
    float     metalnessSample(glm::vec2 uv);
    float     occlusionSample(const glm::vec2& uv);
    glm::vec3 emissionSample(const glm::vec2& uv);
    glm::vec3 brdfLutSample(glm::vec2& uv);

    glm::vec3 cubemapSampling(const glm::vec3& direction, CubeMap* cubeMap);



protected:
    int32_t calCubeMapUV(const glm::vec3& direction, glm::vec2& uv);
    glm::vec3 getNormalFromMap(shader_struct_v2f* v2f);

public:
    glm::vec3 _tangent;
    glm::vec3 _bitangent;

    IBLMap* _iblMap = nullptr;
};

class SkyBoxShader : public PBRShader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) override;
};

static Ref<Shader> Create() {
    switch (shaderType) {
        case ShaderType::None:
            return nullptr;
        case ShaderType::BlinnShader:
            return CreateRef<BlinnShader>();
        case ShaderType::SkyBoxShader:
            return CreateRef<SkyBoxShader>();
        case ShaderType::PBRShader:
            return CreateRef<PBRShader>();
    }

    return nullptr;
}

} // namespace ABraveFish