// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "drawobject.h"
#include "pass.h"
#include "texture.h"

#include <QHash>
#include <QString>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class Scene;
struct physis_Layer;
class ObjectScene;
class RenderManager;
class Device;
class SceneState;

class ObjectPass : public RendererPass
{
public:
    ObjectPass(RenderManager *renderer, SceneState *appState);
    ~ObjectPass() override;

    void render(VkCommandBuffer commandBuffer, Camera &camera, const Scene &scene, const std::vector<DrawObjectInstance> &models) override;

private:
    void createPipeline();
    void createBillboardPipeline();
    void addScene(VkCommandBuffer commandBuffer, Camera &camera, const ObjectScene &scene);
    void addLayer(VkCommandBuffer commandBuffer, const Camera &camera, const physis_Layer &layer);
    void drawBillboard(VkCommandBuffer commandBuffer, const Camera &camera, const Texture &texture, glm::vec4 color, glm::vec3 position);
    Texture addTexture(const QString &path);

    VkPipeline m_pipeline = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;

    VkPipeline m_billboardPipeline = nullptr;
    VkPipelineLayout m_billboardPipelineLayout = nullptr;
    VkDescriptorSetLayout m_setLayout = nullptr;

    RenderManager *m_renderer = nullptr;
    Device &m_device;
    SceneState *m_appState = nullptr;
    Texture m_spotLightTexture;
    Texture m_pointLightTexture;
    Texture m_planeLightTexture;
    Texture m_directionalLightTexture;
    Texture m_lineLightTexture;
    Texture m_chairTexture;
    VkSampler m_sampler = nullptr;

    QHash<VkImage, VkDescriptorSet> m_cachedBillboardTextures;
};
