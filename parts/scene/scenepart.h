// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <physis.hpp>

#include "mdlexport.h"
#include "rendermanager.h"

class SceneListWidget;
class SceneState;
class ObjectPropertiesWidget;
struct physis_SqPackResource;

class VulkanWindow;

class ScenePart : public QWidget
{
    Q_OBJECT

public:
    explicit ScenePart(physis_SqPackResource *data, QWidget *parent = nullptr);

    void loadSgb(physis_Buffer file);

private:
    SceneState *m_appState = nullptr;
    physis_SqPackResource *m_data = nullptr;
    SceneListWidget *m_sceneListWidget = nullptr;
    ObjectPropertiesWidget *m_objectPropertiesWidget = nullptr;
};
