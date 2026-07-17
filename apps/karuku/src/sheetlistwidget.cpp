// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sheetlistwidget.h"

#include <KLocalizedString>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QVBoxLayout>

SheetListWidget::SheetListWidget(const physis_SqPackResource *data, QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    auto searchModel = new QSortFilterProxyModel(this);
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

    const auto originalModel = new QStringListModel(this);
    searchModel->setSourceModel(originalModel);

    QStringList list;

    const auto names = physis_sqpack_get_all_sheet_names(data);
    for (uint32_t i = 0; i < names.name_count; i++) {
        list.push_back(QString::fromStdString(names.names[i]));
    }
    physis_sqpack_free_all_sheet_names(names);

    originalModel->setStringList(list);

    m_listWidget = new QListView();
    m_listWidget->setWhatsThis(i18nc("@info:whatsthis", "A list of Excel sheet names. Select one to view it's contents."));
    m_listWidget->setModel(searchModel);
    m_listWidget->setEditTriggers(QListView::EditTrigger::NoEditTriggers);

    connect(m_listWidget, &QListView::activated, [this, searchModel](const QModelIndex &index) {
        Q_EMIT sheetSelected(searchModel->mapToSource(index).data(Qt::DisplayRole).toString());
    });

    layout->addWidget(m_listWidget);
}

void SheetListWidget::focusSearchField() const
{
    m_searchEdit->setFocus(Qt::FocusReason::ShortcutFocusReason);
}

void SheetListWidget::goToSheet(const QString &name) const
{
    const auto indices = m_listWidget->model()->match(m_listWidget->model()->index(0, 0), Qt::DisplayRole, name);
    if (indices.isEmpty()) {
        return;
    }
    m_listWidget->scrollTo(indices.constFirst(), QListView::ScrollHint::PositionAtCenter);
    m_listWidget->selectionModel()->select(indices.constFirst(), QItemSelectionModel::ClearAndSelect);
}

#include "moc_sheetlistwidget.cpp"
