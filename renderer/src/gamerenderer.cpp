// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gamerenderer.h"

#include <array>

#include <glm/ext/matrix_clip_space.hpp>
#include <physis.hpp>
#include <spirv_glsl.hpp>

#include "camera.h"
#include "dxbc_reader.h"
#include "rendermanager.h"
#include "swapchain.h"

#include <magic_enum/include/magic_enum.hpp>

// TODO: maybe need UV?
// note: SQEX passes the vertice positions as UV coordinates (yes, -1 to 1.) the shaders then transform them back with the g_CommonParameter.m_RenderTarget vec4
const std::vector<glm::vec4> planeVertices = {
    {1.0f, 1.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f, 1.0f, 1.0f},
    {-1.0f, -1.0f, 1.0f, 1.0f},

    {-1.0f, -1.0f, 1.0f, 1.0f},
    {1.0f, -1.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
};

dxvk::Logger dxvk::Logger::s_instance("dxbc.log");

const std::array<std::string, 14> passes = {
    // Shadows?
    /* 0 */ "PASS_0",
    // Z "prepass"
    /* 1 */ "PASS_Z_OPAQUE",
    // computes and stores normals (TODO: denote how these normals are special)
    /* 2 */ "PASS_G_OPAQUE",
    // g run for each light
    // takes view pos, then unknown texture and normal
    /* 3 */ "PASS_LIGHTING_OPAQUE",
    /* 4 */ "PASS_G_SEMITRANSPARENCY",
    /* 5 */ "PASS_COMPOSITE_OPAQUE",
    /* 6 */ "PASS_7",
    /* 7 */ "PASS_WATER",
    /* 8 */ "PASS_WATER_Z",
    /* 9 */ "PASS_SEMITRANSPARENCY",
    /* 10 */ "PASS_COMPOSITE_SEMITRANSPARENCY",
    /* 11 */ "PASS_10",
    /* 12 */ "PASS_12",
    /* 13 */ "PASS_14"};

const int INVALID_PASS = 255;

GameRenderer::GameRenderer(Device &device, GameData *data)
    : m_device(device)
    , m_data(data)
    , m_shaderManager(device)
{
    auto repositories = physis_gamedata_get_repositories(data);
    for (int i = 0; i < repositories.repositories_count; i++) {
        if (strcmp(repositories.repositories[i].name, "ex5") == 0) {
            qInfo() << "The renderer is switching to Dawntrail mode. Please file a bug if you think this was a mistake.";
            m_dawntrailMode = true;
        }
    }
    physis_gamedata_free_repositories(repositories);

    m_dummyTex = m_device.createDummyTexture();
    m_dummyBuffer = m_device.createDummyBuffer();

    if (!m_dawntrailMode) {
        m_tileNormal =
            m_device.addGameTexture(VK_FORMAT_R8G8B8A8_UNORM, physis_texture_parse(physis_gamedata_extract_file(m_data, "chara/common/texture/-tile_n.tex")));
        m_device.nameTexture(m_tileNormal, "chara/common/texture/-tile_n.tex");
        m_tileDiffuse =
            m_device.addGameTexture(VK_FORMAT_R8G8B8A8_UNORM, physis_texture_parse(physis_gamedata_extract_file(m_data, "chara/common/texture/-tile_d.tex")));
        m_device.nameTexture(m_tileDiffuse, "chara/common/texture/-tile_d.tex");
    }

    size_t vertexSize = planeVertices.size() * sizeof(glm::vec4);
    m_planeVertexBuffer = m_device.createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_device.copyToBuffer(m_planeVertexBuffer, (void *)planeVertices.data(), vertexSize);

    directionalLightningShpk = physis_parse_shpk(physis_gamedata_extract_file(m_data, "shader/sm5/shpk/directionallighting.shpk"));
    createViewPositionShpk = physis_parse_shpk(physis_gamedata_extract_file(m_data, "shader/sm5/shpk/createviewposition.shpk"));
    backgroundShpk = physis_parse_shpk(physis_gamedata_extract_file(m_data, "shader/sm5/shpk/bg.shpk"));

    // camera data
    {
        g_CameraParameter = m_device.createBuffer(sizeof(CameraParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_CameraParameter, "g_CameraParameter");
    }

    // instance data
    {
        g_InstanceParameter = m_device.createBuffer(sizeof(InstanceParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_InstanceParameter, "g_InstanceParameter");

        InstanceParameter instanceParameter{};
        instanceParameter.g_InstanceParameter.m_MulColor = glm::vec4(1.0f);
        instanceParameter.g_InstanceParameter.m_EnvParameter = glm::vec4(1.0f);

        /*const float wetnessMin = 0.0f;
        const float wetnessMax = 1.0f;
        const float maybeWetness = 0.0f;

        instanceParameter.g_InstanceParameter.m_Wetness = {maybeWetness, 2.0f, wetnessMin, wetnessMax};
        instanceParameter.g_InstanceParameter.m_CameraLight.m_DiffuseSpecular = glm::vec4(1.0f);
        instanceParameter.g_InstanceParameter.m_CameraLight.m_Rim = glm::vec4(1.0f);*/

        m_device.copyToBuffer(g_InstanceParameter, &instanceParameter, sizeof(InstanceParameter));
    }

    // model data
    {
        g_ModelParameter = m_device.createBuffer(sizeof(ModelParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_ModelParameter, "g_ModelParameter");

        ModelParameter modelParameter{};
        modelParameter.g_ModelParameter.m_Params = glm::vec4(1.0f);
        m_device.copyToBuffer(g_ModelParameter, &modelParameter, sizeof(ModelParameter));
    }

    // material data
    {
        g_TransparencyMaterialParameter = m_device.createBuffer(sizeof(MaterialParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_TransparencyMaterialParameter, "g_TransparencyMaterialParameter");

        MaterialParameters materialParameter{};
        materialParameter.parameters[0] = glm::vec4(1.0f, 1.0f, 1.0f, 2.0f); // diffuse color then alpha threshold
        materialParameter.parameters[5].z = 1.0f;
        m_device.copyToBuffer(g_TransparencyMaterialParameter, &materialParameter, sizeof(MaterialParameters));
    }

    // light data
    {
        g_LightParam = m_device.createBuffer(sizeof(LightParam), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_LightParam, "g_LightParam");

        LightParam lightParam{};
        lightParam.m_Position = glm::vec4(-5);
        lightParam.m_Direction = glm::normalize(glm::vec4(0) - lightParam.m_Position);
        lightParam.m_DiffuseColor = glm::vec4(1);
        lightParam.m_SpecularColor = glm::vec4(1);
        lightParam.m_Attenuation = glm::vec4(5.0f);
        /*lightParam.m_ClipMin = glm::vec4(0.0f);
        lightParam.m_ClipMax = glm::vec4(5.0f);*/

        m_device.copyToBuffer(g_LightParam, &lightParam, sizeof(LightParam));
    }

    // common data
    {
        g_CommonParameter = m_device.createBuffer(sizeof(CommonParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_CommonParameter, "g_CommonParameter");
    }

    // scene data
    {
        g_SceneParameter = m_device.createBuffer(sizeof(SceneParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_SceneParameter, "g_SceneParameter");

        SceneParameter sceneParameter{};
        m_device.copyToBuffer(g_SceneParameter, &sceneParameter, sizeof(SceneParameter));
    }

    // customize data
    {
        g_CustomizeParameter = m_device.createBuffer(sizeof(CustomizeParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_CustomizeParameter, "g_CustomizeParameter");

        CustomizeParameter customizeParameter{};
        customizeParameter.m_SkinColor = glm::vec4(1.0f);
        customizeParameter.m_HairFresnelValue0 = glm::vec4(1.0f);
        customizeParameter.m_LipColor = glm::vec4(1.0f);
        customizeParameter.m_MainColor = glm::vec4(1.0f);
        customizeParameter.m_HairFresnelValue0 = glm::vec4(1.0f);
        customizeParameter.m_MeshColor = glm::vec4(1.0f);
        customizeParameter.m_LeftColor = glm::vec4(1.0f);
        customizeParameter.m_RightColor = glm::vec4(1.0f);
        customizeParameter.m_OptionColor = glm::vec4(1.0f);

        m_device.copyToBuffer(g_CustomizeParameter, &customizeParameter, sizeof(CustomizeParameter));
    }

    // material parameter dynamic
    {
        g_MaterialParameterDynamic = m_device.createBuffer(sizeof(MaterialParameterDynamic), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_MaterialParameterDynamic, "g_MaterialParameterDynamic");

        MaterialParameterDynamic materialParameterDynamic{};

        m_device.copyToBuffer(g_MaterialParameterDynamic, &materialParameterDynamic, sizeof(CustomizeParameter));
    }

    // decal color
    {
        g_DecalColor = m_device.createBuffer(sizeof(glm::vec4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_DecalColor, "g_DecalColor");

        glm::vec4 color{};

        m_device.copyToBuffer(g_DecalColor, &color, sizeof(glm::vec4));
    }

    // ambient params
    {
        g_AmbientParam = m_device.createBuffer(sizeof(AmbientParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_AmbientParam, "g_AmbientParam");

        AmbientParameters ambientParameters{};
        for (int i = 0; i < 6; i++) {
            ambientParameters.g_AmbientParam[i] = glm::vec4(0.1f);
        }
        for (int i = 0; i < 4; i++) {
            ambientParameters.g_AdditionalAmbientParam[i] = glm::vec4(1.0f);
        }

        m_device.copyToBuffer(g_AmbientParam, &ambientParameters, sizeof(AmbientParameters));
    }

    // shader type parameter
    if (m_dawntrailMode) {
        g_ShaderTypeParameter = m_device.createBuffer(sizeof(ShaderTypeParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_ShaderTypeParameter, "g_ShaderTypeParameter");

        ShaderTypeParameter shaderTypeParameter{};

        m_device.copyToBuffer(g_ShaderTypeParameter, &shaderTypeParameter, sizeof(ShaderTypeParameter));
    }

    // pbr common parameter
    if (m_dawntrailMode) {
        g_PbrParameterCommon = m_device.createBuffer(sizeof(PbrParameterCommon), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_PbrParameterCommon, "g_PbrParameterCommon");

        PbrParameterCommon pbrParameterCommon{};
        pbrParameterCommon.unk = {31.01465, 30.99805, 1.00, 0.00};

        m_device.copyToBuffer(g_PbrParameterCommon, &pbrParameterCommon, sizeof(PbrParameterCommon));
    }

    // worldViewMatrix
    if (m_dawntrailMode) {
        g_WorldViewMatrix = m_device.createBuffer(sizeof(WorldViewMatrix), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_device.nameBuffer(g_WorldViewMatrix, "g_WorldViewMatrix");
    }

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.maxAnisotropy = 1.0f;

    vkCreateSampler(m_device.device, &samplerInfo, nullptr, &m_sampler);

    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

    vkCreateSampler(m_device.device, &samplerInfo, nullptr, &m_normalSampler);

    createImageResources();
}

void GameRenderer::render(VkCommandBuffer commandBuffer, Camera &camera, Scene &scene, const std::vector<DrawObjectInstance> &models)
{
    Q_UNUSED(scene)

    // TODO: this shouldn't be here
    CameraParameter cameraParameter{};

    const glm::mat4 viewProjectionMatrix = camera.perspective * camera.view;

    // unknown
    cameraParameter.m_unknownMatrix = glm::mat4(1.0f);
    cameraParameter.m_unknown4 = glm::mat4(1.0f);

    cameraParameter.m_ViewMatrix = glm::transpose(camera.view);
    cameraParameter.m_InverseViewMatrix = cameraParameter.m_unknownMatrix;
    cameraParameter.m_ViewProjectionMatrix = glm::transpose(viewProjectionMatrix);
    cameraParameter.m_InverseViewProjectionMatrix = glm::transpose(glm::inverse(viewProjectionMatrix));

    // known params
    cameraParameter.m_InverseProjectionMatrix = glm::transpose(glm::inverse(viewProjectionMatrix));
    cameraParameter.m_ProjectionMatrix = glm::transpose(viewProjectionMatrix);

    cameraParameter.m_MainViewToProjectionMatrix = glm::transpose(viewProjectionMatrix);
    cameraParameter.m_EyePosition = glm::vec4(camera.position, 0.0f);
    cameraParameter.m_LookAtVector = glm::vec4(0.0f); // placeholder

    m_device.copyToBuffer(g_CameraParameter, &cameraParameter, sizeof(CameraParameter));

    WorldViewMatrix worldViewMatrix;
    worldViewMatrix.m_WorldViewMatrix = glm::mat4(1.0f);
    worldViewMatrix.m_EyePosition = glm::vec4(camera.position, 0.0f);

    m_device.copyToBuffer(g_WorldViewMatrix, &worldViewMatrix, sizeof(WorldViewMatrix));

    int i = 0;
    for (const auto &pass : passes) {
        // hardcoded to the known pass for now
        if (pass == "PASS_G_OPAQUE" || pass == "PASS_Z_OPAQUE") {
            beginPass(commandBuffer, pass);

            for (auto &model : models) {
                VkDebugUtilsLabelEXT labelExt{};
                labelExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                labelExt.pLabelName = model.name.toStdString().c_str();
                m_device.beginDebugMarker(commandBuffer, labelExt);

                // copy bone data
                {
                    const int jointMatrixSize = m_dawntrailMode ? JOINT_MATRIX_SIZE_DAWNTRAIL : JOINT_MATRIX_SIZE_ARR;
                    const size_t bufferSize = sizeof(glm::mat3x4) * jointMatrixSize;
                    void *mapped_data = nullptr;
                    vkMapMemory(m_device.device, model.sourceObject->boneInfoBuffer.memory, 0, bufferSize, 0, &mapped_data);

                    std::vector<glm::mat3x4> newBoneData(model.sourceObject->boneData.size());
                    for (int i = 0; i < jointMatrixSize; i++) {
                        newBoneData[i] = glm::transpose(model.sourceObject->boneData[i]);
                    }

                    memcpy(mapped_data, newBoneData.data(), bufferSize);

                    VkMappedMemoryRange range = {};
                    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                    range.memory = model.sourceObject->boneInfoBuffer.memory;
                    range.size = bufferSize;
                    vkFlushMappedMemoryRanges(m_device.device, 1, &range);

                    vkUnmapMemory(m_device.device, model.sourceObject->boneInfoBuffer.memory);
                }

                for (const auto &part : model.sourceObject->parts) {
                    RenderMaterial renderMaterial;
                    if (static_cast<size_t>(part.materialIndex + 1) > model.sourceObject->materials.size()) {
                        renderMaterial = model.sourceObject->materials[0]; // TODO: better fallback
                    } else {
                        renderMaterial = model.sourceObject->materials[part.materialIndex];
                    }

                    if (renderMaterial.shaderPackage.p_ptr == nullptr) {
                        qWarning() << "Invalid shader package!";
                        continue;
                    }

                    std::vector<uint32_t> systemKeys;
                    // Dawntrail has no more decode depth buffers it seems
                    if (renderMaterial.type == MaterialType::Skin && !m_dawntrailMode) {
                        systemKeys.push_back(physis_shpk_crc("DecodeDepthBuffer_RAWZ"));
                    }

                    std::vector<uint32_t> sceneKeys;
                    for (int j = 0; j < renderMaterial.shaderPackage.num_scene_keys; j++) {
                        auto id = renderMaterial.shaderPackage.scene_keys[j].id;

                        bool found = false;
                        if (id == physis_shpk_crc("TransformView")) {
                            if (model.sourceObject->skinned) {
                                sceneKeys.push_back(physis_shpk_crc("TransformViewSkin"));
                            } else {
                                sceneKeys.push_back(physis_shpk_crc("TransformViewRigid"));
                            }
                            found = true;
                        }

                        // Fall back to default if needed
                        if (!found) {
                            sceneKeys.push_back(renderMaterial.shaderPackage.scene_keys[j].default_value);
                        }
                    }

                    std::vector<uint32_t> materialKeys;
                    for (int j = 0; j < renderMaterial.shaderPackage.num_material_keys; j++) {
                        auto id = renderMaterial.shaderPackage.material_keys[j].id;

                        bool found = false;
                        for (uint32_t z = 0; z < renderMaterial.mat.num_shader_keys; z++) {
                            if (renderMaterial.mat.shader_keys[z].category == id) {
                                materialKeys.push_back(renderMaterial.mat.shader_keys[z].value);
                                found = true;
                            }
                        }

                        // Fall back to default if needed
                        if (!found) {
                            materialKeys.push_back(renderMaterial.shaderPackage.material_keys[j].default_value);
                        }
                    }
                    std::vector<uint32_t> subviewKeys = {physis_shpk_crc("Default"), physis_shpk_crc("SUB_VIEW_MAIN")};

                    const uint32_t selector = physis_shpk_build_selector_from_all_keys(systemKeys.data(),
                                                                                       systemKeys.size(),
                                                                                       sceneKeys.data(),
                                                                                       sceneKeys.size(),
                                                                                       materialKeys.data(),
                                                                                       materialKeys.size(),
                                                                                       subviewKeys.data(),
                                                                                       subviewKeys.size());
                    const physis_SHPKNode node = physis_shpk_get_node(&renderMaterial.shaderPackage, selector);

                    // check if invalid
                    if (node.pass_count == 0) {
                        continue;
                    }

                    // this is an index into the node's pass array, not to get confused with the global one we always follow.
                    const int passIndice = node.pass_indices[i];
                    if (passIndice != INVALID_PASS) {
                        const Pass currentPass = node.passes[passIndice];

                        const uint32_t vertexShaderIndice = currentPass.vertex_shader;
                        const uint32_t pixelShaderIndice = currentPass.pixel_shader;

                        physis_Shader vertexShader = renderMaterial.shaderPackage.vertex_shaders[vertexShaderIndice];
                        physis_Shader pixelShader = renderMaterial.shaderPackage.pixel_shaders[pixelShaderIndice];

                        auto &pipeline = bindPipeline(commandBuffer,
                                                      pass,
                                                      vertexShader,
                                                      pixelShader,
                                                      renderMaterial.mat.shpk_name,
                                                      &model.sourceObject->model,
                                                      &part.originalPart);
                        bindDescriptorSets(commandBuffer, pipeline, model.sourceObject, &renderMaterial, pass);

                        VkDeviceSize offsets[] = {0};
                        for (int j = 0; j < part.originalPart.num_streams; j++) {
                            vkCmdBindVertexBuffers(commandBuffer, j, 1, &part.streamBuffer[j].buffer, offsets);
                        }
                        vkCmdBindIndexBuffer(commandBuffer, part.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

                        vkCmdDrawIndexed(commandBuffer, part.numIndices, 1, 0, 0, 0);
                    }
                }

                m_device.endDebugMarker(commandBuffer);
            }

            endPass(commandBuffer);

            m_device.transitionTexture(commandBuffer, m_depthBuffer, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        } else if (pass == "PASS_LIGHTING_OPAQUE") {
            m_device.transitionTexture(commandBuffer, m_normalGBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // first we need to generate the view positions with createviewpositions
            beginPass(commandBuffer, "PASS_LIGHTING_OPAQUE_VIEWPOSITION");
            {
                std::vector<uint32_t> systemKeys = {};
                if (!m_dawntrailMode) {
                    systemKeys.push_back(physis_shpk_crc("DecodeDepthBuffer_RAWZ"));
                }
                std::vector<uint32_t> subviewKeys = {
                    physis_shpk_crc("Default"),
                    physis_shpk_crc("SUB_VIEW_MAIN"),
                };

                const uint32_t selector = physis_shpk_build_selector_from_all_keys(systemKeys.data(),
                                                                                   systemKeys.size(),
                                                                                   nullptr,
                                                                                   0,
                                                                                   nullptr,
                                                                                   0,
                                                                                   subviewKeys.data(),
                                                                                   subviewKeys.size());
                const physis_SHPKNode node = physis_shpk_get_node(&createViewPositionShpk, selector);

                // check if invalid
                if (node.pass_count == 0) {
                    continue;
                }

                const int passIndice = node.pass_indices[i];
                if (passIndice != INVALID_PASS) {
                    const Pass currentPass = node.passes[passIndice];

                    const uint32_t vertexShaderIndice = currentPass.vertex_shader;
                    const uint32_t pixelShaderIndice = currentPass.pixel_shader;

                    physis_Shader vertexShader = createViewPositionShpk.vertex_shaders[vertexShaderIndice];
                    physis_Shader pixelShader = createViewPositionShpk.pixel_shaders[pixelShaderIndice];

                    auto &pipeline = bindPipeline(commandBuffer,
                                                  "PASS_LIGHTING_OPAQUE_VIEWPOSITION",
                                                  vertexShader,
                                                  pixelShader,
                                                  "createviewposition.shpk",
                                                  nullptr,
                                                  nullptr);
                    bindDescriptorSets(commandBuffer, pipeline, nullptr, nullptr, pass);

                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_planeVertexBuffer.buffer, offsets);

                    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
                }
            }
            endPass(commandBuffer);

            m_device.transitionTexture(commandBuffer, m_viewPositionBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            beginPass(commandBuffer, pass);
            // then run the directionallighting shader
            {
                std::vector<uint32_t> systemKeys = {};
                if (!m_dawntrailMode) {
                    systemKeys.push_back(physis_shpk_crc("DecodeDepthBuffer_RAWZ"));
                }
                std::vector<uint32_t> sceneKeys = {
                    physis_shpk_crc("GetDirectionalLight_Enable"),
                    physis_shpk_crc("GetFakeSpecular_Disable"),
                    physis_shpk_crc("GetUnderWaterLighting_Disable"),
                };
                std::vector<uint32_t> subviewKeys = {
                    physis_shpk_crc("Default"),
                    physis_shpk_crc("SUB_VIEW_MAIN"),
                };

                const uint32_t selector = physis_shpk_build_selector_from_all_keys(systemKeys.data(),
                                                                                   systemKeys.size(),
                                                                                   sceneKeys.data(),
                                                                                   sceneKeys.size(),
                                                                                   nullptr,
                                                                                   0,
                                                                                   subviewKeys.data(),
                                                                                   subviewKeys.size());
                const physis_SHPKNode node = physis_shpk_get_node(&directionalLightningShpk, selector);

                // check if invalid
                if (node.pass_count == 0) {
                    continue;
                }

                const int passIndice = node.pass_indices[i];
                if (passIndice != INVALID_PASS) {
                    const Pass currentPass = node.passes[passIndice];

                    const uint32_t vertexShaderIndice = currentPass.vertex_shader;
                    const uint32_t pixelShaderIndice = currentPass.pixel_shader;

                    physis_Shader vertexShader = directionalLightningShpk.vertex_shaders[vertexShaderIndice];
                    physis_Shader pixelShader = directionalLightningShpk.pixel_shaders[pixelShaderIndice];

                    auto &pipeline = bindPipeline(commandBuffer, pass, vertexShader, pixelShader, "directionallighting.shpk", nullptr, nullptr);
                    bindDescriptorSets(commandBuffer, pipeline, nullptr, nullptr, pass);

                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_planeVertexBuffer.buffer, offsets);

                    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
                }
            }
            endPass(commandBuffer);

            m_device.transitionTexture(commandBuffer, m_lightBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            m_device.transitionTexture(commandBuffer,
                                       m_lightSpecularBuffer,
                                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        } else if (pass == "PASS_COMPOSITE_SEMITRANSPARENCY") {
            beginPass(commandBuffer, pass);

            for (auto &model : models) {
                for (const auto &part : model.sourceObject->parts) {
                    RenderMaterial renderMaterial;
                    if (static_cast<size_t>(part.materialIndex + 1) > model.sourceObject->materials.size()) {
                        renderMaterial = model.sourceObject->materials[0]; // TODO: better fallback
                    } else {
                        renderMaterial = model.sourceObject->materials[part.materialIndex];
                    }

                    if (renderMaterial.shaderPackage.p_ptr == nullptr) {
                        qWarning() << "Invalid shader package!";
                        continue;
                    }

                    std::vector<uint32_t> systemKeys;
                    if (renderMaterial.type == MaterialType::Skin && !m_dawntrailMode) {
                        systemKeys.push_back(physis_shpk_crc("DecodeDepthBuffer_RAWZ"));
                    }

                    std::vector<uint32_t> sceneKeys;
                    for (int j = 0; j < renderMaterial.shaderPackage.num_scene_keys; j++) {
                        auto id = renderMaterial.shaderPackage.scene_keys[j].id;

                        bool found = false;
                        if (id == physis_shpk_crc("TransformView")) {
                            if (model.sourceObject->skinned) {
                                sceneKeys.push_back(physis_shpk_crc("TransformViewSkin"));
                            } else {
                                sceneKeys.push_back(physis_shpk_crc("TransformViewRigid"));
                            }
                            found = true;
                        }

                        // Fall back to default if needed
                        if (!found) {
                            sceneKeys.push_back(renderMaterial.shaderPackage.scene_keys[j].default_value);
                        }
                    }

                    std::vector<uint32_t> materialKeys;
                    for (int j = 0; j < renderMaterial.shaderPackage.num_material_keys; j++) {
                        auto id = renderMaterial.shaderPackage.material_keys[j].id;

                        bool found = false;
                        for (uint32_t z = 0; z < renderMaterial.mat.num_shader_keys; z++) {
                            if (renderMaterial.mat.shader_keys[z].category == id) {
                                materialKeys.push_back(renderMaterial.mat.shader_keys[z].value);
                                found = true;
                            }
                        }

                        // Fall back to default if needed
                        if (!found) {
                            materialKeys.push_back(renderMaterial.shaderPackage.material_keys[j].default_value);
                        }
                    }
                    std::vector<uint32_t> subviewKeys = {physis_shpk_crc("Default"), physis_shpk_crc("SUB_VIEW_MAIN")};

                    const uint32_t selector = physis_shpk_build_selector_from_all_keys(systemKeys.data(),
                                                                                       systemKeys.size(),
                                                                                       sceneKeys.data(),
                                                                                       sceneKeys.size(),
                                                                                       materialKeys.data(),
                                                                                       materialKeys.size(),
                                                                                       subviewKeys.data(),
                                                                                       subviewKeys.size());
                    const physis_SHPKNode node = physis_shpk_get_node(&renderMaterial.shaderPackage, selector);

                    // check if invalid
                    if (node.pass_count == 0) {
                        continue;
                    }

                    // this is an index into the node's pass array, not to get confused with the global one we always follow.
                    const int passIndice = node.pass_indices[i];
                    if (passIndice != INVALID_PASS) {
                        const Pass currentPass = node.passes[passIndice];

                        const uint32_t vertexShaderIndice = currentPass.vertex_shader;
                        const uint32_t pixelShaderIndice = currentPass.pixel_shader;

                        physis_Shader vertexShader = renderMaterial.shaderPackage.vertex_shaders[vertexShaderIndice];
                        physis_Shader pixelShader = renderMaterial.shaderPackage.pixel_shaders[pixelShaderIndice];

                        auto &pipeline = bindPipeline(commandBuffer,
                                                      pass,
                                                      vertexShader,
                                                      pixelShader,
                                                      renderMaterial.mat.shpk_name,
                                                      &model.sourceObject->model,
                                                      &part.originalPart);
                        bindDescriptorSets(commandBuffer, pipeline, model.sourceObject, &renderMaterial, pass);

                        VkDeviceSize offsets[] = {0};
                        for (int j = 0; j < part.originalPart.num_streams; j++) {
                            vkCmdBindVertexBuffers(commandBuffer, j, 1, &part.streamBuffer[j].buffer, offsets);
                        }
                        vkCmdBindIndexBuffer(commandBuffer, part.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

                        vkCmdDrawIndexed(commandBuffer, part.numIndices, 1, 0, 0, 0);
                    }
                }
            }

            endPass(commandBuffer);

            m_device.transitionTexture(commandBuffer, m_compositeBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        i++;
    }
}

void GameRenderer::resize()
{
    // TODO: this is because of our terrible resource handling. an image referenced in these may be gone due to resizing, for example
    for (auto &[hash, cachedPipeline] : m_cachedPipelines) {
        cachedPipeline.cachedDescriptors.clear();
    }

    createImageResources();
}

void GameRenderer::beginPass(VkCommandBuffer commandBuffer, const std::string_view passName)
{
    VkDebugUtilsLabelEXT labelExt{};
    labelExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    labelExt.pLabelName = passName.data();
    m_device.beginDebugMarker(commandBuffer, labelExt);

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.extent = m_device.swapChain->extent;

    std::vector<VkRenderingAttachmentInfo> colorAttachments;
    VkRenderingAttachmentInfo depthStencilAttachment{};

    if (passName == "PASS_G_OPAQUE") {
        m_device.transitionTexture(commandBuffer, m_normalGBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // normals, it seems like
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_normalGBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }

        // test for another
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_motionGBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }

        // test for another
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_diffuseGBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }

        // test for another
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_unkGBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }

        // test for another
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_motionGBuffer2.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }

        m_device.transitionTexture(commandBuffer, m_depthBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        // depth
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_depthBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentInfo.clearValue.depthStencil.depth = 1.0f;

            depthStencilAttachment = attachmentInfo;
        }
    } else if (passName == "PASS_LIGHTING_OPAQUE") {
        m_device.transitionTexture(commandBuffer, m_lightBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // diffuse
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_lightBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }

        m_device.transitionTexture(commandBuffer, m_lightSpecularBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // specular?
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_lightSpecularBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }
    } else if (passName == "PASS_LIGHTING_OPAQUE_VIEWPOSITION") {
        m_device.transitionTexture(commandBuffer, m_viewPositionBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // TODO: Hack we should not be using a special pass for this, we should just design our API better
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_viewPositionBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }
    } else if (passName == "PASS_Z_OPAQUE") {
        m_device.transitionTexture(commandBuffer, m_ZBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        m_device.transitionTexture(commandBuffer, m_depthBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        // normals, it seems like
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_ZBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }
    } else if (passName == "PASS_COMPOSITE_SEMITRANSPARENCY") {
        m_device.transitionTexture(commandBuffer, m_compositeBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // composite
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_compositeBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }

        m_device.transitionTexture(commandBuffer, m_depthBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        // depth buffer for depth testing
        {
            VkRenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = m_depthBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            depthStencilAttachment = attachmentInfo;
        }
    }

    renderingInfo.layerCount = 1;
    renderingInfo.pColorAttachments = colorAttachments.data();
    renderingInfo.colorAttachmentCount = colorAttachments.size();

    if (depthStencilAttachment.imageView != VK_NULL_HANDLE) {
        renderingInfo.pDepthAttachment = &depthStencilAttachment;
    }

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
}

void GameRenderer::endPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRendering(commandBuffer);

    m_device.endDebugMarker(commandBuffer);
}

GameRenderer::CachedPipeline &GameRenderer::bindPipeline(VkCommandBuffer commandBuffer,
                                                         std::string_view passName,
                                                         physis_Shader &vertexShader,
                                                         physis_Shader &pixelShader,
                                                         std::string_view shaderName,
                                                         const physis_MDL *mdl,
                                                         const physis_Part *part)
{
    const uint32_t hash = vertexShader.len + pixelShader.len + physis_shpk_crc(passName.data());
    if (!m_cachedPipelines.contains(hash)) {
        auto vertexShaderModule = m_shaderManager.convertShaderModule(vertexShader, spv::ExecutionModelVertex);
        auto fragmentShaderModule = m_shaderManager.convertShaderModule(pixelShader, spv::ExecutionModelFragment);

        m_device.nameObject(VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64_t>(vertexShaderModule), shaderName);
        m_device.nameObject(VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64_t>(fragmentShaderModule), shaderName);

        VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
        vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageInfo.module = vertexShaderModule;
        vertexShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
        fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageInfo.module = fragmentShaderModule; // m_renderer.loadShaderFromDisk(":/shaders/dummy.frag.spv");
        fragmentShaderStageInfo.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

        std::vector<VkVertexInputBindingDescription> bindings;
        if (passName == "PASS_LIGHTING_OPAQUE" || passName == "PASS_LIGHTING_OPAQUE_VIEWPOSITION") {
            VkVertexInputBindingDescription binding = {};
            binding.stride = sizeof(glm::vec4);
            bindings.push_back(binding);
        } else {
            for (int i = 0; i < part->num_streams; i++) {
                VkVertexInputBindingDescription binding = {};
                binding.stride = part->stream_strides[i];
                binding.binding = i;
                bindings.push_back(binding);
            }
        }

        auto vertex_glsl = m_shaderManager.getShaderModuleTest(vertexShader);
        auto vertex_resources = vertex_glsl.get_shader_resources();

        auto fragment_glsl = m_shaderManager.getShaderModuleTest(pixelShader);
        auto fragment_resources = fragment_glsl.get_shader_resources();

        std::vector<RequestedSet> requestedSets;

        const auto &collectResources = [&requestedSets](const spirv_cross::CompilerGLSL &glsl,
                                                        const spirv_cross::SmallVector<spirv_cross::Resource> &resources,
                                                        const VkShaderStageFlagBits stageFlagBit) {
            for (auto resource : resources) {
                unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

                if (requestedSets.size() <= set) {
                    requestedSets.resize(set + 1);
                }

                auto &requestSet = requestedSets[set];
                requestSet.used = true;

                if (requestSet.bindings.size() <= binding) {
                    requestSet.bindings.resize(binding + 1);
                }

                auto type = glsl.get_type(resource.type_id);

                if (type.basetype == spirv_cross::SPIRType::Image) {
                    requestSet.bindings[binding].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                } else if (type.basetype == spirv_cross::SPIRType::Struct) {
                    requestSet.bindings[binding].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                } else if (type.basetype == spirv_cross::SPIRType::Sampler) {
                    requestSet.bindings[binding].type = VK_DESCRIPTOR_TYPE_SAMPLER;
                }

                requestSet.bindings[binding].used = true;
                requestSet.bindings[binding].stageFlags |= stageFlagBit;

                qInfo() << "Requesting set" << set << "at" << binding;
            }
        };

        collectResources(vertex_glsl, vertex_resources.uniform_buffers, VK_SHADER_STAGE_VERTEX_BIT);
        collectResources(vertex_glsl, vertex_resources.separate_images, VK_SHADER_STAGE_VERTEX_BIT);
        collectResources(vertex_glsl, vertex_resources.separate_samplers, VK_SHADER_STAGE_VERTEX_BIT);

        collectResources(fragment_glsl, fragment_resources.uniform_buffers, VK_SHADER_STAGE_FRAGMENT_BIT);
        collectResources(fragment_glsl, fragment_resources.separate_images, VK_SHADER_STAGE_FRAGMENT_BIT);
        collectResources(fragment_glsl, fragment_resources.separate_samplers, VK_SHADER_STAGE_FRAGMENT_BIT);

        for (auto &set : requestedSets) {
            if (set.used) {
                int j = 0;
                std::vector<VkDescriptorSetLayoutBinding> bindings;
                for (auto &binding : set.bindings) {
                    if (binding.used) {
                        VkDescriptorSetLayoutBinding boneInfoBufferBinding = {};
                        boneInfoBufferBinding.descriptorType = binding.type;
                        boneInfoBufferBinding.descriptorCount = 1;
                        boneInfoBufferBinding.stageFlags = binding.stageFlags;
                        boneInfoBufferBinding.binding = j;

                        bindings.push_back(boneInfoBufferBinding);
                    }
                    j++;
                }

                VkDescriptorSetLayoutCreateInfo layoutInfo = {};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = bindings.size();
                layoutInfo.pBindings = bindings.data();

                vkCreateDescriptorSetLayout(m_device.device, &layoutInfo, nullptr, &set.layout);
            }
        }

        std::vector<VkVertexInputAttributeDescription> attributeDescs;

        for (auto texture : vertex_resources.stage_inputs) {
            unsigned binding = vertex_glsl.get_decoration(texture.id, spv::DecorationLocation);

            VkVertexInputAttributeDescription uv0Attribute = {};
            uv0Attribute.location = binding;

            if (mdl != nullptr) {
                std::string semanticName = m_shaderManager.getShaderModuleResources(vertexShader, binding);

                for (int i = 0; i < mdl->lods[0].num_vertex_elements; i++) {
                    auto element = mdl->lods[0].vertex_elements[i];

                    auto fromVertexType = [](VertexType type) -> VkFormat {
                        switch (type) {
                        case VertexType::Single1:
                            return VK_FORMAT_R32_SFLOAT;
                        case VertexType::Single2:
                            return VK_FORMAT_R32G32_SFLOAT;
                        case VertexType::Single3:
                            return VK_FORMAT_R32G32B32_SFLOAT;
                        case VertexType::Single4:
                            return VK_FORMAT_R32G32B32A32_SFLOAT;
                        case VertexType::Byte4:
                            return VK_FORMAT_R8G8B8A8_SINT;
                        case VertexType::Short2:
                            break;
                        case VertexType::Short4:
                            break;
                        case VertexType::ByteFloat4:
                            return VK_FORMAT_R8G8B8A8_UNORM;
                        case VertexType::Short2n:
                            break;
                        case VertexType::Short4n:
                            break;
                        case VertexType::Half2:
                            return VK_FORMAT_R16G16_SFLOAT;
                        case VertexType::Half4:
                            return VK_FORMAT_R16G16B16A16_UNORM;
                        case VertexType::UnsignedShort2:
                            break;
                        case VertexType::UnsignedShort4:
                            break;
                        }

                        qInfo() << "Unhandled type" << magic_enum::enum_name(type);
                        return VK_FORMAT_R32G32B32A32_SFLOAT;
                    };

                    if (semanticName == "POSITION" && mdl->lods[0].vertex_elements[i].vertex_usage == VertexUsage::Position) {
                        uv0Attribute.binding = element.stream;
                        uv0Attribute.offset = mdl->lods[0].vertex_elements[i].offset;
                        uv0Attribute.format = fromVertexType(element.vertex_type);
                    } else if (semanticName == "COLOR" && mdl->lods[0].vertex_elements[i].vertex_usage == VertexUsage::Color) {
                        uv0Attribute.binding = element.stream;
                        uv0Attribute.offset = mdl->lods[0].vertex_elements[i].offset;
                        uv0Attribute.format = fromVertexType(element.vertex_type);
                    } else if (semanticName == "TEXCOORD" && mdl->lods[0].vertex_elements[i].vertex_usage == VertexUsage::UV) {
                        uv0Attribute.binding = element.stream;
                        uv0Attribute.offset = mdl->lods[0].vertex_elements[i].offset;
                        uv0Attribute.format = fromVertexType(element.vertex_type);
                    } else if (semanticName == "NORMAL" && mdl->lods[0].vertex_elements[i].vertex_usage == VertexUsage::Normal) {
                        uv0Attribute.binding = element.stream;
                        uv0Attribute.offset = mdl->lods[0].vertex_elements[i].offset;
                        uv0Attribute.format = fromVertexType(element.vertex_type);
                    } else if (semanticName == "BINORMAL"
                               && mdl->lods[0].vertex_elements[i].vertex_usage == VertexUsage::BiTangent) { // TODO: Bitangent is here twice lol
                        uv0Attribute.binding = element.stream;
                        uv0Attribute.offset = mdl->lods[0].vertex_elements[i].offset;
                        uv0Attribute.format = fromVertexType(element.vertex_type);
                    } else if (semanticName == "BLENDWEIGHT" && mdl->lods[0].vertex_elements[i].vertex_usage == VertexUsage::BlendWeights) {
                        uv0Attribute.binding = element.stream;
                        uv0Attribute.offset = mdl->lods[0].vertex_elements[i].offset;
                        uv0Attribute.format = fromVertexType(element.vertex_type);
                    } else if (semanticName == "BLENDINDICES" && mdl->lods[0].vertex_elements[i].vertex_usage == VertexUsage::BlendIndices) {
                        uv0Attribute.binding = element.stream;
                        uv0Attribute.offset = mdl->lods[0].vertex_elements[i].offset;
                        uv0Attribute.format = fromVertexType(element.vertex_type);
                    }
                }

                if (uv0Attribute.format == VK_FORMAT_UNDEFINED) {
                    qWarning() << "Unhandled input name:" << semanticName;
                }
            } else {
                auto type = vertex_glsl.get_type(texture.type_id);
                if (type.basetype == spirv_cross::SPIRType::Int) {
                    switch (type.vecsize) {
                    case 1:
                        uv0Attribute.format = VK_FORMAT_R32_SINT;
                        break;
                    case 2:
                        uv0Attribute.format = VK_FORMAT_R32G32_SINT;
                        break;
                    case 3:
                        uv0Attribute.format = VK_FORMAT_R32G32B32_SINT;
                        break;
                    case 4:
                        uv0Attribute.format = VK_FORMAT_R8G8B8A8_UINT; // supposed to be VK_FORMAT_R32G32B32A32_SINT, but our bone_id is uint8_t currently
                        break;
                    }
                } else if (type.basetype == spirv_cross::SPIRType::Float) {
                    switch (type.vecsize) {
                    case 1:
                        uv0Attribute.format = VK_FORMAT_R32_SFLOAT;
                        break;
                    case 2:
                        uv0Attribute.format = VK_FORMAT_R32G32_SFLOAT;
                        break;
                    case 3:
                        uv0Attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
                        break;
                    case 4:
                        uv0Attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                        break;
                    }
                } else if (type.basetype == spirv_cross::SPIRType::Half) {
                    switch (type.vecsize) {
                    case 1:
                        uv0Attribute.format = VK_FORMAT_R16_SFLOAT;
                        break;
                    case 2:
                        uv0Attribute.format = VK_FORMAT_R16G16_SFLOAT;
                        break;
                    case 3:
                        uv0Attribute.format = VK_FORMAT_R16G16B16_SFLOAT;
                        break;
                    case 4:
                        uv0Attribute.format = VK_FORMAT_R16G16B16A16_SFLOAT;
                        break;
                    }
                }
            }

            attributeDescs.push_back(uv0Attribute);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState = {};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.vertexBindingDescriptionCount = bindings.size();
        vertexInputState.pVertexBindingDescriptions = bindings.data();
        vertexInputState.vertexAttributeDescriptionCount = attributeDescs.size();
        vertexInputState.pVertexAttributeDescriptions = attributeDescs.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE; // TODO: implement cull mode
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

        int colorAttachmentCount = 1;
        // TODO: hardcoded, should be a reusable function to get the color attachments
        if (passName == "PASS_LIGHTING_OPAQUE") {
            colorAttachmentCount = 2;
        } else if (passName == "PASS_G_OPAQUE") {
            colorAttachmentCount = 5;
        }

        for (int i = 0; i < colorAttachmentCount; i++) {
            colorBlendAttachments.push_back(colorBlendAttachment);
        }

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.attachmentCount = colorBlendAttachments.size();
        colorBlending.pAttachments = colorBlendAttachments.data();

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = dynamicStates.size();
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        // pipelineLayoutInfo.pushConstantRangeCount = 1;
        // pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        std::vector<VkDescriptorSetLayout> setLayouts;
        for (auto &set : requestedSets) {
            if (set.used) {
                setLayouts.push_back(set.layout);
            }
        }

        pipelineLayoutInfo.setLayoutCount = setLayouts.size();
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        vkCreatePipelineLayout(m_device.device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        if (passName != "PASS_LIGHTING_OPAQUE_VIEWPOSITION") {
            depthStencil.depthTestEnable = VK_TRUE;
        }
        if (passName == "PASS_G_OPAQUE") {
            depthStencil.depthWriteEnable = VK_TRUE;
        }
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.maxDepthBounds = 1.0f;

        const std::array colorAttachmentFormats = {VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_FORMAT_R8G8B8A8_UNORM};

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = colorAttachmentCount;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
        pipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT; // TODO: hardcoded

        VkGraphicsPipelineCreateInfo createInfo = {};
        createInfo.pNext = &pipelineRenderingCreateInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        createInfo.stageCount = shaderStages.size();
        createInfo.pStages = shaderStages.data();
        createInfo.pVertexInputState = &vertexInputState;
        createInfo.pInputAssemblyState = &inputAssembly;
        createInfo.pViewportState = &viewportState;
        createInfo.pRasterizationState = &rasterizer;
        createInfo.pMultisampleState = &multisampling;
        createInfo.pColorBlendState = &colorBlending;
        createInfo.pDynamicState = &dynamicState;
        createInfo.pDepthStencilState = &depthStencil;
        createInfo.layout = pipelineLayout;
        // createInfo.renderPass = m_renderer.renderPass;

        VkPipeline pipeline = VK_NULL_HANDLE;
        vkCreateGraphicsPipelines(m_device.device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);

        qInfo() << "Created" << pipeline << "for hash" << hash;
        m_cachedPipelines[hash] = CachedPipeline{.pipeline = pipeline,
                                                 .pipelineLayout = pipelineLayout,
                                                 .setLayouts = setLayouts,
                                                 .cachedDescriptors = {},
                                                 .requestedSets = requestedSets,
                                                 .vertexShader = vertexShader,
                                                 .pixelShader = pixelShader};
    }

    auto &pipeline = m_cachedPipelines[hash];
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

    VkViewport viewport = {};
    viewport.width = m_device.swapChain->extent.width;
    viewport.height = m_device.swapChain->extent.height;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent = m_device.swapChain->extent;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    return pipeline;
}

VkDescriptorSet
GameRenderer::createDescriptorFor(const DrawObject *object, const CachedPipeline &pipeline, int i, const RenderMaterial *material, std::string_view pass)
{
    VkDescriptorSet set;

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = m_device.descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &pipeline.setLayouts[i];

    vkAllocateDescriptorSets(m_device.device, &allocateInfo, &set);
    if (set == VK_NULL_HANDLE) {
        // qFatal("Failed to create descriptor set!");
        return VK_NULL_HANDLE;
    }

    // TODO: way too eager
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkDescriptorBufferInfo> bufferInfo;
    std::vector<VkDescriptorImageInfo> imageInfo;

    writes.reserve(pipeline.requestedSets[i].bindings.size());
    bufferInfo.reserve(pipeline.requestedSets[i].bindings.size());
    imageInfo.reserve(pipeline.requestedSets[i].bindings.size());

    int j = 0;
    int z = 0;
    int p = 0;
    VkShaderStageFlags currentStageFlags;
    for (auto binding : pipeline.requestedSets[i].bindings) {
        if (binding.used) {
            // a giant hack
            if (currentStageFlags != binding.stageFlags) {
                z = 0;
                p = 0;
                currentStageFlags = binding.stageFlags;
            }

            VkWriteDescriptorSet &descriptorWrite = writes.emplace_back();
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.descriptorType = binding.type;
            descriptorWrite.dstSet = set;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.dstBinding = j;

            switch (binding.type) {
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
                auto info = &imageInfo.emplace_back();
                descriptorWrite.pImageInfo = info;

                if (binding.stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT && p < pipeline.pixelShader.num_resource_parameters) {
                    auto name = pipeline.pixelShader.resource_parameters[p].name;
                    qInfo() << "Requesting image" << name << "at" << j;
                    if (strcmp(name, "g_SamplerGBuffer") == 0) {
                        info->imageView = m_normalGBuffer.imageView;
                    } else if (strcmp(name, "g_SamplerViewPosition") == 0) {
                        info->imageView = m_viewPositionBuffer.imageView;
                    } else if (strcmp(name, "g_SamplerDepth") == 0) {
                        info->imageView = m_depthBuffer.imageView;
                    } else if (strcmp(name, "g_SamplerNormal") == 0 && material->normalTexture.has_value()) {
                        Q_ASSERT(material);
                        info->imageView = material->normalTexture->imageView;
                    } else if (strcmp(name, "g_SamplerIndex") == 0 && material->indexTexture.has_value()) {
                        Q_ASSERT(material);
                        info->imageView = material->indexTexture->imageView;
                    } else if (strcmp(name, "g_SamplerLightDiffuse") == 0) {
                        Q_ASSERT(material);
                        info->imageView = m_lightBuffer.imageView;
                    } else if (strcmp(name, "g_SamplerLightSpecular") == 0) {
                        Q_ASSERT(material);
                        info->imageView = m_lightSpecularBuffer.imageView;
                    } else if ((strcmp(name, "g_SamplerDiffuse") == 0 || strcmp(name, "g_SamplerDecal") == 0) && material->diffuseTexture.has_value()) {
                        // NOTE: the g_SamplerDecal is 100% a guess, i'm almost certain it's another texture
                        Q_ASSERT(material);
                        info->imageView = material->diffuseTexture->imageView;
                    } else if (strcmp(name, "g_SamplerSpecular") == 0 && material->specularTexture.has_value()) {
                        Q_ASSERT(material);
                        info->imageView = material->specularTexture->imageView;
                    } else if (strcmp(name, "g_SamplerMask") == 0 && material->multiTexture.has_value()) {
                        Q_ASSERT(material);
                        info->imageView = material->multiTexture->imageView;
                    } else if (strcmp(name, "g_SamplerTileNormal") == 0) {
                        info->imageView = m_tileNormal.imageView;
                    } else if (strcmp(name, "g_SamplerTileDiffuse") == 0) {
                        info->imageView = m_tileDiffuse.imageView;
                    } else if (strcmp(name, "g_SamplerTable") == 0) {
                        Q_ASSERT(material);
                        if (!material->tableTexture.has_value()) {
                            qWarning() << "Attempted to use table texture for a non-dyeable material. Something has went wrong!";
                            info->imageView = m_dummyTex.imageView;
                        } else {
                            info->imageView = material->tableTexture->imageView;
                        }
                    } else {
                        info->imageView = m_dummyTex.imageView;
                        qInfo() << "Unknown image" << name;
                    }

                    p++;
                } else {
                    info->imageView = m_dummyTex.imageView;
                }

                info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            } break;
            case VK_DESCRIPTOR_TYPE_SAMPLER: {
                auto info = &imageInfo.emplace_back();
                descriptorWrite.pImageInfo = info;

                info->sampler = m_normalSampler;
            } break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
                auto info = &bufferInfo.emplace_back();
                descriptorWrite.pBufferInfo = info;

                auto useUniformBuffer = [&info](const Buffer &buffer) {
                    info->buffer = buffer.buffer;
                    info->range = buffer.size;
                };

                auto bindBuffer = [this, &useUniformBuffer, &info, j, &object, pass, material](const char *name) {
                    qInfo() << "Requesting" << name << "at" << j;

                    if (strcmp(name, "g_CameraParameter") == 0) {
                        useUniformBuffer(g_CameraParameter);
                    } else if (strcmp(name, "g_JointMatrixArray") == 0 || strcmp(name, "g_JointMatrixArrayPrev") == 0) {
                        Q_ASSERT(object != nullptr);
                        useUniformBuffer(object->boneInfoBuffer);
                    } else if (strcmp(name, "g_InstanceParameter") == 0) {
                        useUniformBuffer(g_InstanceParameter);
                    } else if (strcmp(name, "g_ModelParameter") == 0) {
                        useUniformBuffer(g_ModelParameter);
                    } else if (strcmp(name, "g_MaterialParameter") == 0) {
                        if (pass == "PASS_COMPOSITE_SEMITRANSPARENCY") {
                            // The composite semi-transparency uses a different alphathreshold
                            useUniformBuffer(g_TransparencyMaterialParameter);
                        } else {
                            Q_ASSERT(material);
                            Q_ASSERT(material->materialBuffer.buffer);
                            useUniformBuffer(material->materialBuffer);
                        }
                    } else if (strcmp(name, "g_LightParam") == 0) {
                        useUniformBuffer(g_LightParam);
                    } else if (strcmp(name, "g_CommonParameter") == 0) {
                        useUniformBuffer(g_CommonParameter);
                    } else if (strcmp(name, "g_CustomizeParameter") == 0) {
                        useUniformBuffer(g_CustomizeParameter);
                    } else if (strcmp(name, "g_SceneParameter") == 0) {
                        useUniformBuffer(g_SceneParameter);
                    } else if (strcmp(name, "g_MaterialParameterDynamic") == 0) {
                        useUniformBuffer(g_MaterialParameterDynamic);
                    } else if (strcmp(name, "g_DecalColor") == 0) {
                        useUniformBuffer(g_DecalColor);
                    } else if (strcmp(name, "g_AmbientParam") == 0) {
                        useUniformBuffer(g_AmbientParam);
                    } else if (strcmp(name, "g_ShaderTypeParameter") == 0) {
                        useUniformBuffer(g_ShaderTypeParameter);
                    } else if (strcmp(name, "g_PbrParameterCommon") == 0) {
                        useUniformBuffer(g_PbrParameterCommon);
                    } else if (strcmp(name, "g_WorldViewMatrix") == 0) {
                        useUniformBuffer(g_WorldViewMatrix);
                    } else {
                        qInfo() << "Unknown resource:" << name;
                        info->buffer = m_dummyBuffer.buffer;
                        info->range = 655360;
                    }
                };

                if (binding.stageFlags == VK_SHADER_STAGE_VERTEX_BIT) {
                    auto name = pipeline.vertexShader.scalar_parameters[z].name;

                    bindBuffer(name);
                    z++;
                } else if (binding.stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT) {
                    auto name = pipeline.pixelShader.scalar_parameters[z].name;

                    bindBuffer(name);
                    z++;
                } else {
                    // placeholder buffer so it at least doesn't crash
                    info->buffer = m_dummyBuffer.buffer;
                    info->range = 655360;
                }
            } break;
            default:
                break;
            }
        }
        j++;
    }

    vkUpdateDescriptorSets(m_device.device, writes.size(), writes.data(), 0, nullptr);

    return set;
}

void GameRenderer::createImageResources()
{
    m_normalGBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                             m_device.swapChain->extent.height,
                                             VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_normalGBuffer, "Normal GBuffer");

    m_viewPositionBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                                  m_device.swapChain->extent.height,
                                                  VK_FORMAT_R16G16B16A16_SFLOAT,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_viewPositionBuffer, "View Position");

    m_lightBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                           m_device.swapChain->extent.height,
                                           VK_FORMAT_R8G8B8A8_UNORM,
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_lightBuffer, "Light Diffuse");

    m_lightSpecularBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                                   m_device.swapChain->extent.height,
                                                   VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_lightSpecularBuffer, "Light Specular");

    m_compositeBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                               m_device.swapChain->extent.height,
                                               VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_compositeBuffer, "Composite");

    m_ZBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                       m_device.swapChain->extent.height,
                                       VK_FORMAT_R8G8B8A8_UNORM,
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_ZBuffer, "ZBuffer");

    m_depthBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                           m_device.swapChain->extent.height,
                                           VK_FORMAT_D32_SFLOAT,
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_depthBuffer, "Depth");

    m_motionGBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                             m_device.swapChain->extent.height,
                                             VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_motionGBuffer, "Motion GBuffer");

    m_diffuseGBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                              m_device.swapChain->extent.height,
                                              VK_FORMAT_R8G8B8A8_UNORM,
                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_diffuseGBuffer, "Diffuse GBuffer");

    m_motionGBuffer2 = m_device.createTexture(m_device.swapChain->extent.width,
                                              m_device.swapChain->extent.height,
                                              VK_FORMAT_R8G8B8A8_UNORM,
                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_motionGBuffer2, "Motion 2 GBuffer");

    m_unkGBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                          m_device.swapChain->extent.height,
                                          VK_FORMAT_R8G8B8A8_UNORM,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    m_device.nameTexture(m_unkGBuffer, "Unk GBuffer");

    CommonParameter commonParam{};
    commonParam.m_RenderTarget = {1.0f / m_device.swapChain->extent.width,
                                  1.0f / m_device.swapChain->extent.height,
                                  0.0f,
                                  0.0f}; // used to convert screen-space coordinates back into 0.0-1.0
    commonParam.m_Misc = glm::vec4(1.0f);
    commonParam.m_Misc2 = glm::vec4(1.0f);

    m_device.copyToBuffer(g_CommonParameter, &commonParam, sizeof(CommonParameter));
}

Texture &GameRenderer::getCompositeTexture()
{
    return m_compositeBuffer;
}

void GameRenderer::bindDescriptorSets(VkCommandBuffer commandBuffer,
                                      GameRenderer::CachedPipeline &pipeline,
                                      const DrawObject *object,
                                      const RenderMaterial *material,
                                      std::string_view pass)
{
    for (size_t i = 0; i < pipeline.setLayouts.size(); i++) {
        if (!pipeline.cachedDescriptors.count(i)) {
            if (auto descriptor = createDescriptorFor(object, pipeline, i, material, pass); descriptor != VK_NULL_HANDLE) {
                pipeline.cachedDescriptors[i] = descriptor;
            } else {
                continue;
            }
        }

        // TODO: we can pass all descriptors in one function call
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, i, 1, &pipeline.cachedDescriptors[i], 0, nullptr);
    }
}

Device &GameRenderer::device()
{
    return m_device;
}
