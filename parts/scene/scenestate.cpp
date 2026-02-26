// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenestate.h"

#include <KLocalizedString>

#include "animation.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

using namespace Qt::StringLiterals;

SceneState::SceneState(physis_SqPackResource *resource, QObject *parent)
    : QObject(parent)
{
    // ENPC
    {
        const auto exhFile = physis_sqpack_read(resource, "exd/enpcresident.exh");
        if (exhFile.size == 0) {
            qWarning() << "Failed to read exd/epncresident.exh";
        } else {
            const auto exh = physis_exh_parse(resource->platform, exhFile);
            if (!exh.p_ptr) {
                qWarning() << "Failed to parse exd/enpcresident.exh";
            } else {
                m_enpcResidentSheet = physis_sqpack_read_excel_sheet(resource, "ENpcResident", &exh, Language::English);
            }
        }
    }

    // EOBJ
    {
        const auto exhFile = physis_sqpack_read(resource, "exd/eobjname.exh");
        if (exhFile.size == 0) {
            qWarning() << "Failed to read exd/eobjname.exh";
        } else {
            const auto exh = physis_exh_parse(resource->platform, exhFile);
            if (!exh.p_ptr) {
                qWarning() << "Failed to parse exd/eobjname.exh";
            } else {
                m_eobjNameSheet = physis_sqpack_read_excel_sheet(resource, "EObjName", &exh, Language::English);
            }
        }
    }
}

void ObjectScene::clear()
{
    basePath.clear();
    lgbFiles.clear();
    terrain = {};
    embeddedLgbs.clear();
}

void ObjectScene::load(physis_SqPackResource *data, const physis_ScnSection &section)
{
    basePath = QString::fromLatin1(section.general.bg_path);

    QString bgPath = QStringLiteral("%1/bgplate/").arg(section.general.bg_path);
    std::string bgPathStd = bgPath.toStdString() + "terrain.tera";

    auto tera_buffer = physis_sqpack_read(data, bgPathStd.c_str());
    if (tera_buffer.size > 0) {
        terrain = physis_terrain_parse(data->platform, tera_buffer);
        terrainPath = QString::fromStdString(bgPathStd);
    } else {
        qWarning() << "Failed to load terrain" << bgPathStd;
    }

    const auto loadLgb = [this, data](const char *path) {
        const auto bg_buffer = physis_sqpack_read(data, path);
        if (bg_buffer.size > 0) {
            const auto lgb = physis_lgb_parse(data->platform, bg_buffer);
            if (lgb.num_chunks > 0) {
                lgbFiles.emplace_back(QString::fromLatin1(path), lgb);
            }
        }
    };

    for (uint32_t i = 0; i < section.num_lgb_paths; i++) {
        loadLgb(section.lgb_paths[i]);
    }

    for (uint32_t i = 0; i < section.num_layer_groups; i++) {
        embeddedLgbs.push_back(section.layer_groups[i]);
    }

    for (uint32_t i = 0; i < section.timelines.timeline_count; i++) {
        embeddedTimelines.push_back(section.timelines.timelines[i]);
    }

    for (uint32_t i = 0; i < section.action_descriptors.descriptor_count; i++) {
        actionDescriptors.push_back(section.action_descriptors.descriptors[i]);
    }

    // Process nested shared groups
    for (const auto &[name, lgb] : lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                auto layer = lgb.chunks[i].layers[j];
                for (uint32_t h = 0; h < layer.num_objects; h++) {
                    if (layer.objects[h].data.tag == physis_LayerEntry::Tag::SharedGroup) {
                        processSharedGroup(data, layer.objects[h].instance_id, layer.objects[h].transform, layer.objects[h].data.shared_group._0.asset_path);
                    }
                }
            }
        }
    }

    for (const auto layerGroup : embeddedLgbs) {
        processScnLayerGroup(data, layerGroup);
    }

    animation = new Animation(*this);
}

void SceneState::load(physis_SqPackResource *data, const physis_ScnSection &section)
{
    rootScene.load(data, section);

    // Load terrain and bg by default
    for (int i = 0; i < rootScene.terrain.num_plates; i++) {
        visibleTerrainPlates.push_back(i);
    }

    for (const auto &[name, lgb] : rootScene.lgbFiles) {
        if (name.endsWith(QStringLiteral("bg.lgb"))) {
            for (uint32_t i = 0; i < lgb.num_chunks; i++) {
                for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                    // Skip festival-specific layers
                    if (lgb.chunks[i].layers[j].festival_id == 0) {
                        visibleLayerIds.push_back(lgb.chunks[i].layers[j].id);
                    }
                }
            }
            break;
        }
    }

    processLongestAnimationTime(rootScene);

    Q_EMIT mapLoaded();
}

void SceneState::loadDropIn(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        QJsonDocument document = QJsonDocument::fromJson(f.readAll());
        auto obj = document.object();
        if (obj.contains("appends"_L1)) {
            for (const auto &name : rootScene.lgbFiles | std::views::keys) {
                if (name == obj["appends"_L1]) {
                    DropIn dropIn{};
                    dropIn.appends = name;

                    for (const auto &jsonLayer : obj["layers"_L1].toArray()) {
                        const auto &jsonLayerObj = jsonLayer.toObject();

                        DropInLayer layer;
                        layer.name = jsonLayerObj["name"_L1].toString();

                        for (const auto &jsonObj : jsonLayerObj["objects"_L1].toArray()) {
                            const auto &jsonObjObj = jsonObj.toObject();

                            DropInObject obj{};
                            obj.instanceId = jsonObjObj["instance_id"_L1].toInteger();
                            obj.position[0] = jsonObjObj["position"_L1]["x"_L1].toDouble();
                            obj.position[1] = jsonObjObj["position"_L1]["y"_L1].toDouble();
                            obj.position[2] = jsonObjObj["position"_L1]["z"_L1].toDouble();
                            obj.rotation = jsonObjObj["rotation"_L1].toDouble();

                            const auto &jsonData = jsonObjObj["data"_L1].toObject();
                            const auto &jsonType = jsonData["type"_L1].toString();
                            if (jsonType == "gathering_point"_L1) {
                                DropInGatheringPoint gatheringPoint{};
                                gatheringPoint.baseId = jsonData["base_id"_L1].toInteger();

                                obj.data = gatheringPoint;
                            } else if (jsonType == "battle_npc"_L1) {
                                DropInBattleNpc battleNpc{};
                                battleNpc.baseId = jsonData["base_id"_L1].toInteger();
                                battleNpc.nameId = jsonData["name_id"_L1].toInteger();
                                battleNpc.hp = jsonData["hp"_L1].toInteger();
                                battleNpc.level = jsonData["level"_L1].toInt();

                                obj.data = battleNpc;
                            } else {
                                qWarning() << "Unknown drop-in object type:" << jsonType;
                            }

                            layer.objects.push_back(obj);
                        }

                        dropIn.layers.push_back(layer);
                    }

                    rootScene.dropIns.push_back({path, dropIn});
                }
            }
        }
    }
}

void SceneState::saveDropIns()
{
    for (const auto &[path, dropIn] : rootScene.dropIns) {
        QJsonObject dropInObj;
        dropInObj["appends"_L1] = dropIn.appends;

        QJsonArray layerArray;
        for (const auto &layer : dropIn.layers) {
            QJsonObject layerObj;
            layerObj["name"_L1] = layer.name;

            QJsonArray objArray;
            for (const auto &object : layer.objects) {
                QJsonObject objObj;
                objObj["instance_id"_L1] = (qint64)object.instanceId;
                objObj["position"_L1] = QJsonObject{
                    {"x"_L1, object.position[0]},
                    {"y"_L1, object.position[1]},
                    {"z"_L1, object.position[2]},
                };
                objObj["rotation"_L1] = object.rotation;

                if (const auto data = std::get_if<DropInGatheringPoint>(&object.data)) {
                    objObj["data"_L1] = QJsonObject{
                        {"type"_L1, "gathering_point"_L1},
                        {"base_id"_L1, (qint64)data->baseId},
                    };
                } else if (const auto data = std::get_if<DropInBattleNpc>(&object.data)) {
                    objObj["data"_L1] = QJsonObject{
                        {"type"_L1, "battle_npc"_L1},
                        {"base_id"_L1, (qint64)data->baseId},
                        {"name_id"_L1, (qint64)data->nameId},
                        {"hp"_L1, (qint64)data->hp},
                        {"level"_L1, data->level},
                    };
                }

                objArray.push_back(objObj);
            }

            layerObj["objects"_L1] = objArray;

            layerArray.push_back(layerObj);
        }

        dropInObj["layers"_L1] = layerArray;

        QJsonDocument document(dropInObj);
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(document.toJson());
        }
    }
}

void SceneState::clear()
{
    rootScene.clear();
    visibleLayerIds.clear();
    selectedObject.reset();
    selectedLayer.reset();
    visibleTerrainPlates.clear();
}

void SceneState::showAll()
{
    showAllInScene(rootScene);

    Q_EMIT visibleLayerIdsChanged();
}

QString SceneState::lookupENpcName(const uint32_t id) const
{
    auto row = physis_excel_get_row(&m_enpcResidentSheet, id);
    if (row.columns && strlen(row.columns[0].string._0) > 0) {
        return QString::fromLatin1(row.columns[0].string._0);
    }
    return i18n("Event NPC");
}

QString SceneState::lookupEObjName(const uint32_t id) const
{
    auto row = physis_excel_get_row(&m_eobjNameSheet, id);
    if (row.columns && strlen(row.columns[0].string._0) > 0) {
        return QString::fromLatin1(row.columns[0].string._0);
    }
    return i18n("Event Object");
}

float SceneState::longestAnimationTime() const
{
    return m_longestAnimationTime;
}

void SceneState::updateAllAnimations(const float time)
{
    processUpdateAnimation(rootScene, time);
}

void SceneState::processLongestAnimationTime(const ObjectScene &scene)
{
    m_longestAnimationTime = std::max(m_longestAnimationTime, scene.animation->duration());

    for (const auto &nestedScene : scene.nestedScenes.values()) {
        processLongestAnimationTime(nestedScene);
    }
}

void SceneState::processUpdateAnimation(ObjectScene &scene, float time)
{
    scene.animation->update(scene, time);

    for (auto &nestedScene : scene.nestedScenes.values()) {
        processUpdateAnimation(nestedScene, time);
    }
}

void SceneState::showAllInScene(const ObjectScene &scene)
{
    for (const auto &[_, lgb] : scene.lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                visibleLayerIds.push_back(lgb.chunks[i].layers[j].id);
            }
        }
    }

    for (const auto &layerGroup : scene.embeddedLgbs) {
        for (uint32_t i = 0; i < layerGroup.layer_count; i++) {
            visibleLayerIds.push_back(layerGroup.layers[i].id);
        }
    }

    for (const auto &nestedScene : scene.nestedScenes) {
        showAllInScene(nestedScene);
    }
}

Transformation ObjectScene::locateGameObject(const uint32_t instanceId) const
{
    // TODO: support everything else within ObjectScene, nested transforms etc

    for (const auto &[_, lgb] : lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                for (uint32_t z = 0; z < lgb.chunks[i].layers[j].num_objects; z++) {
                    if (lgb.chunks[i].layers[j].objects[z].instance_id == instanceId) {
                        return lgb.chunks[i].layers[j].objects[z].transform;
                    }
                }
            }
        }
    }

    qWarning() << "Failed to locate game object" << instanceId;

    return {};
}

Transformation ObjectScene::locateGameObjectByBaseId(const uint32_t baseId) const
{
    // TODO: support everything else within ObjectScene, nested transforms etc

    for (const auto &[_, lgb] : lgbFiles) {
        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            for (uint32_t j = 0; j < lgb.chunks[i].num_layers; j++) {
                for (uint32_t z = 0; z < lgb.chunks[i].layers[j].num_objects; z++) {
                    if (lgb.chunks[i].layers[j].objects[z].data.tag != physis_LayerEntry::Tag::EventObject) {
                        continue;
                    }

                    if (lgb.chunks[i].layers[j].objects[z].data.event_object._0.parent_data.base_id == baseId) {
                        return lgb.chunks[i].layers[j].objects[z].transform;
                    }
                }
            }
        }
    }

    qWarning() << "Failed to locate game object by base id" << baseId;

    return {};
}

void ObjectScene::processSharedGroup(physis_SqPackResource *data, uint32_t instanceId, const Transformation &transformation, const char *path)
{
    qInfo() << "Processing" << path;

    const auto sgbFile = physis_sqpack_read(data, path);
    if (sgbFile.size == 0) {
        qWarning() << "Failed to find" << path;
        return;
    }

    const auto sgb = physis_sgb_parse(data->platform, sgbFile);
    if (!sgb.sections) {
        qWarning() << "Failed to parse" << path;
        return;
    }

    // TODO: load more than one section?
    nestedScenes[instanceId].load(data, sgb.sections[0]);
    nestedScenes[instanceId].transformation = transformation;
    nestedScenes[instanceId].isSgb = true;
}

void ObjectScene::processScnLayerGroup(physis_SqPackResource *data, const physis_ScnLayerGroup &group)
{
    for (uint32_t j = 0; j < group.layer_count; j++) {
        const auto layer = group.layers[j];
        for (uint32_t h = 0; h < layer.num_objects; h++) {
            if (layer.objects[h].data.tag == physis_LayerEntry::Tag::SharedGroup) {
                processSharedGroup(data, layer.objects[h].instance_id, layer.objects[h].transform, layer.objects[h].data.shared_group._0.asset_path);
            }
        }
    }
}
