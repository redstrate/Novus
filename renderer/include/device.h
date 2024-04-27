// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>
#include <vector>

#include <vulkan/vulkan.h>

#include "buffer.h"
#include "texture.h"

class SwapChain;

class Device
{
public:
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE, presentQueue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    SwapChain *swapChain = nullptr;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    Buffer createBuffer(size_t size, VkBufferUsageFlags usageFlags);
    void copyToBuffer(Buffer &buffer, void *data, size_t size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkShaderModule createShaderModule(const uint32_t *code, int length);

    VkShaderModule loadShaderFromDisk(std::string_view path);

    Texture createTexture(int width, int height, VkFormat format, VkImageUsageFlags usage);

    Texture createDummyTexture();
    Buffer createDummyBuffer();

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer pT);

    void inlineTransitionImageLayout(VkCommandBuffer commandBuffer,
                                     VkImage image,
                                     VkFormat format,
                                     VkImageAspectFlags aspect,
                                     VkImageSubresourceRange range,
                                     VkImageLayout oldLayout,
                                     VkImageLayout newLayout,
                                     VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                     VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void transitionTexture(VkCommandBuffer commandBuffer, Texture &texture, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkResult nameObject(VkObjectType type, uint64_t object, std::string_view name);
    void nameTexture(Texture &texture, std::string_view name);
};