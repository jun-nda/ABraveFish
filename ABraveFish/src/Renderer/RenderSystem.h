#pragma once

#include "Core/Macros.h"
#include "Shader.h"
#include "Model.h"
#include "glm/glm.hpp"

namespace ABraveFish {
struct Transform {
    glm::mat4 _model;
    glm::mat4 _view;
    glm::mat4 _projection;
    Transform() {}
    Transform(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
        : _model(model)
        , _view(view)
        , _projection(projection) {}
};

struct DrawData {
    Model*       _model;
    Ref<Shader> _shader;
    Transform    _transform;
    float*       _zBuffer;
    TGAImage*    image;
};
} // namespace ABraveFish