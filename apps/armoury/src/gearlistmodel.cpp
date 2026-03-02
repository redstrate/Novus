// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gearlistmodel.h"

#include "settings.h"

#include <KLocalizedString>
#include <QtConcurrent>
#include <magic_enum.hpp>

GearListModel::GearListModel(physis_SqPackResource *data, QObject *parent)
    : QAbstractItemModel(parent)
    , gameData(data)
{
    // smallclothes body
    {
        GearInfo info = {};
        info.name = i18n("SmallClothes Body");
        info.slot = Slot::Body;

        gears.push_back(info);
    }

    // smallclothes legs
    {
        GearInfo info = {};
        info.name = i18n("SmallClothes Legs");
        info.slot = Slot::Legs;

        gears.push_back(info);
    }

    m_exh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/item.exh"));
    m_sheet = physis_sqpack_read_excel_sheet(data, "Item", &m_exh, getLanguage());

    for (unsigned int i = 0; i < m_sheet.page_count; i++) {
        for (unsigned int j = m_exh.pages[i].start_id; j < m_exh.pages[i].start_id + m_sheet.pages[i].entry_count; j++) {
            const auto row = physis_excel_get_row(&m_sheet, j); // TODO: use all rows, free
            if (row.columns) {
                auto primaryModel = row.columns[47].u_int64._0;
                // auto secondaryModel = row.column_data[48].u_int64._0;

                int16_t parts[4];
                memcpy(parts, &primaryModel, sizeof(int16_t) * 4);

                const auto slot = physis_slot_from_id(row.columns[17].u_int8._0);
                if (slot == Slot::Invalid) {
                    continue;
                }

                GearInfo info = {};
                info.name = QString::fromStdString(row.columns[9].string._0);
                info.icon = row.columns[10].u_int16._0;
                info.slot = slot;
                info.modelInfo.primaryID = parts[0];

                gears.push_back(info);
            }
        }
    }

    beginResetModel();

    rootItem = new SceneTreeInformation();
    rootItem->type = TreeType::Root;

    int i = 0;
    for (auto slot : magic_enum::enum_values<Slot>()) {
        if (slot == Slot::Invalid) {
            continue;
        }

        auto categoryItem = new SceneTreeInformation();
        categoryItem->type = TreeType::Category;
        categoryItem->slotType = slot;
        categoryItem->parent = rootItem;
        categoryItem->row = i++;
        rootItem->children.push_back(categoryItem);

        int j = 0;
        for (const auto &gear : gears) {
            if (gear.slot == slot) {
                auto item = new SceneTreeInformation();
                item->type = TreeType::Item;
                item->gear = gear;
                item->parent = categoryItem;
                item->row = j++;
                categoryItem->children.push_back(item);
            }
        }
    }
    endResetModel();

    for (auto slotName : magic_enum::enum_names<Slot>()) {
        slotNames.push_back(QLatin1String(slotName.data()));
    }
}

int GearListModel::rowCount(const QModelIndex &parent) const
{
    SceneTreeInformation *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SceneTreeInformation *>(parent.internalPointer());

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

    SceneTreeInformation *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SceneTreeInformation *>(parent.internalPointer());

    SceneTreeInformation *childItem = parentItem->children[row];
    if (childItem)
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex GearListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto childItem = static_cast<SceneTreeInformation *>(index.internalPointer());
    SceneTreeInformation *parentItem = childItem->parent;

    if (parentItem == rootItem)
        return {};

    return createIndex(parentItem->row, 0, parentItem);
}

QVariant GearListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto item = static_cast<SceneTreeInformation *>(index.internalPointer());

    if (item->type == TreeType::Category) {
        if (role == Qt::DisplayRole) {
            return QLatin1String(magic_enum::enum_name(*item->slotType).data());
        }
    } else if (item->type == TreeType::Item) {
        if (role == Qt::DisplayRole) {
            return item->gear->name;
        } else if (role == Qt::DecorationRole) {
            // TODO: cache these images in memory
            const QString iconName = QString::number(item->gear->icon);
            const QString iconBaseNum = QString::number(item->gear->icon).left(2).leftJustified(iconName.length(), QLatin1Char('0'));

            const QString iconFolder = QStringLiteral("ui/icon/%1").arg(iconBaseNum, 6, QLatin1Char('0'));
            const QString iconFile = QStringLiteral("%1.tex").arg(iconName, 6, QLatin1Char('0'));

            const std::string iconFilename = iconFolder.toStdString() + "/" + iconFile.toStdString();

            auto texFile = physis_sqpack_read(gameData, iconFilename.c_str());
            if (texFile.data != nullptr) {
                auto tex = physis_texture_parse(gameData->platform, texFile);
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
    auto item = static_cast<SceneTreeInformation *>(index.internalPointer());
    if (item->type == TreeType::Item) {
        return item->gear;
    }
    return {};
}

#include "moc_gearlistmodel.cpp"
