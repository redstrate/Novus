// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "hashdatabase.h"
#include "physis.hpp"

#include <QAbstractItemModel>
#include <QFutureWatcher>

class FileCache;
struct SqPackResource;

enum class TreeType { Root, Folder, File };

struct TreeInformation {
    TreeType type;
    TreeInformation *parent = nullptr;
    QString name;
    uint32_t nameHash = 0;
    int row = 0;
    uint32_t hash = 0;
    Hash originalHash;
    QString indexPath;

    std::vector<TreeInformation *> children;

    bool contains(const uint32_t &nameHash) const
    {
        return std::ranges::any_of(children, [nameHash](auto child) {
            return child->nameHash == nameHash;
        });
    }
};

class FileTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FileTreeModel(HashDatabase &database, bool showUnknown, const QString &gamePath, FileCache &cache, QObject *parent = nullptr);

    enum CustomRoles {
        PathRole = Qt::UserRole,
        IsUnknownRole,
        IsFolderRole,
        HashRole,
        IndexPathRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QModelIndex search(const QString &path) const;

private:
    FileCache &m_cache;
    TreeInformation *m_rootItem = nullptr;

    void addKnownFolder(const QString &string);
    void
    addFile(TreeInformation *parentItem, uint32_t filenameHash, const QString &realName, uint32_t nameHash, Hash originalHash, const QString &indexPath) const;
    void addUnknownFolder(TreeInformation *parentItem, uint32_t filenameHash);

    QHash<uint32_t, TreeInformation *> m_knownDirHashes;

    HashDatabase &m_database;
    bool m_showUnknown = false;
};
