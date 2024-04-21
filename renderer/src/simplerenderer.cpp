// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simplerenderer.h"
#include <glm/ext/matrix_transform.hpp>

#include "camera.h"
#include "device.h"
#include "drawobject.h"
#include "swapchain.h"

SimpleRenderer::SimpleRenderer(Device &device)
    : m_device(device)
{
    m_dummyTex = m_device.createDummyTexture();

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
}

void SimpleRenderer::resize()
{
    initRenderPass();
    initDescriptors();
    initPipeline();
    initTextures(m_device.swapChain->extent.width, m_device.swapChain->extent.height);

    std::array<VkImageView, 2> attachments = {m_compositeTexture.imageView, m_depthTexture.imageView};

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_device.swapChain->extent.width;
    framebufferInfo.height = m_device.swapChain->extent.height;
    framebufferInfo.layers = 1;

    vkCreateFramebuffer(m_device.device, &framebufferInfo, nullptr, &m_framebuffer);
}

void SimpleRenderer::render(VkCommandBuffer commandBuffer, uint32_t currentFrame, Camera &camera, const std::vector<DrawObject> &models)
{
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color.float32[0] = 0.24;
    clearValues[0].color.float32[1] = 0.24;
    clearValues[0].color.float32[2] = 0.24;
    clearValues[0].color.float32[3] = 1.0;
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();
    renderPassInfo.renderArea.extent = m_device.swapChain->extent;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (auto model : models) {
        if (model.skinned) {
            if (m_wireframe) {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_skinnedPipelineWireframe);
            } else {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_skinnedPipeline);
            }
        } else {
            if (m_wireframe) {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineWireframe);
            } else {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
            }
        }

        // copy bone data
        {
            const size_t bufferSize = sizeof(glm::mat4) * 128;
            void *mapped_data = nullptr;
            vkMapMemory(m_device.device, model.boneInfoBuffer.memory, 0, bufferSize, 0, &mapped_data);

            memcpy(mapped_data, model.boneData.data(), bufferSize);

            VkMappedMemoryRange range = {};
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.memory = model.boneInfoBuffer.memory;
            range.size = bufferSize;
            vkFlushMappedMemoryRanges(m_device.device, 1, &range);

            vkUnmapMemory(m_device.device, model.boneInfoBuffer.memory);
        }

        for (const auto &part : model.parts) {
            RenderMaterial defaultMaterial = {};

            RenderMaterial *material = nullptr;

            if (static_cast<size_t>(part.materialIndex) >= model.materials.size()) {
                material = &defaultMaterial;
            } else {
                material = &model.materials[part.materialIndex];
            }

            const auto h = hash(model, *material);
            if (!cachedDescriptors.count(h)) {
                if (auto descriptor = createDescriptorFor(model, *material); descriptor != VK_NULL_HANDLE) {
                    cachedDescriptors[h] = descriptor;
                } else {
                    continue;
                }
            }

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &cachedDescriptors[h], 0, nullptr);

            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &part.vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, part.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

            glm::mat4 vp = camera.perspective * camera.view;

            vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4), &vp);

            auto m = glm::mat4(1.0f);
            m = glm::translate(m, model.position);

            vkCmdPushConstants(commandBuffer,
                               m_pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               sizeof(glm::mat4),
                               sizeof(glm::mat4),
                               &m);

            int test = 0;
            vkCmdPushConstants(commandBuffer,
                               m_pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               sizeof(glm::mat4) * 2,
                               sizeof(int),
                               &test);

            int type = (int)material->type;
            vkCmdPushConstants(commandBuffer,
                               m_pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               sizeof(glm::mat4) * 2 + sizeof(int),
                               sizeof(int),
                               &type);

            vkCmdDrawIndexed(commandBuffer, part.numIndices, 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}

void SimpleRenderer::initRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_device.swapChain->surfaceFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(m_device.device, &renderPassInfo, nullptr, &m_renderPass);
}

void SimpleRenderer::initPipeline()
{
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = m_device.loadShaderFromDisk(":/shaders/mesh.vert.spv");
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo skinnedVertexShaderStageInfo = {};
    skinnedVertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    skinnedVertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    skinnedVertexShaderStageInfo.module = m_device.loadShaderFromDisk(":/shaders/skinned.vert.spv");
    skinnedVertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = m_device.loadShaderFromDisk(":/shaders/mesh.frag.spv");
    fragmentShaderStageInfo.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

    VkVertexInputBindingDescription binding = {};
    binding.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, position);

    VkVertexInputAttributeDescription uv0Attribute = {};
    uv0Attribute.format = VK_FORMAT_R32G32_SFLOAT;
    uv0Attribute.location = 1;
    uv0Attribute.offset = offsetof(Vertex, uv0);

    VkVertexInputAttributeDescription uv1Attribute = {};
    uv1Attribute.format = VK_FORMAT_R32G32_SFLOAT;
    uv1Attribute.location = 2;
    uv1Attribute.offset = offsetof(Vertex, uv1);

    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.location = 3;
    normalAttribute.offset = offsetof(Vertex, normal);

    VkVertexInputAttributeDescription bitangentAttribute = {};
    bitangentAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    bitangentAttribute.location = 4;
    bitangentAttribute.offset = offsetof(Vertex, bitangent);

    VkVertexInputAttributeDescription colorAttribute = {};
    colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    colorAttribute.location = 5;
    colorAttribute.offset = offsetof(Vertex, color);

    VkVertexInputAttributeDescription boneWeightAttribute = {};
    boneWeightAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    boneWeightAttribute.location = 6;
    boneWeightAttribute.offset = offsetof(Vertex, bone_weight);

    VkVertexInputAttributeDescription boneIdAttribute = {};
    boneIdAttribute.format = VK_FORMAT_R8G8B8A8_UINT;
    boneIdAttribute.location = 7;
    boneIdAttribute.offset = offsetof(Vertex, bone_id);

    const std::array attributes =
        {positionAttribute, uv0Attribute, uv1Attribute, normalAttribute, bitangentAttribute, colorAttribute, boneWeightAttribute, boneIdAttribute};

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &binding;
    vertexInputState.vertexAttributeDescriptionCount = attributes.size();
    vertexInputState.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.width = m_device.swapChain->extent.width;
    viewport.height = m_device.swapChain->extent.height;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent = m_device.swapChain->extent;

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

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.size = (sizeof(glm::mat4) * 2) + sizeof(int) * 2;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_setLayout;

    vkCreatePipelineLayout(m_device.device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.maxDepthBounds = 1.0f;

    VkGraphicsPipelineCreateInfo createInfo = {};
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
    createInfo.layout = m_pipelineLayout;
    createInfo.renderPass = m_renderPass;

    vkCreateGraphicsPipelines(m_device.device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_pipeline);

    shaderStages[0] = skinnedVertexShaderStageInfo;

    vkCreateGraphicsPipelines(m_device.device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_skinnedPipeline);

    rasterizer.polygonMode = VK_POLYGON_MODE_LINE;

    vkCreateGraphicsPipelines(m_device.device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_skinnedPipelineWireframe);

    shaderStages[0] = vertexShaderStageInfo;

    vkCreateGraphicsPipelines(m_device.device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_pipelineWireframe);
}

void SimpleRenderer::initDescriptors()
{
    VkDescriptorSetLayoutBinding boneInfoBufferBinding = {};
    boneInfoBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    boneInfoBufferBinding.descriptorCount = 1;
    boneInfoBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    boneInfoBufferBinding.binding = 2;

    VkDescriptorSetLayoutBinding textureBinding = {};
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = 1;
    textureBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBinding.binding = 3;

    VkDescriptorSetLayoutBinding normalBinding = {};
    normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalBinding.descriptorCount = 1;
    normalBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    normalBinding.binding = 4;

    VkDescriptorSetLayoutBinding specularBinding = {};
    specularBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    specularBinding.descriptorCount = 1;
    specularBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    specularBinding.binding = 5;

    VkDescriptorSetLayoutBinding multiBinding = {};
    multiBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    multiBinding.descriptorCount = 1;
    multiBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    multiBinding.binding = 6;

    const std::array bindings = {boneInfoBufferBinding, textureBinding, normalBinding, specularBinding, multiBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    vkCreateDescriptorSetLayout(m_device.device, &layoutInfo, nullptr, &m_setLayout);
}

void SimpleRenderer::initTextures(int width, int height)
{
    m_compositeTexture = m_device.createTexture(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    m_depthTexture = m_device.createTexture(width, height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

uint64_t SimpleRenderer::hash(const DrawObject &model, const RenderMaterial &material)
{
    uint64_t hash = 0;
    hash += reinterpret_cast<intptr_t>((void *)&model);
    if (material.diffuseTexture)
        hash += reinterpret_cast<intptr_t>((void *)material.diffuseTexture);
    if (material.normalTexture)
        hash += reinterpret_cast<intptr_t>((void *)material.normalTexture);
    if (material.specularTexture)
        hash += reinterpret_cast<intptr_t>((void *)material.specularTexture);
    if (material.multiTexture)
        hash += reinterpret_cast<intptr_t>((void *)material.multiTexture);
    return hash;
}

VkDescriptorSet SimpleRenderer::createDescriptorFor(const DrawObject &model, const RenderMaterial &material)
{
    VkDescriptorSet set;

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = m_device.descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &m_setLayout;

    vkAllocateDescriptorSets(m_device.device, &allocateInfo, &set);
    if (set == VK_NULL_HANDLE) {
        // qFatal("Failed to create descriptor set!");
        return VK_NULL_HANDLE;
    }

    const size_t bufferSize = sizeof(glm::mat4) * 128;

    std::vector<VkWriteDescriptorSet> writes;

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = model.boneInfoBuffer.buffer;
    bufferInfo.range = bufferSize;

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = set;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.dstBinding = 2;

    writes.push_back(descriptorWrite);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (material.diffuseTexture) {
        imageInfo.imageView = material.diffuseTexture->view;
        imageInfo.sampler = material.diffuseTexture->sampler;
    } else {
        imageInfo.imageView = m_dummyTex.imageView;
        imageInfo.sampler = m_sampler;
    }

    VkWriteDescriptorSet descriptorWrite2 = {};
    descriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite2.dstSet = set;
    descriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite2.descriptorCount = 1;
    descriptorWrite2.pImageInfo = &imageInfo;
    descriptorWrite2.dstBinding = 3;

    writes.push_back(descriptorWrite2);

    VkDescriptorImageInfo normalImageInfo = {};
    normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (material.normalTexture) {
        normalImageInfo.imageView = material.normalTexture->view;
        normalImageInfo.sampler = material.normalTexture->sampler;

        VkWriteDescriptorSet normalDescriptorWrite2 = {};
        normalDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalDescriptorWrite2.dstSet = set;
        normalDescriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        normalDescriptorWrite2.descriptorCount = 1;
        normalDescriptorWrite2.pImageInfo = &normalImageInfo;
        normalDescriptorWrite2.dstBinding = 4;

        writes.push_back(normalDescriptorWrite2);
    }

    VkDescriptorImageInfo specularImageInfo = {};
    specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (material.specularTexture) {
        specularImageInfo.imageView = material.specularTexture->view;
        specularImageInfo.sampler = material.specularTexture->sampler;

        VkWriteDescriptorSet specularDescriptorWrite2 = {};
        specularDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        specularDescriptorWrite2.dstSet = set;
        specularDescriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        specularDescriptorWrite2.descriptorCount = 1;
        specularDescriptorWrite2.pImageInfo = &specularImageInfo;
        specularDescriptorWrite2.dstBinding = 5;

        writes.push_back(specularDescriptorWrite2);
    }

    VkDescriptorImageInfo multiImageInfo = {};
    multiImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (material.multiTexture) {
        multiImageInfo.imageView = material.multiTexture->view;
        multiImageInfo.sampler = material.multiTexture->sampler;

        VkWriteDescriptorSet multiDescriptorWrite2 = {};
        multiDescriptorWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        multiDescriptorWrite2.dstSet = set;
        multiDescriptorWrite2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        multiDescriptorWrite2.descriptorCount = 1;
        multiDescriptorWrite2.pImageInfo = &multiImageInfo;
        multiDescriptorWrite2.dstBinding = 6;

        writes.push_back(multiDescriptorWrite2);
    }

    vkUpdateDescriptorSets(m_device.device, writes.size(), writes.data(), 0, nullptr);

    return set;
}

void SimpleRenderer::addDrawObject(const DrawObject &drawObject)
{
}

Texture &SimpleRenderer::getCompositeTexture()
{
    return m_compositeTexture;
}
