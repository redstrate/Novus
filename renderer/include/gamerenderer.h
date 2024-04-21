// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDebug>
#include <string_view>

#include <glm/glm.hpp>
#include <physis.hpp>
#include <spirv.hpp>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <vulkan/vulkan.h>

#include "baserenderer.h"
#include "buffer.h"
#include "drawobject.h"
#include "shaderstructs.h"
#include "texture.h"

class Device;
struct DrawObject;

/// Performs rendering by using the game's existing shaders.
class GameRenderer : public BaseRenderer
{
public:
    GameRenderer(Device &device, GameData *data);

    void resize() override;

    void render(VkCommandBuffer commandBuffer, uint32_t currentFrame, Camera &camera, const std::vector<DrawObject> &models) override;

    Texture &getCompositeTexture() override;

private:
    struct RequestedBinding {
        VkDescriptorType type;
        VkShaderStageFlags stageFlags;
        bool used = false;
    };

    struct RequestedSet {
        bool used = true;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::vector<RequestedBinding> bindings;
    };

    struct CachedPipeline {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> setLayouts;
        std::map<uint64_t, VkDescriptorSet> cachedDescriptors;
        std::vector<RequestedSet> requestedSets;
        physis_Shader vertexShader, pixelShader;
    };

    void beginPass(uint32_t imageIndex, VkCommandBuffer commandBuffer, std::string_view passName);
    void endPass(VkCommandBuffer commandBuffer, std::string_view passName);
    CachedPipeline &bindPipeline(VkCommandBuffer commandBuffer, std::string_view passName, physis_Shader &vertexShader, physis_Shader &pixelShader);
    VkShaderModule convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel);
    spirv_cross::CompilerGLSL getShaderModuleResources(const physis_Shader &shader);

    void createImageResources();

    physis_SHPK directionalLightningShpk;
    physis_SHPK createViewPositionShpk;

    // combined vertex + pixel code length
    std::unordered_map<uint32_t, CachedPipeline> m_cachedPipelines;

    Device &m_device;
    GameData *m_data = nullptr;

    VkDescriptorSet createDescriptorFor(const DrawObject *object, const CachedPipeline &cachedPipeline, int i, const RenderMaterial *material);
    void bindDescriptorSets(VkCommandBuffer commandBuffer, CachedPipeline &pipeline, const DrawObject *object, const RenderMaterial *material);

    Buffer g_CameraParameter;
    Buffer g_InstanceParameter;
    Buffer g_ModelParameter;
    Buffer g_MaterialParameter;
    Buffer g_CommonParameter;
    Buffer g_LightParam;
    Buffer g_SceneParameter;
    Buffer g_CustomizeParameter;

    Buffer m_planeVertexBuffer;

    Texture m_normalGBuffer;
    Texture m_viewPositionBuffer;
    Texture m_depthBuffer;
    Texture m_compositeBuffer;
    Texture m_dummyTex;
    VkSampler m_sampler;
    Buffer m_dummyBuffer;
};