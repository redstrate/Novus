// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "hashdatabase.h"
#include "physis.hpp"

#include <QAbstractItemModel>
#include <QFutureWatcher>

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
        for (const auto child : children) {
            if (child->nameHash == nameHash) {
                return true;
            }
        }

        return false;
    }
};

class FileTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FileTreeModel(HashDatabase &database, bool showUnknown, const QString &gamePath, SqPackResource *data, QObject *parent = nullptr);

    enum CustomRoles {
        PathRole = Qt::UserRole,
        IsUnknownRole,
        IsFolderRole,
        HashRole,
        IndexPathRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    SqPackResource *gameData = nullptr;
    TreeInformation *rootItem = nullptr;

    void addKnownFolder(const QString &string);
    void addFile(TreeInformation *parentItem, uint32_t filenameHash, const QString &name, uint32_t nameHash, Hash originalHash, const QString &indexPath);
    void addUnknownFolder(TreeInformation *parentItem, uint32_t filenameHash);

    QHash<uint32_t, TreeInformation *> knownDirHashes;

    HashDatabase &m_database;
    bool m_showUnknown = false;
};
