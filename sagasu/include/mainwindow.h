// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMainWindow>
#include <QMap>
#include <QMdiArea>
#include <QTreeWidget>

struct GameData;

class MainWindow : public QMainWindow
{
public:
    MainWindow(GameData *data);

private:
    QMdiArea *mdiArea = nullptr;

    GameData *data;
};