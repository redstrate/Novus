// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QListView>
#include <QWidget>
#include <physis.hpp>

class MapListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapListWidget(GameData *data, QWidget *parent = nullptr);

Q_SIGNALS:
    void mapSelected(const QString &name);

private:
    QListView *listWidget = nullptr;

    GameData *data = nullptr;
};
