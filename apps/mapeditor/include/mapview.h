// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "mdlpart.h"
#include <QComboBox>
#include <QWidget>
#include <physis.hpp>

struct GameData;
class AppState;

class MapView : public QWidget
{
    Q_OBJECT

public:
    explicit MapView(SqPackResource *data, FileCache &cache, AppState *appState, QWidget *parent = nullptr);

    MDLPart &part() const;

    void centerOn(glm::vec3 position);

public Q_SLOTS:
    void addTerrain(QString basePath, physis_Terrain terrain);

private:
    void reloadMap();

    MDLPart *mdlPart = nullptr;

    SqPackResource *m_data;
    FileCache &m_cache;
    AppState *m_appState;
};
