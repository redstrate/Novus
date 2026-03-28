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
    ~SimpleRenderer();

    void resize() override;

    void render(VkCommandBuffer commandBuffer, Camera &camera, Scene &scene, const std::vector<DrawObjectInstance> &models) override;

    Texture &getCompositeTexture() override;

    Device &device() override;

    VkFramebuffer framebuffer();
    VkRenderPass renderPass();

    void freeResources() override;

private:
    void initRenderPass();
    void initPipeline();
    void initDescriptors();
    void initTextures(int width, int height);

    void destroyTextures();
    void destroyPipelines();
    void destroyDescriptors();
    void destroyRenderPass();

    VkDescriptorSet createDescriptorFor(const DrawObject &model, const RenderMaterial &material);

    Texture m_dummyTex;
    VkSampler m_sampler = VK_NULL_HANDLE;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipeline m_skinnedPipeline = VK_NULL_HANDLE;
    VkPipeline m_pipelineWireframe = VK_NULL_HANDLE;
    VkPipeline m_skinnedPipelineWireframe = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    bool m_wireframe = false;

    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;

    std::map<size_t, VkDescriptorSet> cachedDescriptors;

    Texture m_depthTexture;
    Texture m_compositeTexture;

    Device &m_device;
};
