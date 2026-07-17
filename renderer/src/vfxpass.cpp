// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vfxpass.h"

#include "rendermanager.h"
#include "simplerenderer.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

VfxPass::VfxPass(RenderManager &manager)
    : m_device(manager.device())
    , m_renderer(manager)
{
    createPipeline();
}

void VfxPass::render(const VkCommandBuffer commandBuffer, const Camera &camera, const std::vector<VfxObjectInstance> &vfx)
{
    VkDebugUtilsLabelEXT labelExt{};
    labelExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    labelExt.pLabelName = "VFX Pass";
    m_renderer.device().beginDebugMarker(commandBuffer, labelExt);

    for (const auto &vfxInstance : vfx) {
        for (const auto &model : vfxInstance.sourceObject->models) {
            if (model.numIndices == 0) {
                continue;
            }

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

            auto texture = vfxInstance.sourceObject->gameTextures[0];
            if (!m_cachedTextures.contains(texture.image)) {
                addTexture(texture);
            }
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_cachedTextures[texture.image], 0, nullptr);

            const glm::mat4 vp = camera.perspective * camera.view;
            auto m = glm::mat4(1.0f);
            m = glm::translate(
                m,
                glm::vec3{vfxInstance.transformation.translation[0], vfxInstance.transformation.translation[1], vfxInstance.transformation.translation[2]});
            m *= glm::mat4_cast(
                glm::quat(glm::vec3(vfxInstance.transformation.rotation[0], vfxInstance.transformation.rotation[1], vfxInstance.transformation.rotation[2])));
            m = glm::scale(m, {vfxInstance.transformation.scale[0], vfxInstance.transformation.scale[1], vfxInstance.transformation.scale[2]});

            const auto mvp = vp * m;
            vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);

            constexpr VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model.vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(commandBuffer, model.numIndices, 1, 0, 0, 0);
        }
    }

    m_renderer.device().endDebugMarker(commandBuffer);
}

void VfxPass::createPipeline()
{
    VkDescriptorSetLayoutBinding colorSamplerBinding = {};
    colorSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    colorSamplerBinding.descriptorCount = 1;
    colorSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &colorSamplerBinding;

    vkCreateDescriptorSetLayout(m_device.device, &layoutInfo, nullptr, &m_setLayout);

    auto debugVertexShader = m_device.loadShaderFromDisk(":/shaders/vfx.vert.spv");
    auto debugFragmentShader = m_device.loadShaderFromDisk(":/shaders/vfx.frag.spv");

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = debugVertexShader;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = debugFragmentShader;
    fragmentShaderStageInfo.pName = "main";

    const std::array shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

    VkVertexInputBindingDescription vertexBindingDescription = {};
    vertexBindingDescription.stride = sizeof(DrawVertex);

    VkVertexInputAttributeDescription positionAttributeDescription = {};
    positionAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttributeDescription.offset = offsetof(DrawVertex, position);

    VkVertexInputAttributeDescription uv1AttributeDescription = {};
    uv1AttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    uv1AttributeDescription.offset = offsetof(DrawVertex, uv1);
    uv1AttributeDescription.location = 1;

    std::array attributes = {positionAttributeDescription, uv1AttributeDescription};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    constexpr std::array dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPushConstantRange pushConstant = {};
    pushConstant.size = sizeof(glm::mat4);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_setLayout;

    vkCreatePipelineLayout(m_device.device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.maxDepthBounds = 1.0f;

    auto renderer = dynamic_cast<SimpleRenderer *>(m_renderer.renderer());

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = renderer->renderPass();

    vkCreateGraphicsPipelines(m_device.device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline);

    vkDestroyShaderModule(m_device.device, debugVertexShader, nullptr);
    vkDestroyShaderModule(m_device.device, debugFragmentShader, nullptr);
}

void VfxPass::addTexture(const Texture &texture)
{
    VkDescriptorSet set;

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = m_device.descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &m_setLayout;

    vkAllocateDescriptorSets(m_device.device, &allocateInfo, &set);
    if (set == VK_NULL_HANDLE) {
        qWarning("Failed to allocate a new descriptor set!");
    }

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.imageView;
    imageInfo.sampler = m_renderer.defaultSampler();

    VkWriteDescriptorSet tableDescriptorWrite = {};
    tableDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    tableDescriptorWrite.dstSet = set;
    tableDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    tableDescriptorWrite.descriptorCount = 1;
    tableDescriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device.device, 1, &tableDescriptorWrite, 0, nullptr);

    m_cachedTextures[texture.image] = set;
}
