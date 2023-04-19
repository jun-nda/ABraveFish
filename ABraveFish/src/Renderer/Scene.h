#pragma once

#include <functional>

#include "Core/Macros.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBuffer.h"
#include "Renderer/RenderDevice.h"
#include "Renderer/Shader.h"

namespace ABraveFish {

glm::vec3 light_dir(2, 2, 2); // define light_dir

typedef std::function<void(Model**, DrawData*, Transform*, float*, shader_struct_a2v*, int32_t&, int32_t&, int32_t&)>
    buildSceneFunc;

buildSceneFunc buildQiyanaScene = [](Model** model, DrawData* drawData, Transform* transform, float* zBuffer,
                                     shader_struct_a2v* a2v, int32_t& modelNum, int32_t& vertexNum, int32_t& faceNum) {
    modelNum                = 3;
    const char* fileNames[] = {"obj/qiyana/qiyanabody.obj", "obj/qiyana/qiyanaface.obj", "obj/qiyana/qiyanahair.obj"};

    shaderType        = ShaderType::BlinnShader;
    drawData->_shader = Create();

    vertexNum = 0, faceNum = 0;

    for (int i = 0; i < modelNum; i++) {
        if (!model[i])
            model[i] = new Model(fileNames[i]);

        vertexNum += model[i]->getVertCount();
        faceNum += model[i]->getFaceCount();

        drawData->_model = model[i];
        drawData->_shader->setMaterial({&model[i]->_diffuseMap, &model[i]->_normalMap, &model[i]->_specularMap, nullptr,
                                        nullptr, nullptr, nullptr, Color(0.8f, 0.8f, 0.8f), Color(0.8f, 0.8f, 0.8f),
                                        50});
        std::dynamic_pointer_cast<BlinnShader>(drawData->_shader)->setLightData(light_dir, Color(1.f, 1.f, 1.f));

        drawData->_shader->setTransform(transform);

        drawData->_zBuffer = zBuffer;

        // draw call entrypoint
        vertexProcessing(drawData, a2v);
    }
};

buildSceneFunc buldFuhuaScene = [](Model** model, DrawData* drawData, Transform* transform, float* zBuffer,
                                   shader_struct_a2v* a2v, int32_t& modelNum, int32_t& vertexNum, int32_t& faceNum) {
    modelNum                = 4;
    const char* fileNames[] = {"obj/fuhua/fuhuabody.obj", "obj/fuhua/fuhuacloak.obj", "obj/fuhua/fuhuaface.obj",
                               "obj/fuhua/fuhuahair.obj"};

    shaderType        = ShaderType::BlinnShader;
    drawData->_shader = Create();

    vertexNum = 0, faceNum = 0;

    for (int i = 0; i < modelNum; i++) {
        if (!model[i])
            model[i] = new Model(fileNames[i]);

        vertexNum += model[i]->getVertCount();
        faceNum += model[i]->getFaceCount();

        drawData->_model = model[i];
        drawData->_shader->setMaterial({&model[i]->_diffuseMap, &model[i]->_normalMap, &model[i]->_specularMap, nullptr,
                                        nullptr, nullptr, nullptr, Color(0.8f, 0.8f, 0.8f), Color(0.8f, 0.8f, 0.8f),
                                        50});
        std::dynamic_pointer_cast<BlinnShader>(drawData->_shader)->setLightData(light_dir, Color(1.f, 1.f, 1.f));

        drawData->_shader->setTransform(transform);

        drawData->_zBuffer = zBuffer;

        // draw call entrypoint
        vertexProcessing(drawData, a2v);
    }
};

buildSceneFunc buldHelmetScene = [](Model** model, DrawData* drawData, Transform* transform, float* zBuffer,
                                    shader_struct_a2v* a2v, int32_t& modelNum, int32_t& vertexNum, int32_t& faceNum) {
    modelNum                = 2;
    const char* fileNames[] = {"obj/helmet/helmet.obj", "obj/skybox4/box.obj"};

    shaderType        = ShaderType::PBRShader;
    drawData->_shader = Create();

    vertexNum = 0, faceNum = 0;

    bool isSkyBox = false;
    for (int i = 0; i < modelNum; i++) {
        isSkyBox = i == modelNum - 1;
        if (!model[i])
            model[i] = new Model(fileNames[i], isSkyBox, i == 0);

        vertexNum += model[i]->getVertCount();
        faceNum += model[i]->getFaceCount();

        if (isSkyBox) {
            shaderType        = ShaderType::SkyBoxShader;
            drawData->_shader = Create();
            drawData->_shader->setSkyBox(&model[i]->_enviromentMap);
            transform->_view = glm::mat4(glm::mat3(transform->_view)); // È¥µôtranslate

        } else {
            drawData->_shader->setMaterial({&model[i]->_diffuseMap, &model[i]->_normalMap, &model[i]->_specularMap,
                                            &model[i]->_roughnessMap, &model[i]->_metalnessMap,
                                            &model[i]->_occlusionMap, &model[i]->_emissionMap, Color(0.6f, 0.6f, 0.6f),
                                            Color(0.6f, 0.6f, 0.6f), 50});
            std::dynamic_pointer_cast<BlinnShader>(drawData->_shader)->setLightData(light_dir, Color(1.f, 1.f, 1.f));
        }

        drawData->_model   = model[i];
        drawData->_zBuffer = zBuffer;
        drawData->_shader->setTransform(transform);

        // draw call entrypoint
        vertexProcessing(drawData, a2v, isSkyBox);
    }
};

} // namespace ABraveFish