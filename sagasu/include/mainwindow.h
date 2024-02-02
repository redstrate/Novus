// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMap>
#include <QMdiArea>
#include <QTreeWidget>

#include "filecache.h"
#include "hashdatabase.h"
#include "novusmainwindow.h"

struct GameData;

class MainWindow : public NovusMainWindow
{
public:
    MainWindow(const QString &gamePath, GameData *data);

protected:
    void setupFileMenu(QMenu *menu) override;

private:
    GameData *data = nullptr;
    QTabWidget *partHolder = nullptr;
    FileCache fileCache;
    HashDatabase m_database;

    void refreshParts(const QString &path);
};