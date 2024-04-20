// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDebug>
#include <string_view>

#include <physis.hpp>
#include <spirv.hpp>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <vulkan/vulkan.h>

class Renderer;
struct RenderModel;

class RenderSystem
{
public:
    RenderSystem(Renderer &renderer, GameData *data);

    void testInit(::RenderModel *m);

    void render(uint32_t imageIndex, VkCommandBuffer commandBuffer);

    void setSize(uint32_t width, uint32_t height);

private:
    void beginPass(uint32_t imageIndex, VkCommandBuffer commandBuffer, std::string_view passName);
    void endPass(VkCommandBuffer commandBuffer);
    void bindPipeline(VkCommandBuffer commandBuffer, physis_Shader &vertexShader, physis_Shader &pixelShader);
    VkShaderModule convertShaderModule(const physis_Shader &shader, spv::ExecutionModel executionModel);
    spirv_cross::CompilerGLSL getShaderModuleResources(const physis_Shader &shader);

    struct RenderModel {
        physis_SHPK shpk;

        ::RenderModel *internal_model = nullptr;
    };
    std::vector<RenderModel> m_renderModels;

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
    };

    // combined vertex + pixel code length
    std::unordered_map<uint32_t, CachedPipeline> m_cachedPipelines;

    Renderer &m_renderer;
    GameData *m_data = nullptr;
    VkExtent2D m_extent = {640, 480};

    VkDescriptorSet createDescriptorFor(const RenderModel &model, const CachedPipeline &cachedPipeline, int i);
};