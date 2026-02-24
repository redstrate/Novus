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

    m_searchModel = new QSortFilterProxyModel();
    m_searchModel->setRecursiveFilteringEnabled(true);
    m_searchModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

    m_searchEdit = new QLineEdit();
    m_searchEdit->setWhatsThis(i18nc("@info:whatsthis", "Search box for objects."));
    m_searchEdit->setPlaceholderText(i18nc("@info:placeholder", "Search…"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_searchModel->setFilterRegularExpression(text);
    });
    layout->addWidget(m_searchEdit);

    treeWidget = new QTreeView();
    treeWidget->setWhatsThis(i18nc("@info:whatsthis", "A list of objects on this map."));
    treeWidget->setModel(m_searchModel);
    treeWidget->header()->setStretchLastSection(false);
    treeWidget->header()->setSectionsMovable(false);
    connect(treeWidget, &QTreeView::activated, this, [this](const QModelIndex &index) {
        auto originalIndex = m_searchModel->mapToSource(index);
        m_appState->selectedObject = m_objectListModel->objectAt(originalIndex);
        m_appState->selectedLayer = m_objectListModel->layerAt(originalIndex);
        m_appState->selectedTimeline = m_objectListModel->timelineAt(originalIndex);
        m_appState->selectedAction = m_objectListModel->actionAt(originalIndex);
        m_appState->selectedLgb = m_objectListModel->lgbAt(originalIndex);
        Q_EMIT m_appState->selectionChanged();
    });
    layout->addWidget(treeWidget);

    m_objectListModel = new SceneListModel(m_appState, treeWidget);
    connect(m_objectListModel, &SceneListModel::modelReset, this, [this] {
        // This has to be called *here*, for some reason...
        treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    });
    m_searchModel->setSourceModel(m_objectListModel);
}

void SceneListWidget::expandToDepth(const int depth)
{
    treeWidget->expandToDepth(depth);
}

void SceneListWidget::focusSearchField()
{
    m_searchEdit->setFocus(Qt::FocusReason::ShortcutFocusReason);
}

void SceneListWidget::selectObject(uint32_t objectId)
{
    m_searchEdit->clear(); // Clear the search field so all items are visible again.

    auto indices = m_objectListModel->match(m_objectListModel->index(0, 0, {}),
                                            SceneListModel::SceneListRoles::ObjectIdRole,
                                            QVariant::fromValue(objectId),
                                            1,
                                            Qt::MatchExactly | Qt::MatchRecursive);

    if (indices.isEmpty()) {
        qWarning() << "Somehow couldn't find ID" << objectId;
        return;
    }

    const auto index = m_searchModel->mapFromSource(indices.first());
    treeWidget->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

    // Expand parents of this child index
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        treeWidget->expand(parent);
        parent = parent.parent();
    }

    // Ensure item is in view too!
    treeWidget->scrollTo(index);

    Q_EMIT treeWidget->activated(index);
}

QString SceneListWidget::lookupObjectName(uint32_t objectId)
{
    auto indices = m_objectListModel->match(m_objectListModel->index(0, 0, {}),
                                            SceneListModel::SceneListRoles::ObjectIdRole,
                                            QVariant::fromValue(objectId),
                                            1,
                                            Qt::MatchExactly | Qt::MatchRecursive);

    if (indices.isEmpty()) {
        qWarning() << "Somehow couldn't find ID" << objectId;
        return {};
    }

    return indices.first().data().toString();
}
