// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "difftreemodel.h"
#include "filetypes.h"
#include "physis.hpp"
#include "settings.h"

#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QIcon>

DiffTreeModel::DiffTreeModel(HashDatabase &database, physis_SqPackResource *data, QObject *parent)
    : QAbstractItemModel(parent)
    , gameData(data)
    , m_database(database)
{
    rootItem = new TreeInformation();
    rootItem->type = TreeType::Root;
}

int DiffTreeModel::rowCount(const QModelIndex &parent) const
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

int DiffTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QModelIndex DiffTreeModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex DiffTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto childItem = static_cast<TreeInformation *>(index.internalPointer());
    TreeInformation *parentItem = childItem->parent;

    if (parentItem == rootItem)
        return {};

    return createIndex(parentItem->row, index.column(), parentItem);
}

QVariant DiffTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    const auto item = static_cast<TreeInformation *>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        if (item->type == TreeType::Folder) {
            if (item->name.isEmpty()) {
                return i18n("Unknown Folder (%1)").arg(item->hash);
            }
            return item->name;
        }
        if (item->type == TreeType::File) {
            if (item->name.isEmpty()) {
                return i18n("Unknown File (%1)").arg(item->hash);
            }
            return item->name;
        }
    }
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
    }
    if (role == Qt::DecorationRole) {
        if (item->type == TreeType::Folder) {
            return QIcon::fromTheme(QStringLiteral("folder-symbolic"));
        }
        if (item->type == TreeType::File) {
            QFileInfo info(item->name);
            const FileType type = FileTypes::getFileType(info.completeSuffix());

            return QIcon::fromTheme(FileTypes::getFiletypeIcon(type));
        }
    }

    return {};
}

QVariant DiffTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return i18nc("@title:column", "Name");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

void DiffTreeModel::openPatch(const QString &path)
{
    beginResetModel();

    rootItem = new TreeInformation();
    rootItem->type = TreeType::Root;

    auto patch = physis_patch_parse(path.toStdString().c_str());

    SqpkTargetInfo targetInfo{};

    for (uint32_t i = 0; i < patch.num_chunks; i++) {
        const auto chunk = patch.chunks[i];
        switch (chunk.chunk_type.tag) {
        case physis_ZiPatchChunkType::Tag::Sqpk: {
            const auto sqpk = chunk.chunk_type.sqpk._0;
            switch (sqpk.operation.tag) {
            case physis_ZiPatchSqpkOperation::Tag::AddData: {
                const auto addData = sqpk.operation.add_data._0;

                const auto indexPath = physis_patch_index_path(targetInfo, addData.main_id, addData.sub_id, addData.file_id);
                const auto indexGamePath = QDir(getGameDirectory()).absoluteFilePath(QString::fromStdString(indexPath));

                const auto indexFile = physis_index_parse(gameData->platform, indexGamePath.toStdString().c_str());
                if (indexFile.p_ptr) {
                    auto hash = physis_index_hash_from_offset(indexFile, addData.block_offset);

                    // Add parent folder
                    addPath(m_database.getFolder(hash.split_path.path));

                    const auto completeHash =
                        static_cast<uint32_t>(static_cast<uint64_t>(hash.split_path.path) << 32 | static_cast<uint64_t>(hash.split_path.name));

                    const auto parentItem = knownDirHashes.value(hash.split_path.path);
                    if (parentItem) {
                        // Actual file item
                        auto pathItem = new TreeInformation();
                        pathItem->name = m_database.getFilename(completeHash);
                        pathItem->parent = parentItem;
                        pathItem->type = TreeType::File;
                        pathItem->row = parentItem->children.size();
                        pathItem->hash = completeHash; // FIXME: is this the correct/useful thing to show?

                        parentItem->children.push_back(pathItem);
                    } else {
                        qWarning() << "Could not find parent item for" << hash.split_path.path << "item will not be added!";
                    }
                } else {
                    qWarning() << "Could not read index file" << indexGamePath;
                }
            } break;
            case physis_ZiPatchSqpkOperation::Tag::TargetInfo:
                targetInfo = sqpk.operation.target_info._0;
                break;
            case physis_ZiPatchSqpkOperation::Tag::Unknown:
                break;
            }
        } break;
        case physis_ZiPatchChunkType::Tag::Unknown:
            break;
        }
    }

    endResetModel();
}

void DiffTreeModel::addPath(const QString &string)
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

#include "moc_difftreemodel.cpp"
