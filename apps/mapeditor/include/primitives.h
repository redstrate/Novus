// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vulkan/vulkan.h>

#include "buffer.h"

class RenderManager;

struct Sphere {
    Buffer vertexBuffer, indexBuffer;
    uint32_t indexCount;
};

class Primitives
{
public:
    static void Initialize(RenderManager *renderer);
    static void Cleanup(RenderManager *renderer);

    static void DrawSphere(VkCommandBuffer commandBuffer);

    static Sphere sphere;
};
