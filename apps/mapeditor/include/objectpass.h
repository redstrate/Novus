#pragma once

#include "pass.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class RenderManager;
class Device;
class AppState;

class ObjectPass : public RendererPass
{
public:
    ObjectPass(RenderManager *renderer, AppState *appState);

    void render(VkCommandBuffer commandBuffer, Camera &camera) override;

private:
    void createPipeline();

    VkPipeline m_pipeline = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;

    RenderManager *m_renderer;
    Device &m_device;
    AppState *m_appState;
};
