// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mapview.h"

#include <QThreadPool>
#include <QVBoxLayout>

#include "appstate.h"
#include "filecache.h"
#include "objectpass.h"

MapView::MapView(physis_SqPackResource *data, FileCache &cache, AppState *appState, QWidget *parent)
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

    connect(appState, &AppState::mapLoaded, this, &MapView::reloadMap);
    connect(appState, &AppState::visibleLayerIdsChanged, this, &MapView::reloadMap);
}

MDLPart &MapView::part() const
{
    return *mdlPart;
}

void MapView::centerOn(const glm::vec3 position)
{
    mdlPart->position = position;
}

void MapView::addTerrain(QString basePath, physis_Terrain terrain)
{
    for (int i = 0; i < terrain.num_plates; i++) {
        QString mdlPath = QStringLiteral("%1%2").arg(basePath, QString::fromStdString(terrain.plates[i].filename));
        std::string mdlPathStd = mdlPath.toStdString();

        auto plateMdlFile = physis_sqpack_read(m_data, mdlPathStd.c_str());
        auto plateMdl = physis_mdl_parse(m_data->platform, plateMdlFile);
        if (plateMdl.p_ptr != nullptr) {
            std::vector<physis_Material> materials;
            for (uint32_t j = 0; j < plateMdl.num_material_names; j++) {
                const char *material_name = plateMdl.material_names[j];

                const auto matFile = m_cache.lookupFile(QLatin1String(material_name));
                if (matFile.size > 0) {
                    auto mat = physis_material_parse(m_data->platform, matFile);
                    materials.push_back(mat);
                }
            }

            mdlPart->addModel(plateMdl,
                              false,
                              glm::vec3(terrain.plates[i].position[0], 0.0f, terrain.plates[i].position[1]),
                              QStringLiteral("terapart%1").arg(i),
                              materials,
                              0);

            // We don't need this, and it will just take up memory
            physis_mdl_free(&plateMdl);

            physis_free_file(&plateMdlFile);
        }
    }
}

void MapView::reloadMap()
{
    mdlPart->clear();

    QString base2Path = m_appState->basePath.left(m_appState->basePath.lastIndexOf(QStringLiteral("/level/")));
    QString bgPath = QStringLiteral("bg/%1/bgplate/").arg(base2Path);

    std::string bgPathStd = bgPath.toStdString() + "terrain.tera";

    auto tera_buffer = physis_sqpack_read(m_data, bgPathStd.c_str());
    if (tera_buffer.size > 0) {
        auto tera = physis_terrain_parse(m_data->platform, tera_buffer);
        addTerrain(bgPath, tera);
    } else {
        qWarning() << "Failed to load" << bgPathStd;
    }

    // add bg models
    for (const auto &[name, lgb] : m_appState->lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            const auto chunk = lgb.chunks[i];
            for (uint32_t j = 0; j < chunk.num_layers; j++) {
                const auto layer = chunk.layers[j];
                if (!m_appState->visibleLayerIds.contains(layer.id)) {
                    continue;
                }

                for (uint32_t z = 0; z < layer.num_objects; z++) {
                    const auto object = layer.objects[z];

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
                                    std::vector<physis_Material> materials;
                                    for (uint32_t j = 0; j < plateMdl.num_material_names; j++) {
                                        const char *material_name = plateMdl.material_names[j];

                                        const auto matFile = m_cache.lookupFile(QLatin1String(material_name));
                                        if (matFile.size > 0) {
                                            auto mat = physis_material_parse(m_data->platform, matFile);
                                            materials.push_back(mat);
                                        }
                                    }

                                    mdlPart->addModel(
                                        plateMdl,
                                        false,
                                        glm::vec3(object.transform.translation[0], object.transform.translation[1], object.transform.translation[2]),
                                        QString::fromStdString(assetPath),
                                        materials,
                                        0);

                                    // We don't need this, and it will just take up memory
                                    physis_mdl_free(&plateMdl);
                                } else {
                                    qWarning() << "Failed to load" << assetPath;
                                }

                                physis_free_file(&plateMdlFile);
                            } else {
                                mdlPart->addExistingModel(
                                    QString::fromStdString(assetPath),
                                    glm::vec3(object.transform.translation[0], object.transform.translation[1], object.transform.translation[2]));
                            }
                        }
                    } break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}

#include "moc_mapview.cpp"
