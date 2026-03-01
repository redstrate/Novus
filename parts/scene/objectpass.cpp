// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectpass.h"
#include "device.h"
#include "primitives.h"
#include "rendermanager.h"
#include "simplerenderer.h"
#include "swapchain.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "scenestate.h"

#include <glm/gtc/type_ptr.hpp>

ObjectPass::ObjectPass(RenderManager *renderer, SceneState *appState)
    : m_renderer(renderer)
    , m_device(m_renderer->device())
    , m_appState(appState)
{
    createPipeline();

    Primitives::Initialize(m_renderer);
}

void ObjectPass::render(VkCommandBuffer commandBuffer, Camera &camera)
{
    if (dynamic_cast<SimpleRenderer *>(m_renderer->renderer())) {
        VkDebugUtilsLabelEXT labelExt{};
        labelExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelExt.pLabelName = "Object Pass";
        m_renderer->device().beginDebugMarker(commandBuffer, labelExt);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        // TODO: support nested scenes
        for (const auto &[_, lgb] : m_appState->rootScene.lgbFiles) {
            for (uint32_t i = 0; i < lgb.num_chunks; i++) {
                const auto &chunk = lgb.chunks[i];
                for (uint32_t j = 0; j < chunk.num_layers; j++) {
                    const auto &layer = chunk.layers[j];
                    if (!m_appState->visibleLayerIds.contains(layer.id)) {
                        continue;
                    }

                    for (uint32_t z = 0; z < layer.num_objects; z++) {
                        const auto &object = layer.objects[z];

                        glm::mat4 vp = camera.perspective * camera.view;

                        vkCmdPushConstants(commandBuffer,
                                           m_pipelineLayout,
                                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                           0,
                                           sizeof(glm::mat4),
                                           &vp);

                        auto m = glm::mat4(1.0f);
                        m = glm::translate(m, {object.transform.translation[0], object.transform.translation[1], object.transform.translation[2]});
                        m *= glm::mat4_cast(glm::quat(glm::vec3(object.transform.rotation[0], object.transform.rotation[1], object.transform.rotation[2])));
                        m = glm::scale(m, {object.transform.scale[0], object.transform.scale[1], object.transform.scale[2]});

                        vkCmdPushConstants(commandBuffer,
                                           m_pipelineLayout,
                                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                           sizeof(glm::mat4),
                                           sizeof(glm::mat4),
                                           &m);

                        glm::vec4 color = glm::vec4(1, 0, 0, 1);
                        if (m_appState->selectedObject && m_appState->selectedObject.value() == &object) {
                            color = glm::vec4(0, 1, 0, 1);
                        }

                        vkCmdPushConstants(commandBuffer,
                                           m_pipelineLayout,
                                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                           sizeof(glm::mat4) * 2,
                                           sizeof(glm::vec4),
                                           &color);

                        const auto decideBasedOnTrigger = [commandBuffer](const physis_TriggerBoxInstanceObject &trigger) {
                            switch (trigger.trigger_box_shape) {
                            case TriggerBoxShape::Box:
                                Primitives::DrawCube(commandBuffer);
                                break;
                            case TriggerBoxShape::Sphere:
                                Primitives::DrawSphere(commandBuffer);
                                break;
                            case TriggerBoxShape::Cylinder:
                                Primitives::DrawCylinder(commandBuffer);
                                break;
                            case TriggerBoxShape::Plane:
                                Primitives::DrawPlane(commandBuffer);
                                break;
                                // Unsupported ones
                            case TriggerBoxShape::None:
                            case TriggerBoxShape::Mesh:
                            case TriggerBoxShape::PlaneTwoSided:
                                break;
                            }
                        };

                        switch (object.data.tag) {
                        case physis_LayerEntry::Tag::MapRange:
                            decideBasedOnTrigger(object.data.map_range._0.parent_data);
                            break;
                        case physis_LayerEntry::Tag::EventRange:
                            decideBasedOnTrigger(object.data.event_range._0.parent_data);
                            break;
                        case physis_LayerEntry::Tag::ExitRange:
                            decideBasedOnTrigger(object.data.exit_range._0.parent_data);
                            break;
                        case physis_LayerEntry::Tag::PrefetchRange:
                            decideBasedOnTrigger(object.data.prefetch_range._0.parent_data);
                            break;
                        default:
                            Primitives::DrawSphere(commandBuffer);
                            break;
                        }
                    }
                }
            }
        }

        for (const auto &[_, dropIn] : m_appState->rootScene.dropIns) {
            for (const auto &layer : dropIn.layers) {
                for (const auto &object : layer.objects) {
                    glm::mat4 vp = camera.perspective * camera.view;

                    vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4), &vp);

                    auto m = glm::mat4(1.0f);
                    m = glm::translate(m, glm::make_vec3(object.position));

                    vkCmdPushConstants(commandBuffer,
                                       m_pipelineLayout,
                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                       sizeof(glm::mat4),
                                       sizeof(glm::mat4),
                                       &m);

                    glm::vec4 color = glm::vec4(1, 0, 0, 1);
                    if (m_appState->selectedDropInObject && m_appState->selectedDropInObject.value() == &object) {
                        color = glm::vec4(0, 1, 0, 1);
                    }

                    vkCmdPushConstants(commandBuffer,
                                       m_pipelineLayout,
                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                       sizeof(glm::mat4) * 2,
                                       sizeof(glm::vec4),
                                       &color);

                    Primitives::DrawCube(commandBuffer);
                }
            }
        }

        m_renderer->device().endDebugMarker(commandBuffer);
    } else {
        qWarning() << "Can't render object pass in non-simple renderer for now!!";
    }
}

void ObjectPass::createPipeline()
{
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = m_device.loadShaderFromDisk(":/shaders/debug.vert.spv");
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = m_device.loadShaderFromDisk(":/shaders/debug.frag.spv");
    fragmentShaderStageInfo.pName = "main";

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

    VkVertexInputBindingDescription vertexBindingDescription = {};
    vertexBindingDescription.stride = sizeof(glm::vec3);

    VkVertexInputAttributeDescription positionAttributeDescription = {};
    positionAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = &positionAttributeDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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
    pushConstant.size = sizeof(glm::mat4) * 2 + sizeof(glm::vec4);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

    vkCreatePipelineLayout(m_device.device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.maxDepthBounds = 1.0f;

    auto renderer = dynamic_cast<SimpleRenderer *>(m_renderer->renderer());

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
}
