// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "buffer.h"
#include "texture.h"

#include <map>
#include <vector>
#include <vulkan/vulkan.h>

class RenderManager;

class ImGuiPass
{
public:
    explicit ImGuiPass(RenderManager &renderer);
    ~ImGuiPass();

    void render(uint32_t imageIndex, VkCommandBuffer commandBuffer);

private:
    void createDescriptorSetLayout();
    void createPipeline();
    void createFontImage();

    VkDescriptorSetLayout m_setLayout = nullptr;

    VkPipelineLayout m_pipelineLayout = nullptr;
    VkPipeline m_pipeline = nullptr;

    Texture m_fontAtlas;

    std::vector<Buffer> m_vertexBuffers, m_indexBuffers;

    std::map<VkImageView, VkDescriptorSet> m_descriptorSets = {};

    RenderManager &m_renderer;
};
