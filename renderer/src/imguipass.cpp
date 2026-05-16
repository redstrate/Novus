// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imguipass.h"

#include <QDebug>
#include <array>
#include <glm/glm.hpp>
#include <imgui.h>

#include "rendermanager.h"

ImGuiPass::ImGuiPass(RenderManager &renderer)
    : m_renderer(renderer)
{
    createDescriptorSetLayout();
    createPipeline();
    createFontImage();
}

ImGuiPass::~ImGuiPass()
{
    m_renderer.device().destroyBuffer(m_indexBuffer);
    m_renderer.device().destroyBuffer(m_vertexBuffer);

    m_renderer.device().destroyTexture(m_fontAtlas);

    vkDestroyPipeline(m_renderer.device().device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_renderer.device().device, m_pipelineLayout, nullptr);

    vkDestroyDescriptorSetLayout(m_renderer.device().device, m_setLayout, nullptr);
}

void ImGuiPass::render(VkCommandBuffer commandBuffer)
{
    ImDrawData *drawData = ImGui::GetDrawData();
    if (drawData == nullptr) {
        return;
    }

    const size_t newVertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
    if (newVertexSize > m_vertexBuffer.size) {
        m_renderer.device().destroyBuffer(m_vertexBuffer);
        m_vertexBuffer = m_renderer.device().createBuffer(newVertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_renderer.device().nameBuffer(m_vertexBuffer, "ImGui Vertex Buffer");
    }

    const size_t newIndexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
    if (newIndexSize > m_indexBuffer.size) {
        m_renderer.device().destroyBuffer(m_indexBuffer);
        m_indexBuffer = m_renderer.device().createBuffer(newIndexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        m_renderer.device().nameBuffer(m_indexBuffer, "ImGui Index Buffer");
    }

    if (m_vertexBuffer.size == 0 || m_indexBuffer.size == 0)
        return;

    ImDrawVert *vertexData = nullptr;
    ImDrawIdx *indexData = nullptr;
    vkMapMemory(m_renderer.device().device, m_vertexBuffer.memory, 0, m_vertexBuffer.size, 0, reinterpret_cast<void **>(&vertexData));
    vkMapMemory(m_renderer.device().device, m_indexBuffer.memory, 0, m_indexBuffer.size, 0, reinterpret_cast<void **>(&indexData));

    for (int i = 0; i < drawData->CmdListsCount; i++) {
        const ImDrawList *cmd_list = drawData->CmdLists[i];

        memcpy(vertexData, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        vertexData += cmd_list->VtxBuffer.Size;

        memcpy(indexData, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        indexData += cmd_list->IdxBuffer.Size;
    }

    vkUnmapMemory(m_renderer.device().device, m_vertexBuffer.memory);
    vkUnmapMemory(m_renderer.device().device, m_indexBuffer.memory);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    float scale[2];
    scale[0] = 2.0f / drawData->DisplaySize.x;
    scale[1] = 2.0f / drawData->DisplaySize.y;

    float translate[2];
    translate[0] = -1.0f - drawData->DisplayPos.x * scale[0];
    translate[1] = -1.0f - drawData->DisplayPos.y * scale[1];

    vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
    vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);

    int vertexOffset = 0, indexOffset = 0;
    const ImVec2 displayPos = drawData->DisplayPos;
    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList *cmd_list = drawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (m_descriptorSets.count((VkImageView)pcmd->TextureId)) {
                vkCmdBindDescriptorSets(commandBuffer,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        m_pipelineLayout,
                                        0,
                                        1,
                                        &m_descriptorSets[(VkImageView)pcmd->TextureId],
                                        0,
                                        nullptr);
            } else {
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_renderer.device().descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &m_setLayout;

                VkDescriptorSet set = nullptr;
                vkAllocateDescriptorSets(m_renderer.device().device, &allocInfo, &set);

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = static_cast<VkImageView>(pcmd->TextureId);
                imageInfo.sampler = m_renderer.defaultSampler();

                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.dstSet = set;
                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(m_renderer.device().device, 1, &descriptorWrite, 0, nullptr);

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &set, 0, nullptr);

                m_descriptorSets[static_cast<VkImageView>(pcmd->TextureId)] = set;
            }

            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                VkRect2D scissor;
                scissor.offset.x = (int32_t)(pcmd->ClipRect.x - displayPos.x) > 0 ? (int32_t)(pcmd->ClipRect.x - displayPos.x) : 0;
                scissor.offset.y = (int32_t)(pcmd->ClipRect.y - displayPos.y) > 0 ? (int32_t)(pcmd->ClipRect.y - displayPos.y) : 0;
                scissor.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissor.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1);

                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            }

            indexOffset += pcmd->ElemCount;
        }

        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

void ImGuiPass::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &samplerBinding;

    vkCreateDescriptorSetLayout(m_renderer.device().device, &createInfo, nullptr, &m_setLayout);
}

void ImGuiPass::createPipeline()
{
    VkShaderModule vertShaderModule = m_renderer.device().loadShaderFromDisk(":/shaders/imgui.vert.spv");
    VkShaderModule fragShaderModule = m_renderer.device().loadShaderFromDisk(":/shaders/imgui.frag.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

    VkVertexInputBindingDescription vertexBindingDescription = {};
    vertexBindingDescription.stride = sizeof(ImDrawVert);
    vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    positionAttribute.offset = offsetof(ImDrawVert, pos);

    VkVertexInputAttributeDescription uvAttribute = {};
    uvAttribute.location = 1;
    uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvAttribute.offset = offsetof(ImDrawVert, uv);

    VkVertexInputAttributeDescription colorAttribute = {};
    colorAttribute.location = 2;
    colorAttribute.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttribute.offset = offsetof(ImDrawVert, col);

    const std::array<VkVertexInputAttributeDescription, 3> attributes = {positionAttribute, uvAttribute, colorAttribute};

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

    const std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPushConstantRange pushConstant = {};
    pushConstant.size = sizeof(glm::vec4);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_setLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    vkCreatePipelineLayout(m_renderer.device().device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
    depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

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
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    pipelineInfo.renderPass = m_renderer.presentationRenderPass();

    vkCreateGraphicsPipelines(m_renderer.device().device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline);

    vkDestroyShaderModule(m_renderer.device().device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_renderer.device().device, vertShaderModule, nullptr);
}

void ImGuiPass::createFontImage()
{
    ImGuiIO &io = ImGui::GetIO();

    unsigned char *pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // TODO: haha, no
    physis_Texture texture{};
    texture.attribute = TextureAttribute_TEXTURE_TYPE2_D;
    texture.format = TextureFormat::B8G8R8A8_UNORM; // TDOO: this means we put in the wrong data..
    texture.width = width;
    texture.height = height;
    texture.depth = 1;
    texture.data = pixels;
    texture.data_size = width * height * 4;
    texture.mip_levels = 1;

    m_fontAtlas = m_renderer.addGameTexture(texture);
    m_renderer.device().nameTexture(m_fontAtlas, "ImGui Font Atlas");

    io.Fonts->SetTexID(static_cast<ImTextureID>(m_fontAtlas.imageView));
}
