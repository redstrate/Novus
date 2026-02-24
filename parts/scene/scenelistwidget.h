// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTreeView>
#include <QWidget>
#include <physis.hpp>

class QSortFilterProxyModel;
class SceneState;
class SceneListModel;
class QStringListModel;

class SceneListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SceneListWidget(SceneState *appState, QWidget *parent = nullptr);

    void expandToDepth(int depth);
    void focusSearchField();
    void selectObject(uint32_t objectId);
    QString lookupObjectName(uint32_t objectId);

private:
    QTreeView *treeWidget = nullptr;
    SceneListModel *m_objectListModel = nullptr;
    SceneState *m_appState = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QSortFilterProxyModel *m_searchModel = nullptr;
};
