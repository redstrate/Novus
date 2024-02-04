// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sheetlistwidget.h"

#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QVBoxLayout>

SheetListWidget::SheetListWidget(GameData *data, QWidget *parent)
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

    auto originalModel = new QStringListModel();
    searchModel->setSourceModel(originalModel);

    QStringList list;

    auto names = physis_gamedata_get_all_sheet_names(data);
    for (uint32_t i = 0; i < names.name_count; i++) {
        list.push_back(QString::fromStdString(names.names[i]));
    }

    originalModel->setStringList(list);

    listWidget = new QListView();
    listWidget->setModel(searchModel);

    connect(listWidget, &QListView::clicked, [this, searchModel](const QModelIndex &index) {
        Q_EMIT sheetSelected(searchModel->mapToSource(index).data(Qt::DisplayRole).toString());
    });

    layout->addWidget(listWidget);
}

#include "moc_sheetlistwidget.cpp"