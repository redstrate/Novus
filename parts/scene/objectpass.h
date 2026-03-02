// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "pass.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct physis_Layer;
class ObjectScene;
class RenderManager;
class Device;
class SceneState;

class ObjectPass : public RendererPass
{
public:
    ObjectPass(RenderManager *renderer, SceneState *appState);

    void render(VkCommandBuffer commandBuffer, Camera &camera) override;

private:
    void createPipeline();
    void addScene(VkCommandBuffer commandBuffer, Camera &camera, const ObjectScene &scene);
    void addLayer(VkCommandBuffer commandBuffer, const Camera &camera, const physis_Layer &layer);

    VkPipeline m_pipeline = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;

    RenderManager *m_renderer;
    Device &m_device;
    SceneState *m_appState;
};
