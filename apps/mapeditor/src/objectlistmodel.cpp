// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectlistmodel.h"
#include "appstate.h"

#include <KLocalizedString>
#include <QIcon>

ObjectListModel::ObjectListModel(AppState *appState, QObject *parent)
    : QAbstractItemModel(parent)
    , m_appState(appState)
{
    m_rootItem = new TreeInformation();
    m_rootItem->type = TreeType::Root;

    connect(m_appState, &AppState::mapLoaded, this, &ObjectListModel::refresh);
}

int ObjectListModel::rowCount(const QModelIndex &parent) const
{
    TreeInformation *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeInformation *>(parent.internalPointer());

    return static_cast<int>(parentItem->children.size());
}

int ObjectListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QModelIndex ObjectListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    TreeInformation *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeInformation *>(parent.internalPointer());

    TreeInformation *childItem = parentItem->children[row];
    if (childItem)
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex ObjectListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto childItem = static_cast<TreeInformation *>(index.internalPointer());
    TreeInformation *parentItem = childItem->parent;

    if (parentItem == m_rootItem)
        return {};

    return createIndex(parentItem->row, 0, parentItem);
}

QVariant ObjectListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto item = static_cast<TreeInformation *>(index.internalPointer());
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
            }
        }
    } else if (index.column() == 1) {
        if (role == Qt::CheckStateRole && item->type == TreeType::Layer) {
            return m_appState->visibleLayerIds.contains(item->id) ? Qt::Checked : Qt::Unchecked;
        }
    }

    return {};
}

QVariant ObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags ObjectListModel::flags(const QModelIndex &index) const
{
    auto item = static_cast<TreeInformation *>(index.internalPointer());
    if (index.column() == 1 && item->type == TreeType::Layer)
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    return QAbstractItemModel::flags(index);
}

bool ObjectListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto item = static_cast<TreeInformation *>(index.internalPointer());
    if (index.column() == 1 && item->type == TreeType::Layer) {
        if (value.value<Qt::CheckState>() == Qt::Checked) {
            m_appState->visibleLayerIds.push_back(item->id);
        } else {
            m_appState->visibleLayerIds.removeAll(item->id);
        }
        Q_EMIT m_appState->visibleLayerIdsChanged();
    }
    return QAbstractItemModel::setData(index, value, role);
}

std::optional<physis_InstanceObject const *> ObjectListModel::objectId(const QModelIndex &index) const
{
    const auto item = static_cast<TreeInformation *>(index.internalPointer());
    if (item && item->type == TreeType::Object) {
        return static_cast<physis_InstanceObject const *>(item->data);
    }
    return std::nullopt;
}

void ObjectListModel::refresh()
{
    beginResetModel();

    for (size_t y = 0; y < m_appState->lgbFiles.size(); y++) {
        const auto &[name, lgb] = m_appState->lgbFiles[y];

        auto fileItem = new TreeInformation();
        fileItem->type = TreeType::File;
        fileItem->parent = m_rootItem;
        fileItem->name = name;
        fileItem->row = y;
        m_rootItem->children.push_back(fileItem);

        for (uint32_t i = 0; i < lgb.num_chunks; i++) {
            const auto &chunk = lgb.chunks[i];
            for (uint32_t j = 0; j < chunk.num_layers; j++) {
                const auto &layer = chunk.layers[j];

                auto layerItem = new TreeInformation();
                layerItem->type = TreeType::Layer;
                layerItem->parent = fileItem;
                layerItem->name = QString::fromLatin1(layer.name);
                layerItem->row = j;
                layerItem->id = layer.id;
                fileItem->children.push_back(layerItem);

                for (uint32_t z = 0; z < layer.num_objects; z++) {
                    const auto &object = layer.objects[z];

                    QString objectName = i18n("Unknown Object");
                    switch (object.data.tag) {
                    case physis_LayerEntry::Tag::BG:
                        objectName = i18n("BG Model");
                        break;
                    case physis_LayerEntry::Tag::EventObject:
                        objectName = i18n("Event Object");
                        break;
                    case physis_LayerEntry::Tag::PopRange:
                        objectName = i18n("Pop Range");
                        break;
                    case physis_LayerEntry::Tag::EventNPC:
                        objectName = i18n("Event NPC");
                        break;
                    default:
                        break;
                    }

                    auto objectItem = new TreeInformation();
                    objectItem->type = TreeType::Object;
                    objectItem->parent = layerItem;
                    objectItem->name = i18n("%1 (%2)", objectName, QString::number(object.instance_id)); // TODO: do display names if we have them
                    objectItem->row = z;
                    objectItem->data = &object;
                    layerItem->children.push_back(objectItem);
                }
            }
        }
    }
    endResetModel();
}

#include "moc_objectlistmodel.cpp"
