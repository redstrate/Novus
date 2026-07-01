// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mapview.h"

#include <QThreadPool>
#include <QVBoxLayout>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui.h>

#include "filecache.h"
#include "frustum.h"
#include "objectpass.h"
#include "scenepart.h"
#include "scenestate.h"
#include "swapchain.h"
#include "utility.h"

#include <glm/gtc/type_ptr.hpp>

MapView::MapView(FileCache &cache, SceneState *appState, QWidget *parent)
    : QWidget(parent)
    , m_cache(cache)
    , m_appState(appState)
{
    m_mdlPart = new MDLPart(m_cache);
    m_mdlPart->enableFreemode();
    connect(m_mdlPart, &MDLPart::initializeRender, this, [this, appState] {
        m_mdlPart->manager()->addPass(new ObjectPass(m_mdlPart->manager(), appState));
    });
    connect(m_mdlPart, &MDLPart::cameraMoved, this, [this] {
        updateLightCulling();
    });
    m_mdlPart->requestUpdate = [this] {
        const auto drawNameplate = [this](uint32_t id, const glm::vec3 position, const QString &name) {
            const auto distance = glm::distance(m_mdlPart->manager()->camera.position, position);
            if (distance > MAX_DEBUG_DRAW_DISTANCE) {
                return;
            }

            const auto toObject = glm::normalize(m_mdlPart->manager()->camera.position - position);
            const auto cameraForward = glm::normalize(glm::vec3(glm::inverse(m_mdlPart->manager()->camera.view)[2]));

            if (glm::dot(toObject, cameraForward) < 0.0) {
                return;
            }

            glm::vec4 pos = m_mdlPart->manager()->camera.perspective * m_mdlPart->manager()->camera.view * glm::vec4(position, 1);
            pos.x /= pos.w;
            pos.y /= pos.w;
            pos.z /= pos.w;

            const glm::vec2 screenSpacePos = {
                ((pos.x + 1.0f) * 0.5f) * m_mdlPart->manager()->device().swapChain->extent.width,
                ((pos.y + 1.0f) * 0.5f) * m_mdlPart->manager()->device().swapChain->extent.height,
            };

            if (screenSpacePos.x <= 0 || screenSpacePos.x >= m_mdlPart->manager()->device().swapChain->extent.width || screenSpacePos.y <= 0
                || screenSpacePos.y >= m_mdlPart->manager()->device().swapChain->extent.height) {
                return;
            }

            // Flipped because the viewport is also flipped
            ImGui::SetNextWindowBgAlpha(0.5f);
            ImGui::PushID(id);
            if (ImGui::Begin(std::to_string(id).c_str(),
                             nullptr,
                             ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize
                                 | ImGuiWindowFlags_NoSavedSettings)) {
                ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - screenSpacePos.x, screenSpacePos.y));
                ImGui::Text("%s", name.toStdString().c_str());
            }
            ImGui::End();
            ImGui::PopID();
        };

        const auto walkScene = [this, &drawNameplate](ObjectScene &scene) {
            for (const auto &[_, lgb] : scene.lgbFiles) {
                for (uint32_t i = 0; i < lgb.chunks->num_layers; i++) {
                    if (!scene.isSgb() && !m_appState->visibleLayerIds.contains(lgb.chunks->layers[i].id)) {
                        continue;
                    }

                    for (uint32_t j = 0; j < lgb.chunks->layers[i].num_objects; j++) {
                        const auto &object = lgb.chunks->layers[i].objects[j];
                        switch (object.data.tag) {
                        case physis_LayerEntry::Tag::EventObject: {
                            const auto eobjName = object.data.event_object._0.parent_data.base_id;
                            if (object.data.event_object._0.bound_instance_id != 0) {
                                drawNameplate(object.data.event_object._0.bound_instance_id,
                                              glm::make_vec3(scene.locateGameObject(object.data.event_object._0.bound_instance_id).translation),
                                              m_appState->lookupEObjName(eobjName) + QStringLiteral(" (Linked)"));
                            }
                            drawNameplate(object.instance_id, glm::make_vec3(object.transform.translation), m_appState->lookupEObjName(eobjName));
                        } break;
                        case physis_LayerEntry::Tag::EventNPC: {
                            const auto enpcName = object.data.event_npc._0.parent_data.parent_data.base_id;
                            drawNameplate(object.instance_id, glm::make_vec3(object.transform.translation), m_appState->lookupENpcName(enpcName));
                        } break;
                        default:
                            break;
                        }
                    }
                }
            }

            // TOOD: embedded lgbs
        };

        walkScene(m_appState->rootScene);
    };

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mdlPart);
    setLayout(layout);

    connect(appState, &SceneState::mapLoaded, this, &MapView::reloadMap);

    // TODO: be more efficient about what we reload
    connect(appState, &SceneState::visibleLayerIdsChanged, this, &MapView::reloadMap);
    connect(appState, &SceneState::visibleTerrainPlatesChanged, this, &MapView::reloadMap);
}

MDLPart &MapView::part() const
{
    return *m_mdlPart;
}

void MapView::centerOn(const glm::vec3 position)
{
    m_mdlPart->position = position;
    Q_EMIT m_mdlPart->cameraMoved();
}

void MapView::clear()
{
    m_mdlPart->clear();
}

void MapView::addTerrain(ObjectScene &scene)
{
    for (int i = 0; i < scene.terrain.num_plates; i++) {
        if (!m_appState->visibleTerrainPlates.contains(i)) {
            continue;
        }

        QString base2Path = scene.basePath.left(scene.basePath.lastIndexOf(QStringLiteral("/level/")));
        QString mdlPath = QStringLiteral("%1/bgplate/%2").arg(base2Path, QString::fromStdString(scene.terrain.plates[i].filename));

        auto plateMdlFile = m_cache.read(mdlPath);
        auto plateMdl = physis_mdl_parse(m_cache.platform(), plateMdlFile);
        if (plateMdl.p_ptr != nullptr) {
            std::vector<std::pair<std::string, physis_Material>> materials;
            for (uint32_t j = 0; j < plateMdl.num_material_names; j++) {
                const char *material_name = plateMdl.material_names[j];

                if (!scene.cachedMaterials.contains(material_name)) {
                    const auto matFile = m_cache.read(QLatin1String(material_name));
                    if (matFile.size > 0) {
                        auto mat = physis_material_parse(m_cache.platform(), matFile);
                        scene.cachedMaterials[material_name] = mat;
                    } else {
                        qWarning() << "Failed to find terrain material" << material_name;
                    }
                }

                materials.push_back(std::make_pair(std::string{material_name}, scene.cachedMaterials[material_name]));
            }

            Transformation transformation{
                .translation = {scene.terrain.plates[i].position[0], 0.0f, scene.terrain.plates[i].position[1]},
                .rotation = {0, 0, 0},
                .scale = {1, 1, 1},
            };

            m_mdlPart->addModel(plateMdl, false, transformation, QStringLiteral("terapart%1").arg(i), materials);

            // We don't need this, and it will just take up memory
            physis_mdl_free(&plateMdl);
        } else {
            qWarning() << "Failed to load plate mdl" << mdlPath;
        }
    }
}

void MapView::reloadMap()
{
    m_mdlPart->clear();

    Transformation transformation{};
    transformation.scale[0] = 1;
    transformation.scale[1] = 1;
    transformation.scale[2] = 1;

    processScene(m_appState->rootScene, transformation);
}

void MapView::processScene(ObjectScene &scene, const Transformation &rootTransformation)
{
    scene.combinedTransformation = addTransformation(rootTransformation, scene.transformation);

    addTerrain(scene);

    for (const auto &layerGroup : scene.embeddedLgbs) {
        for (uint32_t j = 0; j < layerGroup.layer_count; j++) {
            const auto layer = layerGroup.layers[j];
            if (!scene.isSgb() && !m_appState->visibleLayerIds.contains(layer.id)) {
                continue;
            }

            processLayer(scene, layer, scene.combinedTransformation);
        }
    }
    for (const auto &[name, lgb] : scene.lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            const auto chunk = lgb.chunks[i];
            for (uint32_t j = 0; j < chunk.num_layers; j++) {
                const auto layer = chunk.layers[j];
                if (!scene.isSgb() && !m_appState->visibleLayerIds.contains(layer.id)) {
                    continue;
                }

                processLayer(scene, layer, scene.combinedTransformation);
            }
        }
    }

    for (auto &[_, nestedScene] : scene.nestedScenes) {
        if (!scene.isSgb() && m_appState->visibleLayerIds.contains(nestedScene.originatingSgbLayerId)) {
            processScene(nestedScene, scene.combinedTransformation);
        }
    }
}

void MapView::processLayer(ObjectScene &scene, const physis_Layer &layer, const Transformation &rootTransformation)
{
    for (uint32_t z = 0; z < layer.object_set_referenced_count; z++) {
        if (layer.object_set_referenced[z].asset_type == LayerEntryType::Light) {
            const auto buffer = m_cache.read(QString::fromStdString(layer.object_set_referenced[z].obsb_path));
            const auto obsb = physis_obsb_parse(m_cache.platform(), buffer);
            for (uint32_t i = 0; i < obsb.envs_count; i++) {
                // TODO: pick weather
                int section = 0;

                for (uint32_t j = 0; j < obsb.envs[i].sections[section].timeline_count; j++) {
                    const auto &timeline = obsb.envs[i].sections[section].timelines[j];
                    if (timeline.tag == physis_EnvTimelineElement::Tag::ChangeVisibility) {
                        OBSBTimeline newTimeline;

                        for (uint32_t g = 0; g < timeline.change_visibility.point_count; g++) {
                            newTimeline.points.push_back(
                                OBSBPoint{.time = timeline.change_visibility.points[g].time, .visible = timeline.change_visibility.points[g].visible});
                        }

                        scene.obsbTimelines[layer.object_set_referenced[z].instance_id] = newTimeline;
                    }
                }
            }
            break;
        }
    }

    for (uint32_t z = 0; z < layer.num_objects; z++) {
        const auto object = layer.objects[z];

        const auto combinedTransform = addTransformation(rootTransformation, object.transform);

        switch (object.data.tag) {
        case physis_LayerEntry::Tag::BG: {
            std::string assetPath = object.data.bg._0.asset_path;
            if (!assetPath.empty()) {
                if (!m_mdlPart->modelExists(QString::fromStdString(assetPath))) {
                    auto plateMdlFile = m_cache.read(QString::fromStdString(assetPath));
                    if (plateMdlFile.size == 0) {
                        continue;
                    }

                    auto plateMdl = physis_mdl_parse(m_cache.platform(), plateMdlFile);
                    if (plateMdl.p_ptr != nullptr) {
                        std::vector<std::pair<std::string, physis_Material>> materials;
                        for (uint32_t j = 0; j < plateMdl.num_material_names; j++) {
                            const char *material_name = plateMdl.material_names[j];

                            if (!scene.cachedMaterials.contains(material_name)) {
                                const auto matFile = m_cache.read(QLatin1String(material_name));
                                if (matFile.size > 0) {
                                    auto mat = physis_material_parse(m_cache.platform(), matFile);
                                    scene.cachedMaterials[material_name] = mat;
                                } else {
                                    qWarning() << "Failed to find model material" << material_name;
                                }
                            }

                            materials.push_back(std::make_pair(material_name, scene.cachedMaterials[material_name]));
                        }

                        m_mdlPart->addModel(plateMdl, false, combinedTransform, QString::fromStdString(assetPath), materials);

                        // We don't need this, and it will just take up memory
                        physis_mdl_free(&plateMdl);
                    } else {
                        qWarning() << "Failed to load" << assetPath;
                    }
                } else {
                    m_mdlPart->addExistingModel(QString::fromStdString(assetPath), combinedTransform);
                }
            }
        } break;
        case physis_LayerEntry::Tag::LayLight: {
            SceneLight sceneLight;
            sceneLight.id = object.instance_id;
            sceneLight.parentSgbId = scene.originatingSgbId;
            sceneLight.position = glm::make_vec3(combinedTransform.translation);
            sceneLight.type = object.data.lay_light._0.light_type;
            sceneLight.color = glm::vec3(static_cast<float>(object.data.lay_light._0.diffuse_color_hdri.red) / 255.0f,
                                         static_cast<float>(object.data.lay_light._0.diffuse_color_hdri.green) / 255.0f,
                                         static_cast<float>(object.data.lay_light._0.diffuse_color_hdri.blue) / 255.0f);
            sceneLight.intensity = object.data.lay_light._0.diffuse_color_hdri.intensity;

            m_mdlPart->addLight(sceneLight);
        } break;
        case physis_LayerEntry::Tag::Vfx: {
            std::string assetPath = object.data.vfx._0.asset_path;
            if (!assetPath.empty()) {
                if (!m_mdlPart->vfxExists(QString::fromStdString(assetPath))) {
                    auto avfxFile = m_cache.read(QString::fromStdString(assetPath));
                    if (avfxFile.size == 0) {
                        qWarning() << "Could not find" << assetPath;
                        continue;
                    }

                    auto avfx = physis_avfx_parse(m_cache.platform(), avfxFile);
                    if (avfx.model_count > 0) {
                        m_mdlPart->addVfx(avfx, combinedTransform, QString::fromStdString(assetPath));
                    } else {
                        qWarning() << "Failed to load" << assetPath;
                    }
                } else {
                    m_mdlPart->addExistingVfx(QString::fromStdString(assetPath), combinedTransform);
                }
            }
        } break;
        default:
            break;
        }
    }
}

void MapView::updateLightCulling()
{
    m_mdlPart->manager()->scene.culledLights = 0;

    for (auto &light : m_mdlPart->manager()->scene.lights) {
        // Update based on culling
        if (light.id != 0) {
            if (auto bounding_box = m_appState->checkLightBoundingBox(light.id, light.parentSgbId)) {
                bounding_box->min[0] += light.position[0];
                bounding_box->min[1] += light.position[1];
                bounding_box->min[2] += light.position[2];
                bounding_box->max[0] += light.position[0];
                bounding_box->max[1] += light.position[1];
                bounding_box->max[2] += light.position[2];
                if (m_mdlPart->manager()->scene.frustumCulling && !test_aabb_frustum(m_mdlPart->manager()->camera.frustum(), *bounding_box)) {
                    light.active = false;
                    m_mdlPart->manager()->scene.culledLights++;
                    break;
                }
                light.active = true;
            }
        } else {
            light.active = true;
        }

        // Update based on OBSB
        // TODO: check other things than root scene?
        if (m_appState->rootScene.obsbTimelines.contains(light.id)) {
            light.active = m_appState->rootScene.obsbTimelines[light.id].shouldBeVisible(m_mdlPart->manager()->scene.time);
        }
    }
}

#include "moc_mapview.cpp"
