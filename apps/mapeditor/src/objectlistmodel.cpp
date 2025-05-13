// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectlistmodel.h"
#include "appstate.h"

#include <KLocalizedString>

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
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeInformation *>(parent.internalPointer());

    return static_cast<int>(parentItem->children.size());
}

int ObjectListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
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

    if (item->type == TreeType::File) {
        if (role == Qt::DisplayRole) {
            return item->name;
        }
    } else if (item->type == TreeType::Object) {
        if (role == Qt::DisplayRole) {
            return item->name;
        }
    }

    return {};
}

QVariant ObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return i18nc("@title:column Object id", "Id");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

void ObjectListModel::refresh()
{
    beginResetModel();

    auto fileItem = new TreeInformation();
    fileItem->type = TreeType::File;
    fileItem->parent = m_rootItem;
    fileItem->name = QStringLiteral("bg");
    m_rootItem->children.push_back(fileItem);

    for (int i = 0; i < m_appState->bgGroup.num_chunks; i++) {
        const auto chunk = m_appState->bgGroup.chunks[i];
        for (int j = 0; j < chunk.num_layers; j++) {
            const auto layer = chunk.layers[j];
            for (int z = 0; z < layer.num_objects; z++) {
                const auto object = layer.objects[z];
                const QString name = QString::fromLatin1(object.name);

                auto objectItem = new TreeInformation();
                objectItem->type = TreeType::Object;
                objectItem->parent = fileItem;
                objectItem->name = i18n("Unknown (%1)", object.instance_id); // TODO: do display names if we have them
                objectItem->row = z;
                fileItem->children.push_back(objectItem);
            }
        }
    }
    endResetModel();
}

#include "moc_objectlistmodel.cpp"
