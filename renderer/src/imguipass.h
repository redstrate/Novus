// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <vulkan/vulkan.h>

class Renderer;
struct RenderTarget;

class ImGuiPass
{
public:
    explicit ImGuiPass(Renderer &renderer);
    ~ImGuiPass();

    void render(VkCommandBuffer commandBuffer);

private:
    void createDescriptorSetLayout();
    void createPipeline();
    void createFontImage();
    void createBuffer(VkBuffer &buffer, VkDeviceMemory &memory, VkDeviceSize size, VkBufferUsageFlagBits bufferUsage);

    VkDescriptorSetLayout setLayout_ = nullptr;

    VkPipelineLayout pipelineLayout_ = nullptr;
    VkPipeline pipeline_ = nullptr;

    VkImage fontImage_ = nullptr;
    VkDeviceMemory fontMemory_ = nullptr;
    VkImageView fontImageView_ = nullptr;
    VkSampler fontSampler_ = nullptr;

    VkBuffer vertexBuffer = VK_NULL_HANDLE, indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE, indexMemory = VK_NULL_HANDLE;
    size_t vertexSize = 0, indexSize = 0;

    std::map<VkImageView, VkDescriptorSet> descriptorSets_ = {};

    Renderer &renderer_;
};
