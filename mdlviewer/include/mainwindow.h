// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "novusmainwindow.h"

struct GameData;
class MDLPart;

class MainWindow : public NovusMainWindow
{
    Q_OBJECT

public:
    MainWindow(GameData *data);

protected:
    void setupFileMenu(QMenu *menu) override;

    GameData *data = nullptr;
    MDLPart *part = nullptr;
    FileCache cache;
};