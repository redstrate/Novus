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
    createBillboardPipeline();

    // for billboard textures
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.maxLod = 1.0f;

    vkCreateSampler(m_device.device, &samplerInfo, nullptr, &m_sampler);

    m_pointLightTexture = addTexture(QStringLiteral(":/point.png"));
    m_spotLightTexture = addTexture(QStringLiteral(":/spot.png"));
    m_planeLightTexture = addTexture(QStringLiteral(":/plane.png"));
    m_directionalLightTexture = addTexture(QStringLiteral(":/directional.png"));
    m_lineLightTexture = addTexture(QStringLiteral(":/line.png"));
    m_chairTexture = addTexture(QStringLiteral(":/chair.png"));

    Primitives::Initialize(m_renderer);
}

ObjectPass::~ObjectPass()
{
    Primitives::Cleanup(m_renderer);

    vkDestroyPipelineLayout(m_device.device, m_pipelineLayout, nullptr);
    vkDestroyPipeline(m_device.device, m_pipeline, nullptr);
}

void ObjectPass::render(VkCommandBuffer commandBuffer, Camera &camera, const Scene &scene, const std::vector<DrawObjectInstance> &models)
{
    if (dynamic_cast<SimpleRenderer *>(m_renderer->renderer())) {
        VkDebugUtilsLabelEXT labelExt{};
        labelExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelExt.pLabelName = "Object Pass";
        m_renderer->device().beginDebugMarker(commandBuffer, labelExt);

        addScene(commandBuffer, camera, m_appState->rootScene);

        // Draw AABB boundaries
        if (scene.debugFrustumCulling) {
            for (const auto &model : models) {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

                const auto scale = glm::vec3{
                    (model.lastBoundingBox.max[0] - model.lastBoundingBox.min[0]) / 2.0f,
                    (model.lastBoundingBox.max[1] - model.lastBoundingBox.min[1]) / 2.0f,
                    (model.lastBoundingBox.max[2] - model.lastBoundingBox.min[2]) / 2.0f,
                };
                const auto offset = glm::vec3{
                    (model.lastBoundingBox.min[0] + model.lastBoundingBox.max[0]) / 2.0f,
                    (model.lastBoundingBox.min[1] + model.lastBoundingBox.max[1]) / 2.0f,
                    (model.lastBoundingBox.min[2] + model.lastBoundingBox.max[2]) / 2.0f,
                };

                const glm::mat4 vp = camera.perspective * camera.view;
                vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4), &vp);

                auto m = glm::mat4(1.0f);
                m = glm::translate(m, offset);
                m = glm::scale(m, scale);
                vkCmdPushConstants(commandBuffer,
                                   m_pipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                   sizeof(glm::mat4),
                                   sizeof(glm::mat4),
                                   &m);

                const auto color = glm::vec4(0, 0, 1, 1);
                vkCmdPushConstants(commandBuffer,
                                   m_pipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                   sizeof(glm::mat4) * 2,
                                   sizeof(glm::vec4),
                                   &color);

                Primitives::DrawCube(commandBuffer);
            }
        }

        m_renderer->device().endDebugMarker(commandBuffer);
    } else {
        qWarning() << "Can't render object pass in non-simple renderer for now!!";
    }
}

void ObjectPass::createPipeline()
{
    auto debugVertexShader = m_device.loadShaderFromDisk(":/shaders/debug.vert.spv");
    auto debugFragmentShader = m_device.loadShaderFromDisk(":/shaders/debug.frag.spv");

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

    vkDestroyShaderModule(m_device.device, debugVertexShader, nullptr);
    vkDestroyShaderModule(m_device.device, debugFragmentShader, nullptr);
}

void ObjectPass::createBillboardPipeline()
{
    VkDescriptorSetLayoutBinding colorSamplerBinding = {};
    colorSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    colorSamplerBinding.descriptorCount = 1;
    colorSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    colorSamplerBinding.binding = 1;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &colorSamplerBinding;

    vkCreateDescriptorSetLayout(m_device.device, &layoutInfo, nullptr, &m_setLayout);

    auto debugVertexShader = m_device.loadShaderFromDisk(":/shaders/billboard.vert.spv");
    auto debugFragmentShader = m_device.loadShaderFromDisk(":/shaders/billboard.frag.spv");

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

    const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStageInfo, fragmentShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
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
    pushConstant.size = sizeof(glm::mat4) * 2 + sizeof(glm::vec4) * 2;
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_setLayout;

    vkCreatePipelineLayout(m_device.device, &pipelineLayoutInfo, nullptr, &m_billboardPipelineLayout);

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
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
    pipelineInfo.layout = m_billboardPipelineLayout;
    pipelineInfo.renderPass = renderer->renderPass();

    vkCreateGraphicsPipelines(m_device.device, nullptr, 1, &pipelineInfo, nullptr, &m_billboardPipeline);

    vkDestroyShaderModule(m_device.device, debugVertexShader, nullptr);
    vkDestroyShaderModule(m_device.device, debugFragmentShader, nullptr);
}

void ObjectPass::addScene(VkCommandBuffer commandBuffer, Camera &camera, const ObjectScene &scene)
{
    for (const auto &[_, lgb] : scene.lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            const auto &chunk = lgb.chunks[i];
            for (uint32_t j = 0; j < chunk.num_layers; j++) {
                const auto &layer = chunk.layers[j];
                if (!m_appState->visibleLayerIds.contains(layer.id)) {
                    continue;
                }

                addLayer(commandBuffer, camera, layer);
            }
        }
    }

    for (const auto &lgb : scene.embeddedLgbs) {
        for (uint32_t j = 0; j < lgb.layer_count; j++) {
            const auto &layer = lgb.layers[j];
            if (!m_appState->visibleLayerIds.contains(layer.id)) {
                continue;
            }

            addLayer(commandBuffer, camera, layer);
        }
    }

    for (const auto &[_, dropIn] : scene.dropIns) {
        for (const auto &layer : dropIn.layers) {
            for (const auto &object : layer.objects) {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

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

    for (const auto &[_, scene] : scene.nestedScenes) {
        addScene(commandBuffer, camera, scene);
    }
}

void ObjectPass::addLayer(VkCommandBuffer commandBuffer, const Camera &camera, const physis_Layer &layer)
{
    for (uint32_t z = 0; z < layer.num_objects; z++) {
        const auto &object = layer.objects[z];

        glm::mat4 vp = camera.perspective * camera.view;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4), &vp);

        const auto setModel = [this, &object, &commandBuffer](const glm::vec3 relativePosition) {
            auto m = glm::mat4(1.0f);
            m = glm::translate(m,
                               glm::vec3{object.transform.translation[0], object.transform.translation[1], object.transform.translation[2]} + relativePosition);
            m *= glm::mat4_cast(glm::quat(glm::vec3(object.transform.rotation[0], object.transform.rotation[1], object.transform.rotation[2])));
            m = glm::scale(m, {object.transform.scale[0], object.transform.scale[1], object.transform.scale[2]});

            vkCmdPushConstants(commandBuffer,
                               m_pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               sizeof(glm::mat4),
                               sizeof(glm::mat4),
                               &m);
        };
        setModel({});

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

        const auto pos = glm::vec3{object.transform.translation[0], object.transform.translation[1], object.transform.translation[2]};

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
        case physis_LayerEntry::Tag::PopRange:
            for (uint32_t i = 0; i < object.data.pop_range._0.position_count; i++) {
                setModel({object.data.pop_range._0.positions[i][0], object.data.pop_range._0.positions[i][1], object.data.pop_range._0.positions[i][2]});
                Primitives::DrawSphere(commandBuffer);
            }
            setModel({}); // reset position
            Primitives::DrawCube(commandBuffer); // draw normally for now
            break;
        case physis_LayerEntry::Tag::LayLight: {
            const auto lightColor = glm::vec4(object.data.lay_light._0.diffuse_color_hdri.red / 255.0f,
                                              object.data.lay_light._0.diffuse_color_hdri.green / 255.0f,
                                              object.data.lay_light._0.diffuse_color_hdri.blue / 255.0f,
                                              object.data.lay_light._0.diffuse_color_hdri.alpha / 255.0f);
            switch (object.data.lay_light._0.light_type) {
            case LightType::None:
                break;
            case LightType::Directional:
                drawBillboard(commandBuffer, camera, m_directionalLightTexture, lightColor, pos);
                break;
            case LightType::Point:
                drawBillboard(commandBuffer, camera, m_pointLightTexture, lightColor, pos);
                break;
            case LightType::Spot:
                drawBillboard(commandBuffer, camera, m_spotLightTexture, lightColor, pos);
                break;
            case LightType::Plane:
                drawBillboard(commandBuffer, camera, m_planeLightTexture, lightColor, pos);
                break;
            case LightType::Line:
                drawBillboard(commandBuffer, camera, m_lineLightTexture, lightColor, pos);
                break;
            default:
                qWarning() << "Unsupported light type for billboard:" << static_cast<int>(object.data.lay_light._0.light_type);
                break;
            }
        } break;
        case physis_LayerEntry::Tag::SharedGroup:
            break; // Don't render SGBs because it just creates noise, it has visible child objects anyway!
        case physis_LayerEntry::Tag::BG:
            break; // Don't render BG models because it also creates noise, and they have a visible model!
        case physis_LayerEntry::Tag::ChairMarker:
            drawBillboard(commandBuffer, camera, m_chairTexture, glm::vec4(1), pos);
            break;
        default:
            Primitives::DrawCube(commandBuffer);
            break;
        }
    }
}

void ObjectPass::drawBillboard(VkCommandBuffer commandBuffer, const Camera &camera, const Texture &texture, glm::vec4 color, glm::vec3 position)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_billboardPipeline);
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_billboardPipelineLayout,
                            0,
                            1,
                            &m_cachedBillboardTextures[texture.image],
                            0,
                            nullptr);

    glm::mat4 vp = camera.perspective * camera.view;
    vp = glm::translate(vp, position);
    vkCmdPushConstants(commandBuffer, m_billboardPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4), &vp);
    vkCmdPushConstants(commandBuffer,
                       m_billboardPipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       sizeof(glm::mat4),
                       sizeof(glm::mat4),
                       &camera.view);

    vkCmdPushConstants(commandBuffer,
                       m_billboardPipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       sizeof(glm::mat4) * 2,
                       sizeof(glm::vec4),
                       &color);

    const glm::vec4 scale = glm::vec4(1, 1, 1, 0);
    vkCmdPushConstants(commandBuffer,
                       m_billboardPipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       sizeof(glm::mat4) * 2 + sizeof(glm::vec4),
                       sizeof(glm::vec4),
                       &scale);

    vkCmdDraw(commandBuffer, 4, 1, 0, 0);
}

Texture ObjectPass::addTexture(const QString &path)
{
    QImage image(path);

    // TODO: haha, no
    physis_Texture texture{};
    texture.attribute = TextureAttribute_TEXTURE_TYPE2_D;
    texture.format = TextureFormat::B8G8R8A8_UNORM;
    texture.width = image.width();
    texture.height = image.height();
    texture.depth = 1;
    texture.data = reinterpret_cast<uint8_t *>(image.bits());
    texture.data_size = image.sizeInBytes();
    texture.mip_levels = 1;

    auto tex = m_renderer->addGameTexture(texture);
    m_renderer->device().nameTexture(tex, path.toStdString());

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
    imageInfo.imageView = tex.imageView;
    imageInfo.sampler = m_sampler;

    VkWriteDescriptorSet tableDescriptorWrite = {};
    tableDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    tableDescriptorWrite.dstSet = set;
    tableDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    tableDescriptorWrite.descriptorCount = 1;
    tableDescriptorWrite.pImageInfo = &imageInfo;
    tableDescriptorWrite.dstBinding = 1;

    vkUpdateDescriptorSets(m_device.device, 1, &tableDescriptorWrite, 0, nullptr);

    m_cachedBillboardTextures[tex.image] = set;

    return tex;
}
