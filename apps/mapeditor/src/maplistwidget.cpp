// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maplistwidget.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

MapListWidget::MapListWidget(physis_SqPackResource *data, QWidget *parent)
    : QDialog(parent)
    , data(data)
{
    setModal(true);
    setMinimumSize(QSize(640, 480));

    auto layout = new QVBoxLayout();
    setLayout(layout);

    m_searchModel = new QSortFilterProxyModel();
    m_searchModel->setRecursiveFilteringEnabled(true);
    m_searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(searchEdit);

    auto originalModel = new QStandardItemModel();
    m_searchModel->setSourceModel(originalModel);

    auto nameExh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/PlaceName.exh"));
    auto territoryExh = physis_exh_parse(data->platform, physis_sqpack_read(data, "exd/TerritoryType.exh"));

    auto nameSheet = physis_sqpack_read_excel_sheet(data, "PlaceName", &nameExh, Language::English);
    auto territorySheet = physis_sqpack_read_excel_sheet(data, "TerritoryType", &territoryExh, Language::None);

    // TODO: figure out why row_count in EXH is wrong?!
    for (uint32_t i = 0; i < territoryExh.pages[0].row_count; i++) {
        auto territoryExdRow = physis_excel_get_row(&territorySheet, i); // TODO: free, use all rows
        if (territoryExdRow.columns) {
            const char *bg = territoryExdRow.columns[1].string._0;
            if (strlen(bg) == 0) {
                continue;
            }

            int placeRegionKey = territoryExdRow.columns[3].u_int16._0;
            auto regionExdRow = physis_excel_get_row(&nameSheet, placeRegionKey); // TODO: free, use all rows
            const char *placeRegion = regionExdRow.columns[0].string._0;

            int placeZoneKey = territoryExdRow.columns[4].u_int16._0;
            auto zoneExdRow = physis_excel_get_row(&nameSheet, placeZoneKey); // TODO: free, use all rows
            const char *placeZone = zoneExdRow.columns[0].string._0;

            int placeNameKey = territoryExdRow.columns[5].u_int16._0;
            auto nameExdRow = physis_excel_get_row(&nameSheet, placeNameKey); // TODO: free, use all rows
            const char *placeName = nameExdRow.columns[0].string._0;

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
    listWidget->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    listWidget->setModel(m_searchModel);

    connect(listWidget, &QListView::activated, this, &MapListWidget::accept);

    layout->addWidget(listWidget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MapListWidget::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MapListWidget::reject);
    layout->addWidget(buttonBox);

    // Disable when there's no selection
    connect(listWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, buttonBox] {
        buttonBox->button(QDialogButtonBox::Open)->setEnabled(listWidget->selectionModel()->hasSelection());
    });

    // And it should be disabled by default
    buttonBox->button(QDialogButtonBox::Open)->setEnabled(false);
}

QString MapListWidget::acceptedMap() const
{
    return m_acceptedMap;
}

void MapListWidget::accept()
{
    // Figure out the selection first
    const auto index = listWidget->selectionModel()->selectedIndexes().constFirst();
    if (index.isValid()) {
        m_acceptedMap = m_searchModel->mapToSource(index).data(Qt::UserRole + 1).toString();
    }

    QDialog::accept();
}

#include "moc_maplistwidget.cpp"
