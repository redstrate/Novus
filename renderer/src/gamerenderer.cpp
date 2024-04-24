// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gamerenderer.h"

#include <array>

#include <QDebug>

#include <glm/ext/matrix_clip_space.hpp>
#include <physis.hpp>
#include <spirv_glsl.hpp>

#include "camera.h"
#include "dxbc_module.h"
#include "dxbc_reader.h"
#include "rendermanager.h"
#include "swapchain.h"

// TODO: maybe need UV?
// note: SQEX passes the vertice positions as UV coordinates (yes, -1 to 1.) the shaders then transform them back with the g_CommonParameter.m_RenderTarget vec4
const std::vector<glm::vec4> planeVertices = {
    {-1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},

    {-1.0f, 1.0f, 0.0f, 1.0f},
    {-1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
};

dxvk::Logger dxvk::Logger::s_instance("dxbc.log");

const std::array<std::string, 14> passes = {
    // Shadows?
    "PASS_0",
    // Z "prepass"
    "PASS_Z_OPAQUE",
    // computes and stores normals (TODO: denote how these normals are special)
    "PASS_G_OPAQUE",
    // g run for each light
    // takes view pos, then unknown texture and normal
    "PASS_LIGHTING_OPAQUE",
    "PASS_G_SEMITRANSPARENCY",
    "PASS_COMPOSITE_OPAQUE",
    "PASS_7",
    "PASS_WATER",
    "PASS_WATER_Z",
    "PASS_SEMITRANSPARENCY",
    "PASS_COMPOSITE_SEMITRANSPARENCY",
    "PASS_10",
    "PASS_12",
    "PASS_14"};

const int INVALID_PASS = 255;

GameRenderer::GameRenderer(Device &device, GameData *data)
    : m_device(device)
    , m_data(data)
{
    m_dummyTex = m_device.createDummyTexture();
    m_dummyBuffer = m_device.createDummyBuffer();

    size_t vertexSize = planeVertices.size() * sizeof(glm::vec4);
    m_planeVertexBuffer = m_device.createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_device.copyToBuffer(m_planeVertexBuffer, (void *)planeVertices.data(), vertexSize);

    directionalLightningShpk = physis_parse_shpk(physis_gamedata_extract_file(m_data, "shader/sm5/shpk/directionallighting.shpk"));
    createViewPositionShpk = physis_parse_shpk(physis_gamedata_extract_file(m_data, "shader/sm5/shpk/createviewposition.shpk"));

    // camera data
    {
        g_CameraParameter = m_device.createBuffer(sizeof(CameraParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }

    // instance data
    {
        g_InstanceParameter = m_device.createBuffer(sizeof(InstanceParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        InstanceParameter instanceParameter{};
        m_device.copyToBuffer(g_InstanceParameter, &instanceParameter, sizeof(InstanceParameter));
    }

    // model data
    {
        g_ModelParameter = m_device.createBuffer(sizeof(ModelParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        ModelParameter modelParameter{};
        m_device.copyToBuffer(g_ModelParameter, &modelParameter, sizeof(ModelParameter));
    }

    // material data
    {
        g_MaterialParameter = m_device.createBuffer(sizeof(MaterialParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        MaterialParameters materialParameter{};
        materialParameter.parameters[0] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        materialParameter.parameters[5].z = 1.0f;
        m_device.copyToBuffer(g_MaterialParameter, &materialParameter, sizeof(MaterialParameters));
    }

    // light data
    {
        g_LightParam = m_device.createBuffer(sizeof(LightParam), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

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
    }

    // scene data
    {
        g_SceneParameter = m_device.createBuffer(sizeof(SceneParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        SceneParameter sceneParameter{};

        m_device.copyToBuffer(g_SceneParameter, &sceneParameter, sizeof(SceneParameter));
    }

    // customize data
    {
        g_CustomizeParameter = m_device.createBuffer(sizeof(CustomizeParameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        CustomizeParameter customizeParameter{};

        m_device.copyToBuffer(g_CustomizeParameter, &customizeParameter, sizeof(CustomizeParameter));
    }

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.maxLod = 1.0f;

    vkCreateSampler(m_device.device, &samplerInfo, nullptr, &m_sampler);

    createImageResources();
}

void GameRenderer::render(VkCommandBuffer commandBuffer, uint32_t imageIndex, Camera &camera, const std::vector<DrawObject> &models)
{
    // TODO: this shouldn't be here
    CameraParameter cameraParameter{};

    const glm::mat4 viewProjectionMatrix = camera.perspective * camera.view;

    cameraParameter.m_ViewMatrix = glm::transpose(camera.view);
    cameraParameter.m_InverseViewMatrix = glm::transpose(glm::inverse(camera.view));
    cameraParameter.m_ViewProjectionMatrix = glm::transpose(viewProjectionMatrix);
    cameraParameter.m_InverseViewProjectionMatrix = glm::transpose(glm::inverse(viewProjectionMatrix));

    // known params
    cameraParameter.m_InverseProjectionMatrix = glm::transpose(glm::inverse(viewProjectionMatrix));
    cameraParameter.m_ProjectionMatrix = glm::transpose(viewProjectionMatrix);

    cameraParameter.m_MainViewToProjectionMatrix = glm::transpose(glm::inverse(camera.perspective));
    cameraParameter.m_EyePosition = glm::vec3(5.0f); // placeholder
    cameraParameter.m_LookAtVector = glm::vec3(0.0f); // placeholder

    m_device.copyToBuffer(g_CameraParameter, &cameraParameter, sizeof(CameraParameter));

    int i = 0;
    for (const auto pass : passes) {
        // hardcoded to the known pass for now
        if (pass == "PASS_G_OPAQUE" || pass == "PASS_Z_OPAQUE") {
            beginPass(imageIndex, commandBuffer, pass);

            for (auto &model : models) {
                // copy bone data
                {
                    const size_t bufferSize = sizeof(glm::mat3x4) * 64;
                    void *mapped_data = nullptr;
                    vkMapMemory(m_device.device, model.boneInfoBuffer.memory, 0, bufferSize, 0, &mapped_data);

                    std::vector<glm::mat3x4> newBoneData(model.boneData.size());
                    for (int i = 0; i < 64; i++) {
                        newBoneData[i] = glm::transpose(model.boneData[i]);
                    }

                    memcpy(mapped_data, newBoneData.data(), bufferSize);

                    VkMappedMemoryRange range = {};
                    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                    range.memory = model.boneInfoBuffer.memory;
                    range.size = bufferSize;
                    vkFlushMappedMemoryRanges(m_device.device, 1, &range);

                    vkUnmapMemory(m_device.device, model.boneInfoBuffer.memory);
                }

                for (const auto &part : model.parts) {
                    auto &renderMaterial = model.materials[part.materialIndex];

                    if (renderMaterial.shaderPackage.p_ptr == nullptr) {
                        qWarning() << "Invalid shader package!";
                    }

                    std::vector<uint32_t> systemKeys;
                    if (renderMaterial.type == MaterialType::Skin) {
                        systemKeys.push_back(physis_shpk_crc("DecodeDepthBuffer_RAWZ"));
                    }
                    std::vector<uint32_t> sceneKeys = {
                        physis_shpk_crc("TransformViewSkin"),
                        physis_shpk_crc("GetAmbientLight_SH"),
                        physis_shpk_crc("GetReflectColor_Texture"),
                        physis_shpk_crc("GetAmbientOcclusion_None"),
                        physis_shpk_crc("ApplyDitherClipOff"),
                    };
                    std::vector<uint32_t> materialKeys;
                    for (int j = 0; j < renderMaterial.shaderPackage.num_material_keys; j++) {
                        auto value = renderMaterial.shaderPackage.material_keys[j].default_value;
                        materialKeys.push_back(renderMaterial.shaderPackage.material_keys[j].default_value);
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

                        auto &pipeline = bindPipeline(commandBuffer, pass, vertexShader, pixelShader);
                        bindDescriptorSets(commandBuffer, pipeline, &model, &renderMaterial);

                        VkDeviceSize offsets[] = {0};
                        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &part.vertexBuffer.buffer, offsets);
                        vkCmdBindIndexBuffer(commandBuffer, part.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

                        vkCmdDrawIndexed(commandBuffer, part.numIndices, 1, 0, 0, 0);
                    }
                }
            }

            endPass(commandBuffer, pass);
        } else if (pass == "PASS_LIGHTING_OPAQUE") {
            // first we need to generate the view positions with createviewpositions
            beginPass(imageIndex, commandBuffer, "PASS_LIGHTING_OPAQUE_VIEWPOSITION");
            {
                std::vector<uint32_t> systemKeys = {
                    physis_shpk_crc("DecodeDepthBuffer_RAWZ"),
                };
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

                    auto &pipeline = bindPipeline(commandBuffer, "PASS_LIGHTING_OPAQUE_VIEWPOSITION", vertexShader, pixelShader);
                    bindDescriptorSets(commandBuffer, pipeline, nullptr, nullptr);

                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_planeVertexBuffer.buffer, offsets);

                    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
                }
            }
            endPass(commandBuffer, pass);

            beginPass(imageIndex, commandBuffer, pass);
            // then run the directionallighting shader
            {
                std::vector<uint32_t> systemKeys = {
                    physis_shpk_crc("DecodeDepthBuffer_RAWZ"),
                };
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

                    auto &pipeline = bindPipeline(commandBuffer, pass, vertexShader, pixelShader);
                    bindDescriptorSets(commandBuffer, pipeline, nullptr, nullptr);

                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_planeVertexBuffer.buffer, offsets);

                    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
                }
            }
            endPass(commandBuffer, pass);
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

void GameRenderer::beginPass(uint32_t imageIndex, VkCommandBuffer commandBuffer, const std::string_view passName)
{
    VkRenderingInfo renderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO};
    renderingInfo.renderArea.extent = m_device.swapChain->extent;

    std::vector<VkRenderingAttachmentInfo> colorAttachments;
    VkRenderingAttachmentInfo depthStencilAttachment{};

    if (passName == "PASS_G_OPAQUE") {
        // normals, it seems like
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
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

        // unknown, seems to be background?
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            attachmentInfo.imageView = VK_NULL_HANDLE;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            colorAttachments.push_back(attachmentInfo);
        }

        // unknown, seems to be background?
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            attachmentInfo.imageView = VK_NULL_HANDLE;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            colorAttachments.push_back(attachmentInfo);
        }

        // depth
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            attachmentInfo.imageView = m_depthBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentInfo.clearValue.depthStencil.depth = 1.0f;

            depthStencilAttachment = attachmentInfo;
        }
    } else if (passName == "PASS_LIGHTING_OPAQUE") {
        // normals, it seems like
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
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

        // unknown
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            attachmentInfo.imageView = VK_NULL_HANDLE;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            colorAttachments.push_back(attachmentInfo);
        }
    } else if (passName == "PASS_LIGHTING_OPAQUE_VIEWPOSITION") {
        // TODO: Hack we should not be using a special pass for this, we should just design our API better
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            attachmentInfo.imageView = m_viewPositionBuffer.imageView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachmentInfo.clearValue.color.float32[0] = 0.24;
            attachmentInfo.clearValue.color.float32[1] = 0.24;
            attachmentInfo.clearValue.color.float32[2] = 0.24;
            attachmentInfo.clearValue.color.float32[3] = 1.0;

            colorAttachments.push_back(attachmentInfo);
        }
    } else if (passName == "PASS_Z_OPAQUE") {
        // normals, it seems like
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
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

        // unknown
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            attachmentInfo.imageView = VK_NULL_HANDLE;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            colorAttachments.push_back(attachmentInfo);
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

void GameRenderer::endPass(VkCommandBuffer commandBuffer, std::string_view passName)
{
    vkCmdEndRendering(commandBuffer);
}

GameRenderer::CachedPipeline &
GameRenderer::bindPipeline(VkCommandBuffer commandBuffer, std::string_view passName, physis_Shader &vertexShader, physis_Shader &pixelShader)
{
    const uint32_t hash = vertexShader.len + pixelShader.len + physis_shpk_crc(passName.data());
    if (!m_cachedPipelines.contains(hash)) {
        auto vertexShaderModule = convertShaderModule(vertexShader, spv::ExecutionModelVertex);
        auto fragmentShaderModule = convertShaderModule(pixelShader, spv::ExecutionModelFragment);

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

        VkVertexInputBindingDescription binding = {};

        // TODO: temporary
        if (passName == "PASS_G_OPAQUE" || passName == "PASS_Z_OPAQUE") {
            binding.stride = sizeof(Vertex);
        } else if (passName == "PASS_LIGHTING_OPAQUE" || passName == "PASS_LIGHTING_OPAQUE_VIEWPOSITION") {
            binding.stride = sizeof(glm::vec4);
        }

        auto vertex_glsl = getShaderModuleResources(vertexShader);
        auto vertex_resources = vertex_glsl.get_shader_resources();

        auto fragment_glsl = getShaderModuleResources(pixelShader);
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

            auto name = vertex_glsl.get_name(texture.id);

            VkVertexInputAttributeDescription uv0Attribute = {};

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
            } else {
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
            }

            uv0Attribute.location = binding;

            // TODO: temporary
            if (name == "v0") {
                uv0Attribute.offset = offsetof(Vertex, position);
            } else if (name == "v1") {
                uv0Attribute.offset = offsetof(Vertex, color);
            } else if (name == "v2") {
                uv0Attribute.offset = offsetof(Vertex, normal);
            } else if (name == "v3") {
                uv0Attribute.offset = offsetof(Vertex, uv0);
            } else if (name == "v4") {
                uv0Attribute.offset = offsetof(Vertex, bitangent); // FIXME: should be tangent
            } else if (name == "v5") {
                uv0Attribute.offset = offsetof(Vertex, bitangent);
            } else if (name == "v6") {
                uv0Attribute.offset = offsetof(Vertex, bone_weight);
            } else if (name == "v7") {
                uv0Attribute.offset = offsetof(Vertex, bone_id);
            }

            attributeDescs.push_back(uv0Attribute);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState = {};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.vertexBindingDescriptionCount = 1;
        vertexInputState.pVertexBindingDescriptions = &binding;
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
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

        int colorAttachmentCount = 1;
        // TODO: hardcoded, should be a reusable function to get the color attachments
        if (passName == "PASS_G_OPAQUE") {
            colorAttachmentCount = 3;
        } else if (passName == "PASS_LIGHTING_OPAQUE") {
            colorAttachmentCount = 2;
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
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.maxDepthBounds = 1.0f;

        std::array<VkFormat, 3> colorAttachmentFormats = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 3; // TODO: hardcoded
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

VkShaderModule GameRenderer::convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel)
{
    dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader.bytecode), shader.len);

    dxvk::DxbcModule module(reader);

    dxvk::DxbcModuleInfo info;
    auto result = module.compile(info, "test");

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = result.code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(result.code.data());

    VkShaderModule shaderModule;
    vkCreateShaderModule(m_device.device, &createInfo, nullptr, &shaderModule);

    // TODO: for debug only
    spirv_cross::CompilerGLSL glsl(result.code.data(), result.code.dwords());

    auto resources = glsl.get_shader_resources();

    int i = 0;
    for (auto texture : resources.stage_inputs) {
        // glsl.set_name(texture.id, shader.)
        // qInfo() << shader.resource_parameters[i].name << texture.id;
        // qInfo() << "stage input" << i << texture.name << glsl.get_type(texture.type_id).width;
        i++;
        // glsl.set_name(remap.combined_id, "SPIRV_Cross_Combined");
    }

    // Here you can also set up decorations if you want (binding = #N).
    i = 0;
    for (auto texture : resources.separate_images) {
        glsl.set_name(texture.id, shader.resource_parameters[i].name);
        i++;
    }

    i = 0;
    for (auto buffer : resources.uniform_buffers) {
        glsl.set_name(buffer.id, shader.scalar_parameters[i].name);
        i++;
    }

    spirv_cross::CompilerGLSL::Options options;
    options.vulkan_semantics = true;
    options.enable_420pack_extension = false;
    glsl.set_common_options(options);
    glsl.set_entry_point("main", executionModel);

    qInfo() << "Compiled GLSL:" << glsl.compile().c_str();

    return shaderModule;
}

spirv_cross::CompilerGLSL GameRenderer::getShaderModuleResources(const physis_Shader &shader)
{
    dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader.bytecode), shader.len);

    dxvk::DxbcModule module(reader);

    dxvk::DxbcModuleInfo info;
    auto result = module.compile(info, "test");

    // glsl.build_combined_image_samplers();

    return spirv_cross::CompilerGLSL(result.code.data(), result.code.dwords());
}

VkDescriptorSet GameRenderer::createDescriptorFor(const DrawObject *object, const CachedPipeline &pipeline, int i, const RenderMaterial *material)
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

                if (binding.stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT && p < 4) {
                    auto name = pipeline.pixelShader.resource_parameters[p].name;
                    qInfo() << "Requesting image" << name << "at" << j;
                    if (strcmp(name, "g_SamplerGBuffer") == 0) {
                        info->imageView = m_normalGBuffer.imageView;
                    } else if (strcmp(name, "g_SamplerViewPosition") == 0) {
                        info->imageView = m_viewPositionBuffer.imageView;
                    } else if (strcmp(name, "g_SamplerDepth") == 0) {
                        info->imageView = m_depthBuffer.imageView;
                    } else if (strcmp(name, "g_SamplerNormal") == 0) {
                        Q_ASSERT(material);
                        info->imageView = material->normalTexture->view;
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

                info->sampler = m_sampler;
            } break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
                auto info = &bufferInfo.emplace_back();
                descriptorWrite.pBufferInfo = info;

                auto useUniformBuffer = [&info](const Buffer &buffer) {
                    info->buffer = buffer.buffer;
                    info->range = buffer.size;
                };

                auto bindBuffer = [this, &useUniformBuffer, &info, j, &object](const char *name) {
                    qInfo() << "Requesting" << name << "at" << j;

                    if (strcmp(name, "g_CameraParameter") == 0) {
                        useUniformBuffer(g_CameraParameter);
                    } else if (strcmp(name, "g_JointMatrixArray") == 0) {
                        Q_ASSERT(object != nullptr);
                        useUniformBuffer(object->boneInfoBuffer);
                    } else if (strcmp(name, "g_InstanceParameter") == 0) {
                        useUniformBuffer(g_InstanceParameter);
                    } else if (strcmp(name, "g_ModelParameter") == 0) {
                        useUniformBuffer(g_ModelParameter);
                    } else if (strcmp(name, "g_MaterialParameter") == 0) {
                        useUniformBuffer(g_MaterialParameter);
                    } else if (strcmp(name, "g_LightParam") == 0) {
                        useUniformBuffer(g_LightParam);
                    } else if (strcmp(name, "g_CommonParameter") == 0) {
                        useUniformBuffer(g_CommonParameter);
                    } else if (strcmp(name, "g_CustomizeParameter") == 0) {
                        useUniformBuffer(g_CustomizeParameter);
                    } else if (strcmp(name, "g_SceneParameter") == 0) {
                        useUniformBuffer(g_SceneParameter);
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
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    m_viewPositionBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                                  m_device.swapChain->extent.height,
                                                  VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    m_compositeBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                               m_device.swapChain->extent.height,
                                               VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    m_depthBuffer = m_device.createTexture(m_device.swapChain->extent.width,
                                           m_device.swapChain->extent.height,
                                           VK_FORMAT_D32_SFLOAT,
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    CommonParameter commonParam{};
    commonParam.m_RenderTarget = {1.0f / m_device.swapChain->extent.width,
                                  1.0f / m_device.swapChain->extent.height,
                                  0.0f,
                                  0.0f}; // used to convert screen-space coordinates back into 0.0-1.0

    m_device.copyToBuffer(g_CommonParameter, &commonParam, sizeof(CommonParameter));
}

Texture &GameRenderer::getCompositeTexture()
{
    return m_compositeBuffer;
}

void GameRenderer::bindDescriptorSets(VkCommandBuffer commandBuffer,
                                      GameRenderer::CachedPipeline &pipeline,
                                      const DrawObject *object,
                                      const RenderMaterial *material)
{
    int i = 0;
    for (auto setLayout : pipeline.setLayouts) {
        if (!pipeline.cachedDescriptors.count(i)) {
            if (auto descriptor = createDescriptorFor(object, pipeline, i, material); descriptor != VK_NULL_HANDLE) {
                pipeline.cachedDescriptors[i] = descriptor;
            } else {
                continue;
            }
        }

        // TODO: we can pass all descriptors in one function call
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, i, 1, &pipeline.cachedDescriptors[i], 0, nullptr);

        i++;
    }
}
