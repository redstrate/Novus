// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMainWindow>

struct GameData;

class MainWindow : public QMainWindow
{
public:
    MainWindow(GameData *data);

private:
    GameData *data = nullptr;
};