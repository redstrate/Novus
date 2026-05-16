// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gearlistwidget.h"

#include <KLocalizedString>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <magic_enum.hpp>

#include "gearlistmodel.h"

GearListWidget::GearListWidget(FileCache &cache, QWidget *parent)
    : QWidget(parent)
    , m_cache(cache)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    auto searchModel = new QSortFilterProxyModel();
    searchModel->setRecursiveFilteringEnabled(true);
    searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(i18nc("@info:placeholder Search through items", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(searchEdit, &QLineEdit::textChanged, searchModel, qOverload<const QString &>(&QSortFilterProxyModel::setFilterRegularExpression));
    layout->addWidget(searchEdit);

    auto originalModel = new GearListModel(m_cache);
    searchModel->setSourceModel(originalModel);

    m_listWidget = new QTreeView();
    m_listWidget->setModel(searchModel);

    connect(m_listWidget, &QTreeView::clicked, [this, searchModel, originalModel](const QModelIndex &item) {
        if (auto gear = originalModel->getGearFromIndex(searchModel->mapToSource(item))) {
            Q_EMIT gearSelected(*gear);
        }
    });

    layout->addWidget(m_listWidget);
}

#include "moc_gearlistwidget.cpp"
