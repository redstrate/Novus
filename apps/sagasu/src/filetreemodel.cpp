// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetreemodel.h"
#include "filetypes.h"
#include "physis.hpp"

#include <KLocalizedString>
#include <QIcon>
#include <QtConcurrent>

Q_DECLARE_METATYPE(Hash)

FileTreeModel::FileTreeModel(HashDatabase &database, bool showUnknown, const QString &gamePath, SqPackResource *data, QObject *parent)
    : QAbstractItemModel(parent)
    , gameData(data)
    , m_database(database)
    , m_showUnknown(showUnknown)
{
    rootItem = new TreeInformation();
    rootItem->type = TreeType::Root;

    for (const auto &knownFolder : m_database.getKnownFolders()) {
        addKnownFolder(knownFolder);
    }

    QDirIterator it(QStringLiteral("%1/sqpack").arg(gamePath), QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        if (info.exists() && (info.completeSuffix() == QStringLiteral("win32.index"))) {
            std::string pathStd = info.filePath().toStdString();
            const auto indexEntries = physis_index_parse(pathStd.c_str());
            for (uint32_t i = 0; i < indexEntries.num_hashes; i++) {
                const auto hash = indexEntries.hashes[i];
                switch (hash.tag) {
                case Hash::Tag::SplitPath: {
                    const auto completeHash =
                        static_cast<uint32_t>(static_cast<uint64_t>(hash.split_path.path) << 32 | static_cast<uint64_t>(hash.split_path.name));
                    if (knownDirHashes.contains(hash.split_path.path)) {
                        QString name;
                        if (m_database.knowsFile(completeHash)) {
                            name = m_database.getFilename(completeHash);
                        }
                        addFile(knownDirHashes[hash.split_path.path], completeHash, name, hash, info.filePath());
                    } else {
                        addFolder(rootItem, hash.split_path.path);
                    }
                } break;
                case Hash::Tag::FullPath:
                    Q_UNREACHABLE(); // shouldn't be in basic index files
                    break;
                }
            }
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
    Q_UNUSED(parent)
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

    Q_UNREACHABLE();
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

    return createIndex(parentItem->row, index.column(), parentItem);
}

QVariant FileTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto item = static_cast<TreeInformation *>(index.internalPointer());
    if (role == PathRole) {
        if (item->type != TreeType::File || item->name.isEmpty()) {
            return {};
        }

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
    } else if (role == IsUnknownRole) {
        return item->name.isEmpty(); // unknown files/folders have no name (obviously, we don't know what its named!)
    } else if (role == IsFolderRole) {
        return item->type == TreeType::Folder;
    } else if (role == HashRole) {
        return QVariant::fromValue(item->originalHash);
    } else if (role == IndexPathRole) {
        return item->indexPath;
    } else if (role == Qt::DisplayRole) {
        if (item->type == TreeType::Folder) {
            if (item->name.isEmpty()) {
                return i18n("Unknown Folder (%1)").arg(item->hash);
            } else {
                return item->name;
            }
        } else if (item->type == TreeType::File) {
            if (item->name.isEmpty()) {
                return i18n("Unknown File (%1)").arg(item->hash);
            } else {
                return item->name;
            }
        }
    } else if (role == Qt::DecorationRole) {
        if (item->type == TreeType::Folder) {
            return QIcon::fromTheme(QStringLiteral("folder-symbolic"));
        } else if (item->type == TreeType::File) {
            QFileInfo info(item->name);
            const FileType type = FileTypes::getFileType(info.completeSuffix());

            return QIcon::fromTheme(FileTypes::getFiletypeIcon(type));
        }
    }

    return {};
}

QVariant FileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return i18nc("@title:column", "Name");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

void FileTreeModel::addKnownFolder(const QString &string)
{
    auto children = string.split(QStringLiteral("/"));

    QString conct = children[0];
    TreeInformation *parentItem = rootItem;
    for (int i = 0; i < children.size(); i++) {
        if (i > 0) {
            conct += QStringLiteral("/") + children[i];
        }
        std::string conctStd = conct.toStdString();
        auto hash = physis_generate_partial_hash(conctStd.c_str());

        if (knownDirHashes.contains(hash)) {
            parentItem = knownDirHashes[hash];
        } else {
            auto folderItem = new TreeInformation();
            folderItem->name = children[i];
            folderItem->type = TreeType::Folder;
            folderItem->parent = parentItem;
            folderItem->row = parentItem->children.size();
            folderItem->hash = hash;
            parentItem->children.push_back(folderItem);
            parentItem = folderItem;
            knownDirHashes[folderItem->hash] = folderItem;
        }
    }
}

void FileTreeModel::addFile(TreeInformation *parentItem, uint32_t name, const QString &realName, Hash originalHash, const QString &indexPath)
{
    if (realName.isEmpty() && !m_showUnknown) {
        return;
    }

    auto fileItem = new TreeInformation();
    fileItem->hash = name;
    fileItem->name = realName;
    fileItem->type = TreeType::File;
    fileItem->parent = parentItem;
    fileItem->row = parentItem->children.size();
    fileItem->originalHash = originalHash;
    fileItem->indexPath = indexPath;

    parentItem->children.push_back(fileItem);
}

void FileTreeModel::addFolder(TreeInformation *parentItem, uint32_t name)
{
    if (!m_showUnknown) {
        return;
    }

    auto fileItem = new TreeInformation();
    fileItem->hash = name;
    fileItem->type = TreeType::Folder;
    fileItem->parent = parentItem;
    fileItem->row = fileItem->parent->children.size();

    parentItem->children.push_back(fileItem);
}

#include "moc_filetreemodel.cpp"
