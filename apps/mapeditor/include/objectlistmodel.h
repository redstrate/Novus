// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractItemModel>

class AppState;

enum class TreeType {
    /// Root of the tree
    Root,
    /// LGB file
    File,
    /// A layer.
    Layer,
    /// A single object
    Object,
};

struct TreeInformation {
    TreeType type;
    TreeInformation *parent = nullptr;
    int row = 0;
    QString name;

    std::vector<TreeInformation *> children;
};

class ObjectListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ObjectListModel(AppState *appState, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void refresh();

    AppState *m_appState = nullptr;
    TreeInformation *m_rootItem = nullptr;
};
