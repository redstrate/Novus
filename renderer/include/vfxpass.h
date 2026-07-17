// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "vfxobject.h"

#include <QHash>
#include <vector>
#include <vulkan/vulkan.h>

class Device;
struct Camera;
class RenderManager;

class VfxPass
{
public:
    explicit VfxPass(RenderManager &manager);

    void render(VkCommandBuffer commandBuffer, const Camera &camera, const std::vector<VfxObjectInstance> &vfx);

private:
    void createPipeline();
    void addTexture(const Texture &texture);

    Device &m_device;
    RenderManager &m_renderer;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_setLayout = nullptr;

    QHash<VkImage, VkDescriptorSet> m_cachedTextures;
};
