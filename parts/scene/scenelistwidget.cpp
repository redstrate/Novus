// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenelistwidget.h"

#include <KLocalizedString>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QVBoxLayout>

#include "scenelistmodel.h"
#include "scenestate.h"

SceneListWidget::SceneListWidget(SceneState *appState, QWidget *parent)
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

    m_searchEdit = new QLineEdit();
    m_searchEdit->setWhatsThis(i18nc("@info:whatsthis", "Search box for objects."));
    m_searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(m_searchEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(m_searchEdit);

    treeWidget = new QTreeView();
    treeWidget->setWhatsThis(i18nc("@info:whatsthis", "A list of objects on this map."));
    treeWidget->setModel(searchModel);
    treeWidget->header()->setStretchLastSection(false);
    treeWidget->header()->setSectionsMovable(false);
    connect(treeWidget, &QTreeView::activated, this, [this, searchModel](const QModelIndex &index) {
        auto originalIndex = searchModel->mapToSource(index);
        m_appState->selectedObject = m_objectListModel->objectAt(originalIndex);
        m_appState->selectedLayer = m_objectListModel->layerAt(originalIndex);
        m_appState->selectedTimeline = m_objectListModel->timelineAt(originalIndex);
        m_appState->selectedAction = m_objectListModel->actionAt(originalIndex);
        Q_EMIT m_appState->selectionChanged();
    });
    layout->addWidget(treeWidget);

    m_objectListModel = new SceneListModel(m_appState, treeWidget);
    connect(m_objectListModel, &SceneListModel::modelReset, this, [this] {
        // This has to be called *here*, for some reason...
        treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    });
    searchModel->setSourceModel(m_objectListModel);
}

void SceneListWidget::expandToDepth(const int depth)
{
    treeWidget->expandToDepth(depth);
}

void SceneListWidget::focusSearchField()
{
    m_searchEdit->setFocus(Qt::FocusReason::ShortcutFocusReason);
}
