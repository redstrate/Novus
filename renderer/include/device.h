// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <string_view>

#include <vulkan/vulkan.h>

#include "buffer.h"
#include "physis.hpp"
#include "texture.h"

class SwapChain;

class Device
{
public:
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT callback = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE, presentQueue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    SwapChain *swapChain = nullptr;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    Buffer createBuffer(size_t size, VkBufferUsageFlags usageFlags) const;
    void copyToBuffer(const Buffer &buffer, const void *data, size_t size) const;
    void destroyBuffer(Buffer &buffer) const;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    VkShaderModule createShaderModule(const uint32_t *code, int length) const;

    VkShaderModule loadShaderFromDisk(std::string_view path) const;

    Texture createTexture(int width, int height, VkFormat format, VkImageUsageFlags usage) const;
    void destroyTexture(Texture &texture) const;

    Texture createDummyTexture(std::array<uint8_t, 4> values = {255, 255, 255, 255}) const;
    Buffer createDummyBuffer() const;

    VkCommandBuffer beginSingleTimeCommands() const;

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    static void inlineTransitionImageLayout(VkCommandBuffer commandBuffer,
                                            VkImage image,
                                            VkFormat format,
                                            VkImageAspectFlags aspect,
                                            const VkImageSubresourceRange &range,
                                            VkImageLayout oldLayout,
                                            VkImageLayout newLayout,
                                            VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                            VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    static void transitionTexture(VkCommandBuffer commandBuffer, const Texture &texture, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkResult nameObject(VkObjectType type, uint64_t object, std::string_view name) const;
    void nameTexture(Texture &texture, std::string_view name) const;
    void nameBuffer(Buffer &buffer, std::string_view name) const;

    Texture addGameTexture(physis_Texture gameTexture) const;

    void beginDebugMarker(VkCommandBuffer command_buffer, const VkDebugUtilsLabelEXT &marker_info) const;
    void endDebugMarker(VkCommandBuffer command_buffer) const;
    void insertDebugLabel(VkCommandBuffer command_buffer, const VkDebugUtilsLabelEXT &label_info) const;

    void waitForIdle() const;
};
