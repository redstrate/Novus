// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "filecache.h"
#include "mdlpart.h"
#include <QComboBox>
#include <QWidget>
#include <physis.hpp>

class ObjectScene;
struct GameData;
class SceneState;

class MapView : public QWidget
{
    Q_OBJECT

public:
    explicit MapView(physis_SqPackResource *data, FileCache &cache, SceneState *appState, QWidget *parent = nullptr);

    MDLPart &part() const;

    void centerOn(glm::vec3 position);

public Q_SLOTS:
    void addTerrain(QString basePath, physis_Terrain terrain);

private:
    void reloadMap();
    void processScene(const ObjectScene &scene);
    void processLayer(const physis_Layer &layer, const Transformation &rootTransformation);

    MDLPart *mdlPart = nullptr;

    physis_SqPackResource *m_data;
    FileCache &m_cache;
    SceneState *m_appState;
};
