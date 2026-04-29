// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "hashdatabase.h"
#include "physis.hpp"

#include <QAbstractItemModel>
#include <QFutureWatcher>

struct SqPackResource;

enum class TreeType {
    Root,
    Folder,
    File
};

struct TreeInformation {
    TreeType type;
    TreeInformation *parent = nullptr;
    QString name;
    int row = 0;
    uint32_t hash = 0;
    physis_Buffer buffer = {};

    std::vector<TreeInformation *> children;
};

class DiffTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DiffTreeModel(HashDatabase &database, physis_SqPackResource *data, QObject *parent = nullptr);

    enum CustomRoles {
        PathRole = Qt::UserRole,
        BufferRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void openPatch(const QString &path);

private:
    void addGamePath(TreeInformation *baseItem, const QString &string);
    TreeInformation *addIndexPath(const QString &string);

    physis_SqPackResource *gameData = nullptr;
    TreeInformation *rootItem = nullptr;
    HashDatabase &m_database;
    QHash<uint32_t, TreeInformation *> knownDirHashes;
    QHash<uint32_t, TreeInformation *> knownIndexHashes;
};
