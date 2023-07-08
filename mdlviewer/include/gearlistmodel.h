#pragma once

#include "gearview.h"
#include <QAbstractItemModel>

enum class TreeType {
    Root,
    Category,
    Item
};

struct TreeInformation {
    TreeType type;
    std::optional<Slot> slotType;
    TreeInformation* parent = nullptr;
    std::optional<GearInfo> gear;
    int row = 0;

    std::vector<TreeInformation*> children;
};

class GearListModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit GearListModel(GameData* data);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::optional<GearInfo> getGearFromIndex(const QModelIndex& index);

private:
    std::vector<GearInfo> gears;
    QStringList slotNames;

    GameData* gameData = nullptr;
    TreeInformation* rootItem = nullptr;
};