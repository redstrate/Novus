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

class ImGuiPass;
struct ImGuiContext;
class BaseRenderer;

/// Render 3D scenes made up of FFXIV game objects
class RenderManager
{
public:
    RenderManager(GameData *data);

    bool initSwapchain(VkSurfaceKHR surface, int width, int height);
    void resize(VkSurfaceKHR surface, int width, int height);

    void destroySwapchain();

    DrawObject addDrawObject(const physis_MDL &model, int lod);
    void reloadDrawObject(DrawObject &model, uint32_t lod);
    RenderTexture addTexture(uint32_t width, uint32_t height, const uint8_t *data, uint32_t data_size);

    void render(const std::vector<DrawObject> &models);

    VkRenderPass presentationRenderPass() const;

    Camera camera;

    ImGuiContext *ctx = nullptr;

    Device &device();

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
    GameData *m_data = nullptr;
};