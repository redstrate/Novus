// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QProcess>

#include "novusmainwindow.h"

struct GameData;

class MainWindow : public NovusMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow();

private:
    QProcess *process = nullptr;
};