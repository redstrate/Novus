// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vulkan/vulkan.h>

class Texture
{
public:
    VkFormat format;
    VkImageSubresourceRange range;
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;
};