// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maplistwidget.h"

#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

MapListWidget::MapListWidget(GameData *data, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    auto searchModel = new QSortFilterProxyModel();
    searchModel->setRecursiveFilteringEnabled(true);
    searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(QStringLiteral("Search..."));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(searchEdit);

    auto originalModel = new QStandardItemModel();
    searchModel->setSourceModel(originalModel);

    auto exh = physis_parse_excel_sheet_header(physis_gamedata_extract_file(data, "exd/Map.exh"));
    auto nameExh = physis_parse_excel_sheet_header(physis_gamedata_extract_file(data, "exd/PlaceName.exh"));
    auto territoryExh = physis_parse_excel_sheet_header(physis_gamedata_extract_file(data, "exd/TerritoryType.exh"));

    // Only one page, it seems?
    auto exd = physis_gamedata_read_excel_sheet(data, "Map", exh, Language::None, 0);
    auto nameExd = physis_gamedata_read_excel_sheet(data, "PlaceName", nameExh, Language::English, 0);
    auto territoryExd = physis_gamedata_read_excel_sheet(data, "TerritoryType", territoryExh, Language::None, 0);

    for (uint32_t i = 0; i < exd.row_count; i++) {
        const char *id = exd.row_data[i].column_data[6].string._0;

        int territoryTypeKey = exd.row_data[i].column_data[15].u_int16._0;
        if (territoryTypeKey > 0 && territoryTypeKey < territoryExd.row_count) {
            const char *bg = territoryExd.row_data[territoryTypeKey].column_data[1].string._0;

            int placeRegionKey = territoryExd.row_data[territoryTypeKey].column_data[3].u_int16._0;
            const char *placeRegion = nameExd.row_data[placeRegionKey].column_data[0].string._0;

            int placeZoneKey = territoryExd.row_data[territoryTypeKey].column_data[4].u_int16._0;
            const char *placeZone = nameExd.row_data[placeZoneKey].column_data[0].string._0;

            int placeNameKey = territoryExd.row_data[territoryTypeKey].column_data[5].u_int16._0;
            const char *placeName = nameExd.row_data[placeNameKey].column_data[0].string._0;

            QStandardItem *item = new QStandardItem();
            item->setData(QString::fromStdString(bg));
            item->setText(QStringLiteral("%1 (%2, %3, %4)")
                              .arg(QString::fromStdString(id),
                                   QString::fromStdString(placeRegion),
                                   QString::fromStdString(placeZone),
                                   QString::fromStdString(placeName)));

            originalModel->insertRow(originalModel->rowCount(), item);
        }
    }

    listWidget = new QListView();
    listWidget->setModel(searchModel);

    connect(listWidget, &QListView::clicked, [this, searchModel](const QModelIndex &index) {
        Q_EMIT mapSelected(searchModel->mapToSource(index).data(Qt::UserRole + 1).toString());
    });

    layout->addWidget(listWidget);
}

#include "moc_maplistwidget.cpp"