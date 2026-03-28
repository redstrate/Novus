// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vulkan/vulkan.h>

class Texture
{
public:
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkImageSubresourceRange range{};
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
};
