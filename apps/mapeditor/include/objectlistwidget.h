// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QListView>
#include <QWidget>
#include <physis.hpp>

class QStringListModel;
class AppState;

class ObjectListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectListWidget(AppState *appState, QWidget *parent = nullptr);

private:
    void refresh();

    QListView *listWidget = nullptr;
    AppState *m_appState;
    QStringListModel *m_originalModel;
};
