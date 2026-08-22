// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

#include "filecache.h"

class CutbPart;
class QLabel;
class ScenePart;
class ObjectPropertiesWidget;
class SceneListWidget;
class MapView;
class SceneState;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(physis_SqPackResource data);
    ~MainWindow() override;

private:
    void setupActions();

    FileCache m_cache;
    CutbPart *m_part = nullptr;
};
