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
#include "shadermanager.h"
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

    void render(VkCommandBuffer commandBuffer, uint32_t currentFrame, Camera &camera, Scene &scene, const std::vector<DrawObject> &models) override;

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
    CachedPipeline &bindPipeline(VkCommandBuffer commandBuffer,
                                 std::string_view passName,
                                 physis_Shader &vertexShader,
                                 physis_Shader &pixelShader,
                                 std::string_view shaderName);

    void createImageResources();

    physis_SHPK directionalLightningShpk;
    physis_SHPK createViewPositionShpk;
    physis_SHPK backgroundShpk;

    // combined vertex + pixel code length
    std::unordered_map<uint32_t, CachedPipeline> m_cachedPipelines;

    Device &m_device;
    GameData *m_data = nullptr;
    ShaderManager m_shaderManager;

    VkDescriptorSet
    createDescriptorFor(const DrawObject *object, const CachedPipeline &cachedPipeline, int i, const RenderMaterial *material, std::string_view pass);
    void bindDescriptorSets(VkCommandBuffer commandBuffer,
                            CachedPipeline &pipeline,
                            const DrawObject *object,
                            const RenderMaterial *material,
                            std::string_view pass);

    Buffer g_CameraParameter;
    Buffer g_InstanceParameter;
    Buffer g_ModelParameter;
    Buffer g_TransparencyMaterialParameter;
    Buffer g_CommonParameter;
    Buffer g_LightParam;
    Buffer g_SceneParameter;
    Buffer g_CustomizeParameter;
    Buffer g_MaterialParameterDynamic;
    Buffer g_DecalColor;
    Buffer g_AmbientParam;
    Buffer g_ShaderTypeParameter;

    Buffer m_planeVertexBuffer;

    Texture m_normalGBuffer;
    Texture m_viewPositionBuffer;
    Texture m_depthBuffer;
    Texture m_lightBuffer;
    Texture m_lightSpecularBuffer;
    Texture m_compositeBuffer;
    Texture m_ZBuffer; // what is this?
    Texture m_dummyTex;
    VkSampler m_sampler;
    VkSampler m_normalSampler;
    Buffer m_dummyBuffer;
    Texture m_tileNormal;
    Texture m_tileDiffuse;

    // Dawntrail changes part of the rendering system
    bool m_dawntrailMode = false;
};