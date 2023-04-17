#pragma once

#include "Core/Color.h"
#include "Core/Image.h"

#include "RenderBuffer.h"

#include "Core/Macros.h"
#include "Model.h"
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
};

struct Material {
    TGAImage* _diffuseMap;
    TGAImage* _normalMap;
    TGAImage* _specularMap;
    Color     color; // 暂时不知道用来干啥的
    Color     specular;
    float     gloss;

    CubeMap* _cubeMap;
    // float     bump_scale;
};

struct shader_struct_a2v {
    glm::vec3 _objPos;
    glm::vec3 _objNormal;
    glm::vec2 _uv;

    glm::mat4 _model;
    glm::mat4 _view;
    glm::mat4 _projection;
};

struct shader_struct_v2f {
    glm::vec4 _clipPos;
    glm::vec3 _worldPos;
    glm::vec3 _worldNormal;
    glm::vec2 _uv;
    float     _intensity;
    float     _screenDepth;
};

enum class ShaderType { None = 0, BlinnShader = 1, SkyBoxShader = 2 };
static ShaderType shaderType = ShaderType::BlinnShader;

class Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v)                 = 0;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) = 0;

    void setTransform(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection,
                      const glm::mat4& modelInv);
    void setMaterial(const Material& material);
    void setSkyBox(CubeMap* cubeMap);

    glm::vec4 object2ClipPos(const glm::vec3& objPos);
    glm::vec3 object2WorldPos(const glm::vec3& objPos);
    glm::vec3 object2WorldNormal(const glm::vec3& objNormal);

protected:
    Transform _transform;
    Material  _material;

    // TODO: 用户配置
};

struct DrawData {
    Model*        _model;
    float*        _zBuffer;
    RenderBuffer* _rdBuffer = nullptr;
    // TGAImage* _image;

    Ref<Shader> _shader;
    Transform   _transform;
};

class BlinnShader : public Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) override;

    void      setLightData(const glm::vec3& dir, const Color& color);
    glm::vec3 worldSpaceViewDir(glm::vec3 worldPos);

    Color diffuseSample(const glm::vec2& uv);

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

class SkyBoxShader : public Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) override;

protected:
    Color   cubemapSampling(const glm::vec3& direction, CubeMap* cubeMap);
    int32_t calCubeMapUV(const glm::vec3& direction, glm::vec2& uv);
};

class PBRShader : public Shader {
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
    }

    return nullptr;
}

} // namespace ABraveFish