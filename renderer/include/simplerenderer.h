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
#include "texture.h"

class Renderer;
struct RenderModel;
class Device;
struct DrawObject;
struct RenderMaterial;

/// Performs rendering with a basic set of shaders. Can be run without real game data.
class SimpleRenderer : public BaseRenderer
{
public:
    explicit SimpleRenderer(Device &device);

    void resize() override;

    void render(VkCommandBuffer commandBuffer, uint32_t currentFrame, Camera &camera, const std::vector<DrawObject> &models) override;

    void addDrawObject(const DrawObject &drawObject) override;

    Texture &getCompositeTexture() override;

private:
    void initRenderPass();
    void initPipeline();
    void initDescriptors();
    void initTextures(int width, int height);

    VkDescriptorSet createDescriptorFor(const DrawObject &model, const RenderMaterial &material);
    uint64_t hash(const DrawObject &model, const RenderMaterial &material);

    Texture m_dummyTex;
    VkSampler m_sampler = VK_NULL_HANDLE;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipeline m_skinnedPipeline = VK_NULL_HANDLE;
    VkPipeline m_pipelineWireframe = VK_NULL_HANDLE;
    VkPipeline m_skinnedPipelineWireframe = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    bool m_wireframe = false;

    VkFramebuffer m_framebuffer;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;

    std::map<uint64_t, VkDescriptorSet> cachedDescriptors;

    Texture m_depthTexture;
    Texture m_compositeTexture;

    Device &m_device;
};