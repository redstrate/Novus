// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gearlistmodel.h"

#include <KLocalizedString>
#include <QtConcurrent>
#include <magic_enum.hpp>

GearListModel::GearListModel(GameData *data, QObject *parent)
    : QAbstractItemModel(parent)
    , gameData(data)
{
    // smallclothes body
    {
        GearInfo info = {};
        info.name = "SmallClothes Body";
        info.slot = Slot::Body;

        gears.push_back(info);
    }

    // smallclothes legs
    {
        GearInfo info = {};
        info.name = "SmallClothes Legs";
        info.slot = Slot::Legs;

        gears.push_back(info);
    }

    exh = physis_parse_excel_sheet_header(physis_gamedata_extract_file(data, "exd/item.exh"));

    exdFuture = new QFutureWatcher<physis_EXD>(this);
    connect(exdFuture, &QFutureWatcher<physis_EXD>::resultReadyAt, this, &GearListModel::exdFinished);
    connect(exdFuture, &QFutureWatcher<physis_EXD>::finished, this, &GearListModel::finished);

    QVector<uint32_t> pages;
    for (uint32_t i = 0; i < exh->page_count; i++) {
        pages.push_back(i);
    }

    std::function<physis_EXD(int)> loadEXD = [this, data](const int page) -> physis_EXD {
        return physis_gamedata_read_excel_sheet(data, "Item", exh, Language::English, page);
    };

    exdFuture->setFuture(QtConcurrent::mapped(pages, loadEXD));

    for (auto slotName : magic_enum::enum_names<Slot>()) {
        slotNames.push_back(QLatin1String(slotName.data()));
    }

    rootItem = new TreeInformation();
    rootItem->type = TreeType::Root;
}

int GearListModel::rowCount(const QModelIndex &parent) const
{
    TreeInformation *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeInformation *>(parent.internalPointer());

    return static_cast<int>(parentItem->children.size());
}

int GearListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QModelIndex GearListModel::index(int row, int column, const QModelIndex &parent) const
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
    return {};
}

QModelIndex GearListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto childItem = static_cast<TreeInformation *>(index.internalPointer());
    TreeInformation *parentItem = childItem->parent;

    if (parentItem == rootItem)
        return {};

    return createIndex(parentItem->row, 0, parentItem);
}

QVariant GearListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto item = static_cast<TreeInformation *>(index.internalPointer());

    if (item->type == TreeType::Category) {
        if (role == Qt::DisplayRole) {
            return QLatin1String(magic_enum::enum_name(*item->slotType).data());
        }
    } else if (item->type == TreeType::Item) {
        if (role == Qt::DisplayRole) {
            return QLatin1String(item->gear->name.data());
        } else if (role == Qt::DecorationRole) {
            // TODO: cache these images in memory
            const QString iconName = QString::number(item->gear->icon);
            const QString iconBaseNum = QString::number(item->gear->icon).left(2).leftJustified(iconName.length(), QLatin1Char('0'));

            const QString iconFolder = QStringLiteral("ui/icon/%1").arg(iconBaseNum, 6, QLatin1Char('0'));
            const QString iconFile = QStringLiteral("%1.tex").arg(iconName, 6, QLatin1Char('0'));

            const std::string iconFilename = iconFolder.toStdString() + "/" + iconFile.toStdString();

            auto texFile = physis_gamedata_extract_file(gameData, iconFilename.c_str());
            if (texFile.data != nullptr) {
                auto tex = physis_texture_parse(texFile);
                if (tex.rgba != nullptr) {
                    QImage image(tex.rgba, static_cast<int>(tex.width), static_cast<int>(tex.height), QImage::Format_RGBA8888);

                    QPixmap pixmap;
                    pixmap.convertFromImage(image);

                    return pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
            }

            return QIcon::fromTheme(QStringLiteral("unknown"));
        }
    }

    return {};
}

QVariant GearListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return i18nc("@title:column Item name", "Name");
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

std::optional<GearInfo> GearListModel::getGearFromIndex(const QModelIndex &index)
{
    auto item = static_cast<TreeInformation *>(index.internalPointer());
    if (item->type == TreeType::Item) {
        return item->gear;
    }
    return {};
}

void GearListModel::exdFinished(int index)
{
    auto exd = exdFuture->resultAt(index);

    for (unsigned int i = 0; i < exh->row_count; i++) {
        const auto row = physis_exd_read_row(&exd, i); // TODO: use all rows, free

        auto primaryModel = row.row_data[0].column_data[47].u_int64._0;
        // auto secondaryModel = row.column_data[48].u_int64._0;

        int16_t parts[4];
        memcpy(parts, &primaryModel, sizeof(int16_t) * 4);

        GearInfo info = {};
        info.name = row.row_data[0].column_data[9].string._0;
        info.icon = row.row_data[0].column_data[10].u_int16._0;
        info.slot = physis_slot_from_id(row.row_data[0].column_data[17].u_int8._0);
        info.modelInfo.primaryID = parts[0];

        gears.push_back(info);
    }
}

void GearListModel::finished()
{
    beginResetModel();

    int i = 0;
    for (auto slot : magic_enum::enum_values<Slot>()) {
        auto categoryItem = new TreeInformation();
        categoryItem->type = TreeType::Category;
        categoryItem->slotType = slot;
        categoryItem->parent = rootItem;
        categoryItem->row = i++;
        rootItem->children.push_back(categoryItem);

        int j = 0;
        for (const auto &gear : gears) {
            if (gear.slot == slot) {
                auto item = new TreeInformation();
                item->type = TreeType::Item;
                item->gear = gear;
                item->parent = categoryItem;
                item->row = j++;
                categoryItem->children.push_back(item);
            }
        }
    }
    endResetModel();
}

#include "moc_gearlistmodel.cpp"
