// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

#include "filecache.h"

class ObjectListWidget;
struct GameData;
class MapView;
class AppState;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(GameData *data);

private:
    void setupActions();
    void openMap(const QString &basePath);

    GameData *data = nullptr;
    FileCache cache;
    MapView *mapView = nullptr;
    ObjectListWidget *objectListWidget = nullptr;
    AppState *m_appState = nullptr;
};
