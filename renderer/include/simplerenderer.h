// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDebug>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "baserenderer.h"
#include "buffer.h"
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
    ~SimpleRenderer() override;

    void resize() override;

    void render(VkCommandBuffer commandBuffer, Camera &camera, Scene &scene, std::vector<DrawObjectInstance> &models) override;

    Texture &getCompositeTexture() override;

    Device &device() override;

    VkFramebuffer framebuffer() const;
    VkRenderPass renderPass() const;

    void freeResources() override;

private:
    void initRenderPass();
    void initPipeline();
    void initDescriptors();
    void initTextures(int width, int height);

    void destroyTextures();
    void destroyPipelines() const;
    void destroyDescriptors() const;
    void destroyRenderPass() const;

    VkDescriptorSet createDescriptorFor(const DrawObject &model, const RenderMaterial &material) const;

    Texture m_dummyTex;
    VkSampler m_sampler = VK_NULL_HANDLE;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipeline m_skinnedPipeline = VK_NULL_HANDLE;
    VkPipeline m_pipelineWireframe = VK_NULL_HANDLE;
    VkPipeline m_skinnedPipelineWireframe = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;

    std::map<size_t, VkDescriptorSet> m_cachedDescriptors;

    Texture m_depthTexture;
    Texture m_compositeTexture;

    Buffer m_lightsBuffer;

    Device &m_device;
};
