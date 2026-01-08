// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractItemModel>
#include <QFutureWatcher>

#include "gearview.h"

enum class TreeType { Root, Category, Item };

struct SceneTreeInformation {
    TreeType type;
    std::optional<Slot> slotType;
    SceneTreeInformation *parent = nullptr;
    std::optional<GearInfo> gear;
    int row = 0;

    std::vector<SceneTreeInformation *> children;
};

class GearListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit GearListModel(physis_SqPackResource *data, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    std::optional<GearInfo> getGearFromIndex(const QModelIndex &index);

private:
    physis_EXH m_exh;
    physis_ExcelSheet m_sheet;

    std::vector<GearInfo> gears;
    QStringList slotNames;

    physis_SqPackResource *gameData = nullptr;
    SceneTreeInformation *rootItem = nullptr;
};
