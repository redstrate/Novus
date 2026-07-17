// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vulkan/vulkan.h>

#include "buffer.h"

class RenderManager;

struct Sphere {
    Buffer vertexBuffer, indexBuffer;
    uint32_t indexCount = 0;
};

struct Cylinder {
    Buffer vertexBuffer;
    uint32_t vertexCount = 0;
};

class Primitives
{
public:
    static void Initialize(const RenderManager *renderer);
    static void Cleanup(const RenderManager *renderer);

    static void DrawSphere(VkCommandBuffer commandBuffer);
    static void DrawCube(VkCommandBuffer commandBuffer);
    static void DrawCylinder(VkCommandBuffer commandBuffer);
    static void DrawPlane(VkCommandBuffer commandBuffer);

    static Sphere sphere;
    static Sphere cube;
    static Cylinder cylinder;
    static Cylinder plane;
};
