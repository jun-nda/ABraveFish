#pragma once

#include "Core/Color.h"
#include "Core/Image.h"

#include "RenderBuffer.h"
#include "RenderSystem.h"

#include "glm/glm.hpp"

namespace ABraveFish {

struct Material {
    TGAImage* _diffuseMap;
    TGAImage* _normalMap;
    TGAImage* _specularMap;
    Color     color; // 暂时不知道用来干啥的
    Color     specular;
    float     gloss;
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

enum class ShaderType { None = 0, BlinnShader = 1 };
constexpr static ShaderType shaderType = ShaderType::BlinnShader;

class Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v)                 = 0;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) = 0;

    void setTransform(glm::mat4 model, glm::mat4 view, glm::mat4 projection);

    glm::vec4 object2ClipPos(glm::vec3 objPos);
    glm::vec3 object2WorldPos(glm::vec3 objPos);

protected:
    Transform _transform;
    Material  _material;
};

class BlinnShader : public Shader {
public:
    virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
    virtual bool              fragment(shader_struct_v2f* v2f, Color& color) override;

    glm::vec3 worldSpaceViewDir(glm::vec3 worldPos);

    Color diffuseSample(const glm::vec2& uv);

    bool isInShadow(glm::vec4 depthPos, float n_dot_l);

private:
    glm::vec3 _lightDir;
    Color     _ligthColor;
    glm::vec3 _eyePos;

    glm::mat4 _lightVP;
};

static Ref<Shader> Create() {
    switch (shaderType) {
        case ShaderType::None:
            return nullptr;
        case ShaderType::BlinnShader:
            return CreateRef<BlinnShader>();
    }

    return nullptr;
}

} // namespace ABraveFish