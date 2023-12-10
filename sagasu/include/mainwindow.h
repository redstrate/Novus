// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMap>
#include <QMdiArea>
#include <QTreeWidget>

#include "filecache.h"
#include "novusmainwindow.h"

struct GameData;

class MainWindow : public NovusMainWindow
{
public:
    MainWindow(const QString &gamePath, GameData *data);

private:
    GameData *data = nullptr;
    QTabWidget *partHolder = nullptr;
    FileCache fileCache;

    void refreshParts(const QString &path);
};