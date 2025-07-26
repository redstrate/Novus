// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

#include "filecache.h"

class ObjectPropertiesWidget;
class ObjectListWidget;
struct SqPackResource;
class MapView;
class AppState;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SqPackResource *data);

private:
    void setupActions();
    void openMap(const QString &basePath);

    SqPackResource *data = nullptr;
    FileCache cache;
    MapView *mapView = nullptr;
    ObjectListWidget *objectListWidget = nullptr;
    AppState *m_appState = nullptr;
    ObjectPropertiesWidget *objectPropertiesWidget = nullptr;
};
