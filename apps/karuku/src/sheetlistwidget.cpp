// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sheetlistwidget.h"

#include <KLocalizedString>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QVBoxLayout>

SheetListWidget::SheetListWidget(SqPackResource *data, QWidget *parent)
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

    m_searchEdit = new QLineEdit();
    m_searchEdit->setWhatsThis(i18nc("@info:whatsthis", "Search box for Excel sheet names."));
    m_searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(m_searchEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(m_searchEdit);

    auto originalModel = new QStringListModel();
    searchModel->setSourceModel(originalModel);

    QStringList list;

    auto names = physis_gamedata_get_all_sheet_names(data);
    for (uint32_t i = 0; i < names.name_count; i++) {
        list.push_back(QString::fromStdString(names.names[i]));
    }

    originalModel->setStringList(list);

    listWidget = new QListView();
    listWidget->setWhatsThis(i18nc("@info:whatsthis", "A list of Excel sheet names. Select one to view it's contents."));
    listWidget->setModel(searchModel);
    listWidget->setEditTriggers(QListView::EditTrigger::NoEditTriggers);

    connect(listWidget, &QListView::activated, [this, searchModel](const QModelIndex &index) {
        Q_EMIT sheetSelected(searchModel->mapToSource(index).data(Qt::DisplayRole).toString());
    });

    layout->addWidget(listWidget);
}

void SheetListWidget::focusSearchField()
{
    m_searchEdit->setFocus(Qt::FocusReason::ShortcutFocusReason);
}

#include "moc_sheetlistwidget.cpp"
