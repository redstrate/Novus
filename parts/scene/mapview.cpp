// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mapview.h"

#include <QThreadPool>
#include <QVBoxLayout>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "filecache.h"
#include "objectpass.h"
#include "scenestate.h"

MapView::MapView(physis_SqPackResource *data, FileCache &cache, SceneState *appState, QWidget *parent)
    : QWidget(parent)
    , m_data(data)
    , m_cache(cache)
    , m_appState(appState)
{
    mdlPart = new MDLPart(data, cache);
    mdlPart->enableFreemode();
    connect(mdlPart, &MDLPart::initializeRender, this, [this, appState] {
        mdlPart->manager()->addPass(new ObjectPass(mdlPart->manager(), appState));
    });

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mdlPart);
    setLayout(layout);

    connect(appState, &SceneState::mapLoaded, this, &MapView::reloadMap);

    // TODO: be more efficient about what we reload
    connect(appState, &SceneState::visibleLayerIdsChanged, this, &MapView::reloadMap);
    connect(appState, &SceneState::visibleTerrainPlatesChanged, this, &MapView::reloadMap);
}

MDLPart &MapView::part() const
{
    return *mdlPart;
}

void MapView::centerOn(const glm::vec3 position)
{
    mdlPart->position = position;
    Q_EMIT mdlPart->cameraMoved();
}

void MapView::addTerrain(QString basePath, physis_Terrain terrain)
{
    for (int i = 0; i < terrain.num_plates; i++) {
        if (!m_appState->visibleTerrainPlates.contains(i)) {
            continue;
        }

        QString base2Path = basePath.left(basePath.lastIndexOf(QStringLiteral("/level/")));
        QString mdlPath = QStringLiteral("%1/bgplate/%2").arg(base2Path, QString::fromStdString(terrain.plates[i].filename));
        std::string mdlPathStd = mdlPath.toStdString();

        auto plateMdlFile = physis_sqpack_read(m_data, mdlPathStd.c_str());
        auto plateMdl = physis_mdl_parse(m_data->platform, plateMdlFile);
        if (plateMdl.p_ptr != nullptr) {
            std::vector<std::pair<std::string, physis_Material>> materials;
            for (uint32_t j = 0; j < plateMdl.num_material_names; j++) {
                const char *material_name = plateMdl.material_names[j];

                const auto matFile = m_cache.lookupFile(QLatin1String(material_name));
                if (matFile.size > 0) {
                    auto mat = physis_material_parse(m_data->platform, matFile);
                    materials.push_back(std::make_pair(std::string{material_name}, mat));
                } else {
                    qWarning() << "Failed to find terrain material" << material_name;
                }
            }

            Transformation transformation{
                .translation = {terrain.plates[i].position[0], 0.0f, terrain.plates[i].position[1]},
                .rotation = {0, 0, 0},
                .scale = {1, 1, 1},
            };

            mdlPart->addModel(plateMdl, false, transformation, QStringLiteral("terapart%1").arg(i), materials, 0);

            // We don't need this, and it will just take up memory
            physis_mdl_free(&plateMdl);

            physis_free_file(&plateMdlFile);
        } else {
            qWarning() << "Failed to load plate mdl" << mdlPath;
        }
    }
}

void MapView::reloadMap()
{
    mdlPart->clear();

    Transformation transformation{};
    transformation.scale[0] = 1;
    transformation.scale[1] = 1;
    transformation.scale[2] = 1;

    processScene(m_appState->rootScene, transformation);
}

glm::mat4 transformToMat4(const Transformation &transformation)
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(transformation.translation[0], transformation.translation[1], transformation.translation[2]));
    m *= glm::mat4_cast(glm::quat(glm::vec3(transformation.rotation[0], transformation.rotation[1], transformation.rotation[2])));
    m = glm::scale(m, glm::vec3(transformation.scale[0], transformation.scale[1], transformation.scale[2]));

    return m;
}

Transformation fromMat4(const glm::mat4 &m)
{
    glm::vec3 translation{};
    glm::quat rotation{};
    glm::vec3 scale{};
    glm::vec3 skew{};
    glm::vec4 perspective{};
    glm::decompose(m, scale, rotation, translation, skew, perspective);

    auto eulerAngles = glm::eulerAngles(rotation);

    return Transformation{.translation = {translation[0], translation[1], translation[2]},
                          .rotation =
                              {
                                  eulerAngles[0],
                                  eulerAngles[1],
                                  eulerAngles[2],
                              },
                          .scale = {
                              scale[0],
                              scale[1],
                              scale[2],
                          }};
}

Transformation addTransformation(const Transformation &a, const Transformation &b)
{
    // NOTE: I know this is stupid, but I plan on replacing this whole system eventually.

    const auto aMat = transformToMat4(a);
    const auto bMat = transformToMat4(b);

    return fromMat4(aMat * bMat);
}

void MapView::processScene(ObjectScene &scene, const Transformation &rootTransformation)
{
    scene.combinedTransformation = addTransformation(rootTransformation, scene.transformation);

    addTerrain(scene.basePath, scene.terrain);

    for (const auto &layerGroup : scene.embeddedLgbs) {
        for (uint32_t j = 0; j < layerGroup.layer_count; j++) {
            const auto layer = layerGroup.layers[j];
            if (!scene.isSgb && !m_appState->visibleLayerIds.contains(layer.id)) {
                continue;
            }

            processLayer(layer, scene.combinedTransformation);
        }
    }
    for (const auto &[name, lgb] : scene.lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            const auto chunk = lgb.chunks[i];
            for (uint32_t j = 0; j < chunk.num_layers; j++) {
                const auto layer = chunk.layers[j];
                if (!scene.isSgb && !m_appState->visibleLayerIds.contains(layer.id)) {
                    continue;
                }

                processLayer(layer, scene.combinedTransformation);
            }
        }
    }

    for (auto &nestedScene : scene.nestedScenes.values()) {
        processScene(nestedScene, scene.combinedTransformation);
    }
}

void MapView::processLayer(const physis_Layer &layer, const Transformation &rootTransformation)
{
    for (uint32_t z = 0; z < layer.num_objects; z++) {
        const auto object = layer.objects[z];

        const auto combinedTransform = addTransformation(rootTransformation, object.transform);

        switch (object.data.tag) {
        case physis_LayerEntry::Tag::BG: {
            std::string assetPath = object.data.bg._0.asset_path;
            if (!assetPath.empty()) {
                if (!mdlPart->modelExists(QString::fromStdString(assetPath))) {
                    auto plateMdlFile = physis_sqpack_read(m_data, assetPath.c_str());
                    if (plateMdlFile.size == 0) {
                        continue;
                    }

                    auto plateMdl = physis_mdl_parse(m_data->platform, plateMdlFile);
                    if (plateMdl.p_ptr != nullptr) {
                        std::vector<std::pair<std::string, physis_Material>> materials;
                        for (uint32_t j = 0; j < plateMdl.num_material_names; j++) {
                            const char *material_name = plateMdl.material_names[j];

                            const auto matFile = m_cache.lookupFile(QLatin1String(material_name));
                            if (matFile.size > 0) {
                                auto mat = physis_material_parse(m_data->platform, matFile);
                                materials.push_back(std::make_pair(material_name, mat));
                            } else {
                                qWarning() << "Failed to find model material" << material_name;
                            }
                        }

                        mdlPart->addModel(plateMdl, false, combinedTransform, QString::fromStdString(assetPath), materials, 0);

                        // We don't need this, and it will just take up memory
                        physis_mdl_free(&plateMdl);
                    } else {
                        qWarning() << "Failed to load" << assetPath;
                    }

                    physis_free_file(&plateMdlFile);
                } else {
                    mdlPart->addExistingModel(QString::fromStdString(assetPath), combinedTransform);
                }
            }
        } break;
        default:
            break;
        }
    }
}

#include "moc_mapview.cpp"
