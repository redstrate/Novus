// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTreeView>
#include <QWidget>
#include <physis.hpp>

class ObjectListModel;
class QStringListModel;
class AppState;

class ObjectListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectListWidget(AppState *appState, QWidget *parent = nullptr);

private:
    QTreeView *treeWidget = nullptr;
    AppState *m_appState = nullptr;
    ObjectListModel *m_objectListModel = nullptr;
};
