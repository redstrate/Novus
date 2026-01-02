// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <map>
#include <vector>

#include <QString>
#include <glm/ext/matrix_float4x4.hpp>
#include <physis.hpp>
#include <vulkan/vulkan.h>

#include "camera.h"
#include "device.h"
#include "drawobject.h"
#include "scene.h"

class ImGuiPass;
struct ImGuiContext;
class BaseRenderer;
class RendererPass;

/// Render 3D scenes made up of FFXIV game objects
class RenderManager
{
public:
    RenderManager(physis_SqPackResource *data);

    bool initSwapchain(VkSurfaceKHR surface, int width, int height);
    void resize(VkSurfaceKHR surface, int width, int height);

    void destroySwapchain();

    DrawObject *addDrawObject(const physis_MDL &model, int lod);
    void reloadDrawObject(DrawObject &model, uint32_t lod);
    Texture addGameTexture(VkFormat format, physis_Texture gameTexture);

    void render(const std::vector<DrawObjectInstance> &models);

    VkRenderPass presentationRenderPass() const;

    Camera camera;
    Scene scene;

    ImGuiContext *ctx = nullptr;

    Device &device();

    VkSampler defaultSampler();

    void addPass(RendererPass *pass);

    BaseRenderer *renderer();

private:
    void updateCamera(Camera &camera);
    void initBlitPipeline();

    std::array<VkCommandBuffer, 3> m_commandBuffers;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> m_framebuffers;

    ImGuiPass *m_imGuiPass = nullptr;
    Device *m_device = nullptr;
    BaseRenderer *m_renderer = nullptr;
    physis_SqPackResource *m_data = nullptr;
    std::vector<RendererPass *> m_passes;
};
