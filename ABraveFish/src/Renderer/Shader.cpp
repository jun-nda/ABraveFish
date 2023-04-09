#include "Shader.h"

#define VEC4(pos) glm::vec4(pos, 1.f)
namespace ABraveFish {
static float            saturate(float f) { return f < 0 ? 0 : (f > 1 ? 1 : f); }
static glm::vec3 color2Vec3(Color color) { return glm::vec3(color.r, color.g, color.b); }

static Color AMBIENT = Color(54.f / 255, 58.f / 255, 66.f / 255);
void         Shader::setTransform(glm::mat4 model, glm::mat4 view, glm::mat4 projection, glm::mat4 modelInv) {
    _transform = Transform(model, view, projection, modelInv);
}

void Shader::setMaterial(const Material& material) { _material = material; }

glm::vec4 Shader::object2ClipPos(const glm::vec3& objPos) {
    return VEC4(objPos) * _transform._model * _transform._view * _transform._projection;
}

glm::vec3 Shader::object2WorldPos(const glm::vec3& objPos) { return glm::vec3(VEC4(objPos) * _transform._model); }
glm::vec3 Shader::object2WorldNormal(const glm::vec3& objNormal) {
    return glm::vec3(objNormal * glm::mat3(glm::transpose(_transform._modelInv)));
}

shader_struct_v2f BlinnShader::vertex(shader_struct_a2v* a2v) {
    shader_struct_v2f v2f;
    v2f._clipPos  = object2ClipPos(a2v->_objPos);
    v2f._worldPos = object2WorldPos(a2v->_objPos);
    v2f._worldNormal = object2WorldNormal(a2v->_objNormal);
    v2f._uv = a2v->_uv;
    return v2f;
}

bool BlinnShader::fragment(shader_struct_v2f* v2f, Color& color) {
    glm::vec3 worldNormalDir = glm::normalize(v2f->_worldNormal);
    Color     albedo         = diffuseSample(v2f->_uv) * _material.color;

    Color     ambient = glm::dot(_material.color, albedo);
    float     n_dot_l = saturate(glm::dot(worldNormalDir, _lightDir));
    Color     diffuse = _ligthColor * albedo * n_dot_l;
    glm::vec3 viewDir = glm::normalize(worldSpaceViewDir(v2f->_worldPos));
    glm::vec3 halfDir = glm::normalize(viewDir + _lightDir);
    Color     spcular =
        _ligthColor * _material.specular * std::pow(saturate(glm::dot(worldNormalDir, halfDir)), _material.gloss);

    glm::vec4 depth_pos = _lightVP * glm::vec4(v2f->_worldPos, 1.f);
    int       shadow    = isInShadow(depth_pos, n_dot_l);
    shadow              = 1;
    color = ambient + (diffuse + spcular) * shadow;
    //color               = Color(255, 255, 255);
    return false;
}

void BlinnShader::setLightData(const glm::vec3& dir, const Color& color) { _lightDir = dir, _ligthColor = color; }

glm::vec3 BlinnShader::worldSpaceViewDir(glm::vec3 worldPos) { return _eyePos - worldPos; }

Color BlinnShader::diffuseSample(const glm::vec2& uv) { return _material._diffuseMap->get(uv.x, uv.y); }

} // namespace ABraveFish