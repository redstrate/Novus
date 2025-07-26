// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "hashdatabase.h"
#include <QAbstractItemModel>
#include <QFutureWatcher>

struct SqPackResource;

enum class TreeType { Root, Folder, File };

struct TreeInformation {
    TreeType type;
    TreeInformation *parent = nullptr;
    QString name;
    int row = 0;
    uint32_t hash = 0;

    std::vector<TreeInformation *> children;
};

class FileTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FileTreeModel(HashDatabase &database, bool showUnknown, const QString &gamePath, SqPackResource *data, QObject *parent = nullptr);

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
    void addFile(TreeInformation *parentItem, uint32_t filenameHash, const QString &name);
    void addFolder(TreeInformation *parentItem, uint32_t filenameHash);

    QHash<uint32_t, TreeInformation *> knownDirHashes;

    HashDatabase &m_database;
    bool m_showUnknown = false;
};
