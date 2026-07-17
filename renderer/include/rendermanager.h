// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <vector>

#include <QImage>
#include <physis.hpp>
#include <vulkan/vulkan.h>

#include "camera.h"
#include "device.h"
#include "drawobject.h"
#include "scene.h"
#include "vfxobject.h"

class FileCache;
class ImGuiPass;
struct ImGuiContext;
class BaseRenderer;
class RendererPass;
class VfxPass;

/// Render 3D scenes made up of FFXIV game objects
class RenderManager
{
public:
    explicit RenderManager(FileCache &cache);
    ~RenderManager();

    bool initSwapchain(VkSurfaceKHR surface, int width, int height);
    void resize(VkSurfaceKHR surface, int width, int height);

    void destroySwapchain(bool keepSwapchainObject);

    DrawObject *addDrawObject(const physis_MDL &model, const std::string &name) const;
    void reloadDrawObject(DrawObject &DrawObject) const;
    void destroyDrawObject(DrawObject &model) const;

    VfxObject *addVFXObject(const physis_Avfx &vfx, const std::vector<physis_Texture> &textures, const std::string &name) const;
    void reloadVFXObject(VfxObject &vfx) const;
    void destroyVFXObject(const VfxObject &vfx);

    /**
     * @brief Converts the @p gameTexture to the renderable kind.
     *
     * @note The physis_Texture is not used internally by RenderManager, can be freed as soon as the call returns.
     */
    Texture addGameTexture(const physis_Texture &gameTexture) const;

    void render(std::vector<DrawObjectInstance> &models, const std::vector<VfxObjectInstance> &vfx);

    VkRenderPass presentationRenderPass() const;

    Camera camera;
    Scene scene;

    ImGuiContext *ctx = nullptr;

    Device &device() const;

    VkSampler defaultSampler() const;

    void addPass(RendererPass *pass);

    BaseRenderer *renderer() const;

    void freeResources() const;

    QImage grab(std::vector<DrawObjectInstance> &models, std::vector<VfxObjectInstance> &vfx);

private:
    void updateCamera(Camera &camera) const;
    void initBlitPipeline();
    void destroyBlitPipeline() const;

    std::array<VkCommandBuffer, 3> m_commandBuffers{};

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> m_framebuffers;

    ImGuiPass *m_imGuiPass = nullptr;
    std::unique_ptr<Device> m_device;
    BaseRenderer *m_renderer = nullptr;
    FileCache &m_cache;
    std::vector<RendererPass *> m_passes;
    VfxPass *m_vfxPass = nullptr;
};
