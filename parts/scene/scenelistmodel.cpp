// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenelistmodel.h"
#include "scenestate.h"

#include <KLocalizedString>
#include <QIcon>

SceneListModel::SceneListModel(SceneState *appState, QObject *parent)
    : QAbstractItemModel(parent)
    , m_appState(appState)
{
    connect(m_appState, &SceneState::mapLoaded, this, &SceneListModel::refresh);
}

int SceneListModel::rowCount(const QModelIndex &parent) const
{
    SceneTreeInformation const *parentItem;

    if (!parent.isValid())
        parentItem = &m_rootItem;
    else
        parentItem = static_cast<SceneTreeInformation *>(parent.internalPointer());

    return static_cast<int>(parentItem->children.size());
}

int SceneListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QModelIndex SceneListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    SceneTreeInformation const *parentItem;

    if (!parent.isValid())
        parentItem = &m_rootItem;
    else
        parentItem = static_cast<SceneTreeInformation *>(parent.internalPointer());

    SceneTreeInformation *childItem = parentItem->children[row];
    if (childItem)
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex SceneListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto childItem = static_cast<SceneTreeInformation *>(index.internalPointer());
    SceneTreeInformation *parentItem = childItem->parent;

    if (parentItem == &m_rootItem)
        return {};

    return createIndex(parentItem->row, 0, parentItem);
}

QVariant SceneListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (role == SceneListRoles::ObjectIdRole) {
        return item->id;
    }
    if (index.column() == 0) {
        if (role == Qt::DisplayRole) {
            return item->name;
        }
        if (role == Qt::DecorationRole) {
            switch (item->type) {
            case TreeType::Root:
                Q_UNREACHABLE();
            case TreeType::File:
                return QIcon::fromTheme(QStringLiteral("emblem-documents-symbolic"));
            case TreeType::Layer:
                return QIcon::fromTheme(QStringLiteral("dialog-layers-symbolic"));
            case TreeType::Object:
                return QIcon::fromTheme(QStringLiteral("draw-cuboid-symbolic"));
            case TreeType::Plate:
                return QIcon::fromTheme(QStringLiteral("kstars_xplanet-symbolic"));
            case TreeType::Timeline:
            case TreeType::Action:
                return QIcon::fromTheme(QStringLiteral("preferences-desktop-animations"));
            }
        }
    } else if (index.column() == 1) {
        if (role == Qt::CheckStateRole) {
            if (item->type == TreeType::Layer) {
                return m_appState->visibleLayerIds.contains(item->id) ? Qt::Checked : Qt::Unchecked;
            }
            if (item->type == TreeType::Plate) {
                return m_appState->visibleTerrainPlates.contains(item->id) ? Qt::Checked : Qt::Unchecked;
            }
        }
    }

    return {};
}

QVariant SceneListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return i18nc("@title:column Object name", "Name");
        } else if (section == 1) {
            return i18nc("@title:column If the layer is visible", "Visible");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags SceneListModel::flags(const QModelIndex &index) const
{
    auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (index.column() == 1 && (item->type == TreeType::Layer || item->type == TreeType::Plate))
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    return QAbstractItemModel::flags(index);
}

bool SceneListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (index.column() == 1) {
        if (item->type == TreeType::Layer) {
            if (value.value<Qt::CheckState>() == Qt::Checked) {
                m_appState->visibleLayerIds.push_back(item->id);
            } else {
                m_appState->visibleLayerIds.removeAll(item->id);
            }
            Q_EMIT m_appState->visibleLayerIdsChanged();
        } else if (item->type == TreeType::Plate) {
            if (value.value<Qt::CheckState>() == Qt::Checked) {
                m_appState->visibleTerrainPlates.push_back(item->id);
            } else {
                m_appState->visibleTerrainPlates.removeAll(item->id);
            }
            Q_EMIT m_appState->visibleTerrainPlatesChanged();
        }
    }
    return QAbstractItemModel::setData(index, value, role);
}

QHash<int, QByteArray> SceneListModel::roleNames() const
{
    return {
        {ObjectIdRole, "objectId"},
    };
}

std::optional<physis_InstanceObject const *> SceneListModel::objectAt(const QModelIndex &index) const
{
    const auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (item && item->type == TreeType::Object) {
        return static_cast<physis_InstanceObject const *>(item->data);
    }
    return std::nullopt;
}

std::optional<physis_Layer const *> SceneListModel::layerAt(const QModelIndex &index) const
{
    const auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (item && item->type == TreeType::Layer) {
        return static_cast<physis_Layer const *>(item->data);
    }
    return std::nullopt;
}

std::optional<physis_ScnTimeline const *> SceneListModel::timelineAt(const QModelIndex &index) const
{
    const auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (item && item->type == TreeType::Timeline) {
        return static_cast<physis_ScnTimeline const *>(item->data);
    }
    return std::nullopt;
}

std::optional<ScnSGActionControllerDescriptor const *> SceneListModel::actionAt(const QModelIndex &index) const
{
    const auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (item && item->type == TreeType::Action) {
        return static_cast<ScnSGActionControllerDescriptor const *>(item->data);
    }
    return std::nullopt;
}

void SceneListModel::refresh()
{
    beginResetModel();

    // Reset the root item
    m_rootItem.children.clear();

    processScene(&m_rootItem, m_appState->rootScene);

    endResetModel();
}

void SceneListModel::addLayer(uint32_t index, SceneTreeInformation *fileItem, const physis_Layer &layer, ObjectScene &scene)
{
    auto layerItem = new SceneTreeInformation();
    layerItem->type = TreeType::Layer;
    layerItem->parent = fileItem;
    layerItem->name = QString::fromLatin1(layer.name);
    layerItem->row = index;
    layerItem->id = layer.id;
    layerItem->data = &layer;
    fileItem->children.push_back(layerItem);

    for (uint32_t z = 0; z < layer.num_objects; z++) {
        const auto &object = layer.objects[z];

        QString objectName = i18n("Unknown Object");
        switch (object.data.tag) {
        case physis_LayerEntry::Tag::BG:
            objectName = i18n("BG Model");
            break;
        case physis_LayerEntry::Tag::LayLight:
            objectName = i18n("Light");
            break;
        case physis_LayerEntry::Tag::Vfx:
            objectName = i18n("Vfx");
            break;
        case physis_LayerEntry::Tag::EventObject:
            // Give the EObj an actual name.
            objectName = m_appState->lookupEObjName(object.data.event_object._0.parent_data.base_id);
            break;
        case physis_LayerEntry::Tag::PopRange:
            objectName = i18n("Pop Range");
            break;
        case physis_LayerEntry::Tag::EventNPC:
            // Give the ENPC an actual name.
            objectName = m_appState->lookupENpcName(object.data.event_npc._0.parent_data.parent_data.base_id);
            break;
        case physis_LayerEntry::Tag::MapRange:
            objectName = i18n("Map Range");
            break;
        case physis_LayerEntry::Tag::SharedGroup:
            objectName = i18n("Shared Group");
            break;
        case physis_LayerEntry::Tag::Aetheryte:
            objectName = i18n("Aetheryte");
            break;
        case physis_LayerEntry::Tag::ExitRange:
            objectName = i18n("Exit Range");
            break;
        case physis_LayerEntry::Tag::EventRange:
            objectName = i18n("Event Range");
            break;
        case physis_LayerEntry::Tag::ChairMarker:
            objectName = i18n("Chair Marker");
            break;
        case physis_LayerEntry::Tag::PrefetchRange:
            objectName = i18n("Prefetch Range");
            break;
        default:
            break;
        }

        auto objectItem = new SceneTreeInformation();
        objectItem->type = TreeType::Object;
        objectItem->parent = layerItem;
        objectItem->name = i18n("%1 (%2)", objectName, QString::number(object.instance_id)); // TODO: do display names if we have them
        objectItem->row = z;
        objectItem->data = &object;
        objectItem->id = object.instance_id;
        layerItem->children.push_back(objectItem);

        // Load nested shared group data
        if (scene.nestedScenes.contains(object.instance_id)) {
            auto &nestedScene = scene.nestedScenes[object.instance_id];
            processScene(objectItem, nestedScene);
        }
    }
}

void SceneListModel::processScene(SceneTreeInformation *parentNode, ObjectScene &scene)
{
    if (scene.terrain.num_plates > 0) {
        auto terrainItem = new SceneTreeInformation();
        terrainItem->type = TreeType::File;
        terrainItem->parent = parentNode;
        terrainItem->name = i18n("Terrain");
        terrainItem->row = parentNode->children.size();
        parentNode->children.push_back(terrainItem);

        // Add terrain plates
        for (int i = 0; i < scene.terrain.num_plates; i++) {
            auto layerItem = new SceneTreeInformation();
            layerItem->type = TreeType::Plate;
            layerItem->parent = terrainItem;
            layerItem->name = i18n("Plate %1").arg(i);
            layerItem->row = i;
            layerItem->id = i;
            terrainItem->children.push_back(layerItem);
        }
    }

    if (scene.embeddedTimelines.length() > 0) {
        auto timelinesItem = new SceneTreeInformation();
        timelinesItem->type = TreeType::File;
        timelinesItem->parent = parentNode;
        timelinesItem->name = i18n("Timelines");
        timelinesItem->row = parentNode->children.size();
        parentNode->children.push_back(timelinesItem);

        for (uint32_t i = 0; i < scene.embeddedTimelines.size(); i++) {
            auto timelineItem = new SceneTreeInformation();
            timelineItem->type = TreeType::Timeline;
            timelineItem->parent = timelinesItem;
            timelineItem->name = i18n("Timeline");
            timelineItem->row = i;
            timelineItem->data = &scene.embeddedTimelines[i];
            timelinesItem->children.push_back(timelineItem);
        }
    }

    if (scene.actionDescriptors.size() > 0) {
        auto actionsItem = new SceneTreeInformation();
        actionsItem->type = TreeType::File;
        actionsItem->parent = parentNode;
        actionsItem->name = i18n("Actions");
        actionsItem->row = parentNode->children.size();
        parentNode->children.push_back(actionsItem);

        for (uint32_t i = 0; i < scene.actionDescriptors.size(); i++) {
            auto actionItem = new SceneTreeInformation();
            actionItem->type = TreeType::Action;
            actionItem->parent = actionsItem;
            actionItem->name = i18n("Action");
            actionItem->row = i;
            actionItem->data = &scene.actionDescriptors[i];
            actionsItem->children.push_back(actionItem);
        }
    }

    // External LGB files
    for (size_t y = 0; y < scene.lgbFiles.size(); y++) {
        const auto &[name, lgb] = scene.lgbFiles[y];

        auto fileItem = new SceneTreeInformation();
        fileItem->type = TreeType::File;
        fileItem->parent = parentNode;
        fileItem->name = name;
        fileItem->row = parentNode->children.size();
        parentNode->children.push_back(fileItem);

        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            const auto &chunk = lgb.chunks[i];
            for (uint32_t j = 0; j < chunk.num_layers; j++) {
                addLayer(j, fileItem, chunk.layers[j], scene);
            }
        }
    }

    // Embeded LGBs
    for (size_t y = 0; y < scene.embeddedLgbs.size(); y++) {
        const auto &lgb = scene.embeddedLgbs[y];

        auto fileItem = new SceneTreeInformation();
        fileItem->type = TreeType::File;
        fileItem->parent = parentNode;
        fileItem->name = QString::fromLatin1(lgb.name);
        fileItem->row = parentNode->children.size();
        parentNode->children.push_back(fileItem);

        for (uint32_t i = 0; i < lgb.layer_count; i++) {
            addLayer(i, fileItem, lgb.layers[i], scene);
        }
    }
}

// "moc_scenelistmodel.cpp"
