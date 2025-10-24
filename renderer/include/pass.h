// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vulkan/vulkan.h>

class Camera;

class RendererPass
{
public:
    virtual ~RendererPass() = default;

    virtual void render(VkCommandBuffer commandBuffer, Camera &camera) = 0;
};
