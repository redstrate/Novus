// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectlistwidget.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QVBoxLayout>

#include "appstate.h"
#include "objectlistmodel.h"

ObjectListWidget::ObjectListWidget(AppState *appState, QWidget *parent)
    : QWidget(parent)
    , m_appState(appState)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    auto searchModel = new QSortFilterProxyModel();
    searchModel->setRecursiveFilteringEnabled(true);
    searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    auto searchEdit = new QLineEdit();
    searchEdit->setWhatsThis(i18nc("@info:whatsthis", "Search box for objects."));
    searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(searchEdit);

    m_objectListModel = new ObjectListModel(appState, this);
    searchModel->setSourceModel(m_objectListModel);

    treeWidget = new QTreeView();
    treeWidget->setWhatsThis(i18nc("@info:whatsthis", "A list of objects on this map."));
    treeWidget->setModel(searchModel);
    treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    connect(treeWidget, &QTreeView::activated, this, [this, searchModel](const QModelIndex &index) {
        auto originalIndex = searchModel->mapToSource(index);
        m_appState->selectedObject = m_objectListModel->objectId(originalIndex);
        Q_EMIT m_appState->selectedObjectChanged();
    });

    layout->addWidget(treeWidget);
}

#include "moc_objectlistwidget.cpp"
