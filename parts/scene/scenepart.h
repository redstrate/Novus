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
struct physis_SqPackResource;
class VulkanWindow;
class Animation;

class ScenePart : public QWidget
{
    Q_OBJECT

public:
    explicit ScenePart(physis_SqPackResource *data, QWidget *parent = nullptr);

    void loadSgb(physis_Buffer file);

    SceneState *sceneState() const;
    MapView *mapView() const;

private:
    SceneState *m_appState = nullptr;
    physis_SqPackResource *m_data = nullptr;
    SceneListWidget *m_sceneListWidget = nullptr;
    ObjectPropertiesWidget *m_objectPropertiesWidget = nullptr;
    MapView *m_mapView = nullptr;
    FileCache m_fileCache;
    Animation *m_animation;
    QSlider *m_animationTimeSlider = nullptr;
};
