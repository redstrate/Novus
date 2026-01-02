// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maplistwidget.h"

#include <KLocalizedString>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

MapListWidget::MapListWidget(physis_SqPackResource *data, QWidget *parent)
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
    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(searchEdit);

    auto originalModel = new QStandardItemModel();
    searchModel->setSourceModel(originalModel);

    auto nameExh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/PlaceName.exh"));
    auto territoryExh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/TerritoryType.exh"));

    auto nameSheet = physis_sqpack_read_excel_sheet(data, "PlaceName", &nameExh, Language::English);
    auto territorySheet = physis_sqpack_read_excel_sheet(data, "TerritoryType", &territoryExh, Language::None);

    // TODO: figure out why row_count in EXH is wrong?!
    for (uint32_t i = 0; i < territoryExh.pages[0].row_count; i++) {
        auto territoryExdRow = physis_excel_get_row(&territorySheet, i); // TODO: free, use all rows
        if (territoryExdRow.row_data) {
            const char *bg = territoryExdRow.row_data[0].column_data[1].string._0;
            if (strlen(bg) == 0) {
                continue;
            }

            int placeRegionKey = territoryExdRow.row_data[0].column_data[3].u_int16._0;
            auto regionExdRow = physis_excel_get_row(&nameSheet, placeRegionKey); // TODO: free, use all rows
            const char *placeRegion = regionExdRow.row_data[0].column_data[0].string._0;

            int placeZoneKey = territoryExdRow.row_data[0].column_data[4].u_int16._0;
            auto zoneExdRow = physis_excel_get_row(&nameSheet, placeZoneKey); // TODO: free, use all rows
            const char *placeZone = zoneExdRow.row_data[0].column_data[0].string._0;

            int placeNameKey = territoryExdRow.row_data[0].column_data[5].u_int16._0;
            auto nameExdRow = physis_excel_get_row(&nameSheet, placeNameKey); // TODO: free, use all rows
            const char *placeName = nameExdRow.row_data[0].column_data[0].string._0;

            QStandardItem *item = new QStandardItem();
            item->setData(QString::fromStdString(bg));
            item->setText(QStringLiteral("%1 %2 (%3, %4, %5)")
                              .arg(QString::number(i))
                              .arg(QString::fromStdString(bg),
                                   QString::fromStdString(placeRegion),
                                   QString::fromStdString(placeZone),
                                   QString::fromStdString(placeName)));

            originalModel->insertRow(originalModel->rowCount(), item);
        }
    }

    listWidget = new QListView();
    listWidget->setModel(searchModel);

    connect(listWidget, &QListView::activated, [this, searchModel](const QModelIndex &index) {
        Q_EMIT mapSelected(searchModel->mapToSource(index).data(Qt::UserRole + 1).toString());
    });

    layout->addWidget(listWidget);
}

#include "moc_maplistwidget.cpp"
