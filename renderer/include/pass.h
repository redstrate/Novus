#pragma once

#include <vulkan/vulkan.h>

class Camera;

class RendererPass
{
public:
    virtual void render(VkCommandBuffer commandBuffer, Camera &camera) = 0;
};