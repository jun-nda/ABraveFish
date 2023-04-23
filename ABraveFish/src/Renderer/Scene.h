#pragma once

#include <functional>

#include "Core/Macros.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBuffer.h"
#include "Renderer/RenderDevice.h"
#include "Renderer/Shader.h"
#include "Util.h"

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
        std::dynamic_pointer_cast<BlinnShader>(drawData->_shader)->setEyePos(drawData->_camera->eye);

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
        std::dynamic_pointer_cast<BlinnShader>(drawData->_shader)->setEyePos(drawData->_camera->eye);

        drawData->_shader->setTransform(transform);

        drawData->_zBuffer = zBuffer;

        // draw call entrypoint
        vertexProcessing(drawData, a2v);
    }
};

TGAImage* texture_from_file(const char* file_name) {
    TGAImage* texture = new TGAImage();
    texture->read_tga_file(file_name);
    texture->flip_vertically();
    return texture;
}

CubeMap* cubeMapFromFiles( const char* positive_x, const char* negative_x, const char* positive_y, const char* negative_y,
    const char* positive_z, const char* negative_z ) {
    CubeMap* cubemap   = new CubeMap();
    cubemap->faces[0]  = texture_from_file(positive_x);
    cubemap->faces[1]  = texture_from_file(negative_x);
    cubemap->faces[2]  = texture_from_file(positive_y);
    cubemap->faces[3]  = texture_from_file(negative_y);
    cubemap->faces[4]  = texture_from_file(positive_z);
    cubemap->faces[5]  = texture_from_file(negative_z);
    return cubemap;
}

void loadIBLMap( DrawData* drawData, const char* path ) {
    auto        shader = std::dynamic_pointer_cast<PBRShader>(drawData->_shader);
    int         i, j;
    IBLMap*     iblmap   = new IBLMap();
    const char* faces[6] = {"px", "nx", "py", "ny", "pz", "nz"};
    char        paths[6][256];

    iblmap->_mipLevels = 10;

    /* diffuse environment map */
    for (j = 0; j < 6; j++) {
        sprintf_s(paths[j], "%s/i_%s.tga", path, faces[j]);
    }
    iblmap->_irradianceMap = cubeMapFromFiles(paths[0], paths[1], paths[2], paths[3], paths[4], paths[5]);

    /* specular environment maps */
    for (i = 0; i < iblmap->_mipLevels; i++) {
        for (j = 0; j < 6; j++) {
            sprintf_s(paths[j], "%s/m%d_%s.tga", path, i, faces[j]);
        }
        iblmap->_preFilter_maps[i] = cubeMapFromFiles(paths[0], paths[1], paths[2], paths[3], paths[4], paths[5]);
    }

    /* brdf lookup texture */
    iblmap->_brdfLut = texture_from_file("obj/common/BRDF_LUT.tga");

    shader->_iblMap = iblmap;
    drawData->_iblMap = iblmap;
}

buildSceneFunc buldHelmetScene = [](Model** model, DrawData* drawData, Transform* transform, float* zBuffer,
                                    shader_struct_a2v* a2v, int32_t& modelNum, int32_t& vertexNum, int32_t& faceNum) {
    modelNum                = 2;
    const char* fileNames[] = {"obj/helmet/helmet.obj", "obj/skybox4/box.obj"};

    shaderType        = ShaderType::PBRShader;
    drawData->_shader = Create();

    vertexNum = 0, faceNum = 0;

    bool isSkyBox = false;
    auto shader   = std::dynamic_pointer_cast<PBRShader>(drawData->_shader);
    for (int i = 0; i < modelNum; i++) {
        isSkyBox = i == modelNum - 1;
        if (!model[i])
            model[i] = new Model(fileNames[i], isSkyBox, i == 0);

        vertexNum += model[i]->getVertCount();
        faceNum += model[i]->getFaceCount();

        if (isSkyBox) {
            shaderType        = ShaderType::SkyBoxShader;
            drawData->_shader = Create();
            drawData->_shader->setSkyBox(model[i]->_enviromentMap);
            transform->_view = glm::mat4(glm::mat3(transform->_view)); // È¥µôtranslate

        } else {
            drawData->_shader->setMaterial({&model[i]->_diffuseMap, &model[i]->_normalMap, &model[i]->_specularMap,
                                            &model[i]->_roughnessMap, &model[i]->_metalnessMap,
                                            &model[i]->_occlusionMap, &model[i]->_emissionMap, Color(0.6f, 0.6f, 0.6f),
                                            Color(0.6f, 0.6f, 0.6f), 50});
            shader->setLightData(light_dir, Color(1.f, 1.f, 1.f));
            shader->setEyePos(drawData->_camera->eye);
        }

        drawData->_model   = model[i];
        drawData->_zBuffer = zBuffer;
        drawData->_shader->setTransform(transform);

        if (!drawData->_iblMap)
            loadIBLMap(drawData, "obj/common2");
        else
            shader->_iblMap = drawData->_iblMap;

        // draw call entrypoint
        vertexProcessing(drawData, a2v, isSkyBox);
    }
};

} // namespace ABraveFish