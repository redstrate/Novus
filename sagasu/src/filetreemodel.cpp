// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetreemodel.h"
#include "physis.hpp"

#include <QtConcurrent>

FileTreeModel::FileTreeModel(GameData *data)
    : gameData(data)
    , QAbstractItemModel()
{
    rootItem = new TreeInformation();
    rootItem->type = TreeType::Root;

    for (auto knownFolder : m_database.getKnownFolders()) {
        addKnownFolder(knownFolder);
    }

    auto indexEntries = physis_index_parse("/home/josh/.local/share/astra/game/{1973dab7-aa23-4af1-8a48-bfec78dd6c8e}/game/sqpack/ffxiv/000000.win32.index");
    for (int i = 0; i < indexEntries.num_entries; i++) {
        if (knownDirHashes.contains(indexEntries.dir_entries[i])) {
            QString name;
            if (m_database.knowsFile(indexEntries.filename_entries[i])) {
                name = m_database.getFilename(indexEntries.filename_entries[i]);
            }
            addFile(knownDirHashes[indexEntries.dir_entries[i]], indexEntries.filename_entries[i], name);
        } else {
            addFolder(rootItem, indexEntries.dir_entries[i]);
        }
    }
}

int FileTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeInformation *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeInformation *>(parent.internalPointer());

    return parentItem->children.size();
}

int FileTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    TreeInformation *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeInformation *>(parent.internalPointer());

    TreeInformation *childItem = parentItem->children[row];
    if (childItem)
        return createIndex(row, column, childItem);

    return {};
}

QModelIndex FileTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto childItem = static_cast<TreeInformation *>(index.internalPointer());
    TreeInformation *parentItem = childItem->parent;

    if (parentItem == rootItem)
        return {};

    return createIndex(parentItem->row, 0, parentItem);
}

QVariant FileTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto item = static_cast<TreeInformation *>(index.internalPointer());
    if (role == Qt::UserRole) {
        // build the full path
        QString path;
        TreeInformation *parent = item;
        while (parent != rootItem) {
            if (path.isEmpty()) {
                path = parent->name;
            } else {
                path = parent->name + QStringLiteral("/") + path;
            }
            parent = parent->parent;
        }

        return path;
    } else if (role == Qt::DisplayRole) {
        if (item->type == TreeType::Folder) {
            if (item->name.isEmpty()) {
                return QStringLiteral("Unknown Folder (%1)").arg(item->hash);
            } else {
                return item->name;
            }
        } else if (item->type == TreeType::File) {
            if (item->name.isEmpty()) {
                return QStringLiteral("Unknown File (%1)").arg(item->hash);
            } else {
                return item->name;
            }
        }
    }

    return {};
}

QVariant FileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return QStringLiteral("Name");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

void FileTreeModel::addKnownFolder(QString string)
{
    auto children = string.split(QStringLiteral("/"));

    QString conct = children[0];
    TreeInformation *parentItem = rootItem;
    for (int i = 0; i < children.size(); i++) {
        if (i > 0) {
            conct += QStringLiteral("/") + children[i];
        }
        std::string conctStd = conct.toStdString();
        auto folderItem = new TreeInformation();
        folderItem->name = children[i];
        folderItem->type = TreeType::Folder;
        folderItem->parent = parentItem;
        folderItem->row = i + 1;
        folderItem->hash = physis_generate_partial_hash(conctStd.c_str());
        parentItem->children.push_back(folderItem);
        parentItem = folderItem;
        knownDirHashes[folderItem->hash] = folderItem;
    }
}

void FileTreeModel::addFile(TreeInformation *parentItem, uint32_t name, QString realName)
{
    auto fileItem = new TreeInformation();
    fileItem->hash = name;
    fileItem->name = realName;
    fileItem->type = TreeType::File;
    fileItem->parent = parentItem;

    parentItem->children.push_back(fileItem);
}

void FileTreeModel::addFolder(TreeInformation *parentItem, uint32_t name)
{
    auto fileItem = new TreeInformation();
    fileItem->hash = name;
    fileItem->type = TreeType::Folder;
    fileItem->parent = parentItem;

    parentItem->children.push_back(fileItem);
}

#include "moc_filetreemodel.cpp"