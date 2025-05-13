#pragma once

#include "pass.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class RenderManager;
class Device;

class ObjectPass : public RendererPass
{
public:
    ObjectPass(RenderManager *renderer);

    void render(VkCommandBuffer commandBuffer, Camera &camera) override;

private:
    void createPipeline();

    VkPipeline pipeline_ = nullptr;
    VkPipelineLayout pipelineLayout_ = nullptr;

    RenderManager *m_renderer;
    Device &m_device;
};
