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

    qInfo() << "Reading index files...";

    QDirIterator it(QStringLiteral("%1/sqpack").arg(gamePath), QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        if (info.exists() && info.completeSuffix().contains(QStringLiteral(".index"))) {
            std::string pathStd = info.filePath().toStdString();
            const auto indexEntries = physis_index_parse(pathStd.c_str());
            for (uint32_t i = 0; i < indexEntries.num_hashes; i++) {
                const auto hash = indexEntries.hashes[i];
                switch (hash.tag) {
                case Hash::Tag::SplitPath: {
                    if (!knownDirHashes.contains(hash.split_path.path)) {
                        addUnknownFolder(rootItem, hash.split_path.path);
                    }

                    const auto completeHash =
                        static_cast<uint32_t>(static_cast<uint64_t>(hash.split_path.path) << 32 | static_cast<uint64_t>(hash.split_path.name));

                    QString name;
                    if (m_database.knowsFile(completeHash)) {
                        name = m_database.getFilename(completeHash);
                    }
                    addFile(knownDirHashes.value(hash.split_path.path), completeHash, name, hash.split_path.name, hash, info.filePath());
                } break;
                case Hash::Tag::FullPath:
                    if (m_database.knowsPath(hash.full_path._0)) {
                        const QString path = m_database.getPath(hash.full_path._0);

                        if (path.contains(QStringLiteral("/"))) {
                            const int lastSlash = path.lastIndexOf(QStringLiteral("/"));
                            const QString filename = path.sliced(lastSlash + 1, path.length() - lastSlash - 1);
                            const QString foldername = path.left(lastSlash);
                            const auto folderHash = physis_generate_partial_hash(foldername.toStdString().c_str());

                            addFile(knownDirHashes.value(folderHash),
                                    hash.full_path._0,
                                    filename,
                                    physis_generate_partial_hash(filename.toStdString().c_str()),
                                    hash,
                                    info.filePath());
                        } else {
                            Q_UNREACHABLE(); // root files don't exist in FFXIV
                        }
                    }
                }
            }
        }
    }

    qInfo() << "Finished reading!";
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
        if (item->name.isEmpty()) {
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

QModelIndex FileTreeModel::search(const QString &path) const
{
    const auto list = match(index(0, 0), PathRole, path, 1, Qt::MatchExactly | Qt::MatchRecursive);
    if (list.isEmpty()) {
        return {};
    }
    return list.constFirst();
}

void FileTreeModel::addKnownFolder(const QString &string)
{
    const QStringList children = string.split(QLatin1Char('/'));

    QString conct = children[0];
    conct.reserve(string.length());
    TreeInformation *parentItem = rootItem;
    for (int i = 0; i < children.size(); i++) {
        if (i > 0) {
            conct += QStringLiteral("/%1").arg(children[i]);
        }
        std::string conctStd = conct.toStdString();
        const auto hash = physis_generate_partial_hash(conctStd.c_str());

        if (knownDirHashes.contains(hash)) {
            parentItem = knownDirHashes.value(hash);
        } else {
            auto folderItem = new TreeInformation();
            folderItem->name = children[i];
            folderItem->type = TreeType::Folder;
            folderItem->parent = parentItem;
            folderItem->row = parentItem->children.size();
            folderItem->hash = hash;
            parentItem->children.push_back(folderItem);
            parentItem = folderItem;
            knownDirHashes.insert(folderItem->hash, folderItem);
        }
    }
}

void FileTreeModel::addFile(TreeInformation *parentItem, uint32_t name, const QString &realName, uint32_t nameHash, Hash originalHash, const QString &indexPath)
{
    if (realName.isEmpty() && !m_showUnknown) {
        return;
    }

    if (!parentItem) {
        return;
    }

    // Skip files we already found
    if (parentItem->contains(nameHash)) {
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
    fileItem->nameHash = nameHash;

    parentItem->children.push_back(fileItem);
}

void FileTreeModel::addUnknownFolder(TreeInformation *parentItem, uint32_t name)
{
    if (!m_showUnknown) {
        return;
    }

    auto folderItem = new TreeInformation();
    folderItem->hash = name;
    folderItem->type = TreeType::Folder;
    folderItem->parent = parentItem;
    folderItem->row = folderItem->parent->children.size();

    parentItem->children.push_back(folderItem);

    knownDirHashes.insert(name, folderItem);
}

#include "moc_filetreemodel.cpp"
