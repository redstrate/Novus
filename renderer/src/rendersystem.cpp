// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rendersystem.h"

#include <array>

#include <QDebug>

#include <physis.hpp>

#include "dxbc_module.h"
#include "dxbc_reader.h"
#include "renderer.hpp"
#include <spirv_glsl.hpp>

#include <glm/ext/matrix_clip_space.hpp>

dxvk::Logger dxvk::Logger::s_instance("dxbc.log");

const std::array<std::string, 14> passes = {
    // Shadows?
    "PASS_0",
    "PASS_Z_OPAQUE",
    // Z "Prepass", normals + depth (or is this maybe G_OPAQUE?
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

RenderSystem::RenderSystem(Renderer &renderer, GameData *data)
    : m_renderer(renderer)
    , m_data(data)
{
    directionalLightningShpk = physis_parse_shpk(physis_gamedata_extract_file(m_data, "shader/sm5/shpk/directionallighting.shpk"));

    // camera data
    {
        g_CameraParameter = createUniformBuffer(sizeof(CameraParameter));
    }

    // joint matrix data
    {
        g_JointMatrixArray = createUniformBuffer(sizeof(JointMatrixArray));
        JointMatrixArray jointMatrixArray{};
        for (int i = 0; i < 64; i++) {
            jointMatrixArray.g_JointMatrixArray[i] = glm::mat3x4(1.0f);
        }
        copyDataToUniform(g_JointMatrixArray, &jointMatrixArray, sizeof(JointMatrixArray));
    }

    // instance data
    {
        g_InstanceParameter = createUniformBuffer(sizeof(InstanceParameter));

        InstanceParameter instanceParameter{};
        copyDataToUniform(g_InstanceParameter, &instanceParameter, sizeof(InstanceParameter));
    }

    // model data
    {
        g_ModelParameter = createUniformBuffer(sizeof(ModelParameter));

        ModelParameter modelParameter{};
        copyDataToUniform(g_ModelParameter, &modelParameter, sizeof(ModelParameter));
    }

    // material data
    {
        g_MaterialParameter = createUniformBuffer(sizeof(MaterialParameter));

        MaterialParameter materialParameter{};
        materialParameter.g_AlphaThreshold = 0.0f;
        materialParameter.g_DiffuseColor = glm::vec3(1.0f);
        copyDataToUniform(g_MaterialParameter, &materialParameter, sizeof(MaterialParameter));
    }
}

void RenderSystem::testInit(::RenderModel *m)
{
    RenderModel model{.shpk = physis_parse_shpk(physis_gamedata_extract_file(m_data, "shader/sm5/shpk/character.shpk")),
                      .internal_model = new ::RenderModel(*m)};
    m_renderModels.push_back(model);
}

void RenderSystem::render(uint32_t imageIndex, VkCommandBuffer commandBuffer)
{
    // TODO: this shouldn't be here
    CameraParameter cameraParameter{};

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 1000.0f);
    glm::mat4 viewMatrix = m_renderer.view;
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    cameraParameter.m_ViewMatrix = viewMatrix;
    cameraParameter.m_InverseViewMatrix = glm::inverse(viewMatrix);
    cameraParameter.m_ViewProjectionMatrix = glm::transpose(viewProjectionMatrix);
    cameraParameter.m_InverseViewProjectionMatrix = glm::inverse(viewProjectionMatrix);
    cameraParameter.m_InverseProjectionMatrix = glm::inverse(projectionMatrix);
    cameraParameter.m_ProjectionMatrix = projectionMatrix;
    /*cameraParameter.m_MainViewToProjectionMatrix = glm::mat4(1.0f); // ???
    cameraParameter.m_EyePosition = glm::vec3(5.0f); // placeholder
    cameraParameter.m_LookAtVector = glm::vec3(0.0f); // placeholder*/

    copyDataToUniform(g_CameraParameter, &cameraParameter, sizeof(CameraParameter));

    int i = 0;
    for (const auto pass : passes) {
        beginPass(imageIndex, commandBuffer, pass);

        // hardcoded to the known pass for now
        if (pass == "PASS_G_OPAQUE") {
            for (auto &model : m_renderModels) {
                std::vector<uint32_t> systemKeys;
                std::vector<uint32_t> sceneKeys = {
                    physis_shpk_crc("TransformViewSkin"),
                    physis_shpk_crc("GetAmbientLight_SH"),
                    physis_shpk_crc("GetReflectColor_Texture"),
                    physis_shpk_crc("GetAmbientOcclusion_None"),
                    physis_shpk_crc("ApplyDitherClipOff"),
                };
                std::vector<uint32_t> materialKeys;
                for (int j = 0; j < model.shpk.num_material_keys; j++) {
                    materialKeys.push_back(model.shpk.material_keys[j].default_value);
                }
                std::vector<uint32_t> subviewKeys = {
                    physis_shpk_crc("Default"),
                    physis_shpk_crc("SUB_VIEW_MAIN"),
                };

                const u_int32_t selector = physis_shpk_build_selector_from_all_keys(systemKeys.data(),
                                                                                    systemKeys.size(),
                                                                                    sceneKeys.data(),
                                                                                    sceneKeys.size(),
                                                                                    materialKeys.data(),
                                                                                    materialKeys.size(),
                                                                                    subviewKeys.data(),
                                                                                    subviewKeys.size());
                const physis_SHPKNode node = physis_shpk_get_node(&model.shpk, selector);

                // check if invalid
                if (node.pass_count == 0) {
                    continue;
                }

                // this is an index into the node's pass array, not to get confused with the global one we always follow.
                const int passIndice = node.pass_indices[i];
                if (passIndice != INVALID_PASS) {
                    const Pass currentPass = node.passes[passIndice];

                    const uint32_t vertexShaderIndice = currentPass.vertex_shader;
                    const uint32_t pixelShaderIndice = currentPass.vertex_shader;

                    physis_Shader vertexShader = model.shpk.vertex_shaders[vertexShaderIndice];
                    physis_Shader pixelShader = model.shpk.pixel_shaders[pixelShaderIndice];

                    bindPipeline(commandBuffer, vertexShader, pixelShader);

                    for (const auto &part : model.internal_model->parts) {
                        const uint32_t hash = vertexShader.len + pixelShader.len;
                        auto &cachedPipeline = m_cachedPipelines[hash];

                        int i = 0;
                        for (auto setLayout : cachedPipeline.setLayouts) {
                            if (!cachedPipeline.cachedDescriptors.count(i)) {
                                if (auto descriptor = createDescriptorFor(model, cachedPipeline, i); descriptor != VK_NULL_HANDLE) {
                                    cachedPipeline.cachedDescriptors[i] = descriptor;
                                } else {
                                    continue;
                                }
                            }

                            // TODO: we can pass all descriptors in one function call
                            vkCmdBindDescriptorSets(commandBuffer,
                                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    cachedPipeline.pipelineLayout,
                                                    i,
                                                    1,
                                                    &cachedPipeline.cachedDescriptors[i],
                                                    0,
                                                    nullptr);

                            i++;
                        }

                        VkDeviceSize offsets[] = {0};
                        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &part.vertexBuffer, offsets);
                        vkCmdBindIndexBuffer(commandBuffer, part.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

                        vkCmdDrawIndexed(commandBuffer, part.numIndices, 1, 0, 0, 0);
                    }
                }
            }
        } else if (pass == "PASS_LIGHTING_OPAQUE") {
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

            const u_int32_t selector = physis_shpk_build_selector_from_all_keys(systemKeys.data(),
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
                const uint32_t pixelShaderIndice = currentPass.vertex_shader;

                physis_Shader vertexShader = directionalLightningShpk.vertex_shaders[vertexShaderIndice];
                physis_Shader pixelShader = directionalLightningShpk.pixel_shaders[pixelShaderIndice];

                // TODO: draw plane for directional lighting
            }
        }

        endPass(commandBuffer);

        i++;
    }
}

void RenderSystem::setSize(uint32_t width, uint32_t height)
{
    m_extent = {width, height};
}

void RenderSystem::beginPass(uint32_t imageIndex, VkCommandBuffer commandBuffer, const std::string_view passName)
{
    VkRenderingInfo renderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO};

    renderingInfo.renderArea.extent = m_extent;

    std::vector<VkRenderingAttachmentInfo> colorAttachments;
    VkRenderingAttachmentInfo depthStencilAttachment{};

    if (passName == "PASS_G_OPAQUE") {
        // normals, it seems like
        {
            VkRenderingAttachmentInfo attachmentInfo{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            attachmentInfo.imageView = m_renderer.swapchainViews[imageIndex];
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
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
            attachmentInfo.imageView = m_renderer.depthView;
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentInfo.clearValue.depthStencil.depth = 1.0f;

            depthStencilAttachment = attachmentInfo;
        }
    } else {
        // qWarning() << "Unimplemented pass" << passName;
    }

    renderingInfo.layerCount = 1;
    renderingInfo.pColorAttachments = colorAttachments.data();
    renderingInfo.colorAttachmentCount = colorAttachments.size();

    if (depthStencilAttachment.imageView != VK_NULL_HANDLE) {
        renderingInfo.pDepthAttachment = &depthStencilAttachment;
    }

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
}

void RenderSystem::endPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRendering(commandBuffer);
}

void RenderSystem::bindPipeline(VkCommandBuffer commandBuffer, physis_Shader &vertexShader, physis_Shader &pixelShader)
{
    const uint32_t hash = vertexShader.len + pixelShader.len;
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
        fragmentShaderStageInfo.module = fragmentShaderModule;
        fragmentShaderStageInfo.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

        VkVertexInputBindingDescription binding = {};
        binding.stride = sizeof(Vertex);

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

                vkCreateDescriptorSetLayout(m_renderer.device, &layoutInfo, nullptr, &set.layout);
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

        VkViewport viewport = {};
        viewport.width = 640.0;
        viewport.height = 480.0;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.extent.width = 640;
        scissor.extent.height = 480;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments = {colorBlendAttachment, colorBlendAttachment, colorBlendAttachment};

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.attachmentCount = colorBlendAttachments.size();
        colorBlending.pAttachments = colorBlendAttachments.data();

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

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
        vkCreatePipelineLayout(m_renderer.device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

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
        vkCreateGraphicsPipelines(m_renderer.device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);

        qInfo() << "Created" << pipeline << "for hash" << hash;
        m_cachedPipelines[hash] = CachedPipeline{.pipeline = pipeline,
                                                 .pipelineLayout = pipelineLayout,
                                                 .setLayouts = setLayouts,
                                                 .requestedSets = requestedSets,
                                                 .vertexShader = vertexShader,
                                                 .pixelShader = pixelShader};
    }

    auto &pipeline = m_cachedPipelines[hash];
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline); // TODO: return CachedPipeline&
}

VkShaderModule RenderSystem::convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel)
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
    vkCreateShaderModule(m_renderer.device, &createInfo, nullptr, &shaderModule);

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

spirv_cross::CompilerGLSL RenderSystem::getShaderModuleResources(const physis_Shader &shader)
{
    dxvk::DxbcReader reader(reinterpret_cast<const char *>(shader.bytecode), shader.len);

    dxvk::DxbcModule module(reader);

    dxvk::DxbcModuleInfo info;
    auto result = module.compile(info, "test");

    // glsl.build_combined_image_samplers();

    return spirv_cross::CompilerGLSL(result.code.data(), result.code.dwords());
}

VkDescriptorSet RenderSystem::createDescriptorFor(const RenderModel &model, const CachedPipeline &pipeline, int i)
{
    VkDescriptorSet set;

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = m_renderer.descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &pipeline.setLayouts[i];

    vkAllocateDescriptorSets(m_renderer.device, &allocateInfo, &set);
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
    for (auto binding : pipeline.requestedSets[i].bindings) {
        if (binding.used) {
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

                info->imageView = m_renderer.dummyView;
                info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            } break;
            case VK_DESCRIPTOR_TYPE_SAMPLER: {
                auto info = &imageInfo.emplace_back();
                descriptorWrite.pImageInfo = info;

                info->sampler = m_renderer.dummySampler;
            } break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
                auto info = &bufferInfo.emplace_back();
                descriptorWrite.pBufferInfo = info;

                auto useUniformBuffer = [&info](UniformBuffer &buffer) {
                    info->buffer = buffer.buffer;
                    info->range = buffer.size;
                };

                if (binding.stageFlags == VK_SHADER_STAGE_VERTEX_BIT && z < 4) {
                    auto name = pipeline.vertexShader.scalar_parameters[z].name;
                    qInfo() << "Requesting" << name << "at" << j;

                    if (strcmp(name, "g_CameraParameter") == 0) {
                        useUniformBuffer(g_CameraParameter);
                    } else if (strcmp(name, "g_JointMatrixArray") == 0) {
                        useUniformBuffer(g_JointMatrixArray);
                    } else if (strcmp(name, "g_InstanceParameter") == 0) {
                        useUniformBuffer(g_InstanceParameter);
                    } else if (strcmp(name, "g_ModelParameter") == 0) {
                        useUniformBuffer(g_ModelParameter);
                    } else {
                        qInfo() << "Unknown resource:" << name;
                        info->buffer = m_renderer.dummyBuffer;
                        info->range = 655360;
                    }

                    z++;
                } else if (binding.stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT && j == 0) {
                    // TODO: temporary hack
                    useUniformBuffer(g_MaterialParameter);
                } else {
                    // placeholder buffer so it at least doesn't crash
                    info->buffer = m_renderer.dummyBuffer;
                    info->range = 655360;
                }
            } break;
            }
        }
        j++;
    }

    vkUpdateDescriptorSets(m_renderer.device, writes.size(), writes.data(), 0, nullptr);

    return set;
}

RenderSystem::UniformBuffer RenderSystem::createUniformBuffer(size_t size)
{
    UniformBuffer uniformBuffer{};
    uniformBuffer.size = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(m_renderer.device, &bufferInfo, nullptr, &uniformBuffer.buffer);

    // allocate staging memory
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_renderer.device, uniformBuffer.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        m_renderer.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(m_renderer.device, &allocInfo, nullptr, &uniformBuffer.memory);

    vkBindBufferMemory(m_renderer.device, uniformBuffer.buffer, uniformBuffer.memory, 0);

    return uniformBuffer;
}

void RenderSystem::copyDataToUniform(RenderSystem::UniformBuffer &uniformBuffer, void *data, size_t size)
{
    // copy to staging buffer
    void *mapped_data;
    vkMapMemory(m_renderer.device, uniformBuffer.memory, 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, size);
    vkUnmapMemory(m_renderer.device, uniformBuffer.memory);
}
