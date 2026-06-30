// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <physis.hpp>

#include "filecache.h"
#include "mdlexport.h"
#include "rendermanager.h"

class QSlider;
class MapView;
class SceneListWidget;
class SceneState;
class ObjectPropertiesWidget;
class VulkanWindow;

// Cutoff before nameplates and billboards are drawn.
constexpr float MAX_DEBUG_DRAW_DISTANCE = 50.0f;

class ScenePart : public QWidget
{
    Q_OBJECT

public:
    // TODO: eventually remove fixedSize hack used in map editor
    explicit ScenePart(FileCache &cache, bool fixedSize = false, QWidget *parent = nullptr);
    ~ScenePart() override;

    void loadSgb(physis_Buffer file);
    void loadLvb(physis_Buffer file, int territoryTypeHint = 0, int contentFinderConditionHint = 0);
    void focusSearchField();
    void selectObject(uint32_t objectId);
    QString lookupObjectName(uint32_t objectId);
    void clear();
    void save();

    SceneState *sceneState() const;
    MapView *mapView() const;

private:
    FileCache &m_cache;
    SceneState *m_appState = nullptr;
    SceneListWidget *m_sceneListWidget = nullptr;
    ObjectPropertiesWidget *m_objectPropertiesWidget = nullptr;
    MapView *m_mapView = nullptr;
    QSlider *m_animationTimeSlider = nullptr;
    physis_Sgb m_sgb{};
    physis_Lvb m_lvb{};
};
