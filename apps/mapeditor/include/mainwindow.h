// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KXmlGuiWindow>

#include "filecache.h"

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

public Q_SLOTS:
    void configure();

private:
    void setupActions();
    void openMap(const QString &basePath, int territoryType, int contentFinderCondition);
    void updateActionState();

    FileCache m_cache;
    ScenePart *m_part = nullptr;
    QAction *m_centerObjectAction = nullptr;
    QAction *m_goToEntranceAction = nullptr;
    QAction *m_goToExitAction = nullptr;
    QAction *m_gimmickListAction = nullptr;
    QAction *m_effectListAction = nullptr;
    QAction *m_goToObjectAction = nullptr;
    QAction *m_closeAction = nullptr;
    QAction *m_saveAction = nullptr;
    QLabel *m_cameraPosLabel = nullptr;
    uint32_t m_lgbEventRange = 0;
    std::vector<int32_t> m_mapEffects;
};
