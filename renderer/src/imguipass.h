// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "buffer.h"
#include "texture.h"

#include <map>
#include <vulkan/vulkan.h>

class RenderManager;

class ImGuiPass
{
public:
    explicit ImGuiPass(RenderManager &renderer);
    ~ImGuiPass();

    void render(VkCommandBuffer commandBuffer);

private:
    void createDescriptorSetLayout();
    void createPipeline();
    void createFontImage();

    VkDescriptorSetLayout setLayout_ = nullptr;

    VkPipelineLayout pipelineLayout_ = nullptr;
    VkPipeline pipeline_ = nullptr;

    Texture fontAtlas;

    Buffer vertexBuffer, indexBuffer;

    std::map<VkImageView, VkDescriptorSet> descriptorSets_ = {};

    RenderManager &renderer_;
};
