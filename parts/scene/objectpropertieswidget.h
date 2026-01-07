// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

struct physis_PrefetchRangeInstanceObject;
struct physis_ChairMarkerInstanceObject;
struct physis_EventRangeInstanceObject;
struct physis_ExitRangeInstanceObject;
struct physis_AetheryteInstanceObject;
struct physis_SharedGroupInstanceObject;
struct physis_GameInstanceObject;
struct physis_NPCInstanceObject;
struct physis_TriggerBoxInstanceObject;
struct physis_MapRangeInstanceObject;
struct physis_Layer;
struct physis_PopRangeInstanceObject;
struct physis_EventInstanceObject;
class QVBoxLayout;
struct physis_InstanceObject;
struct physis_BGInstanceObject;
class SceneState;
class QLineEdit;
struct physis_ENPCInstanceObject;

class ObjectPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectPropertiesWidget(SceneState *appState, QWidget *parent = nullptr);

private:
    void resetSections();
    void refreshObjectData(const physis_InstanceObject &object);
    void refreshLayerData(const physis_Layer &layer);

    void addCommonSection(const physis_InstanceObject &object);
    void addBGSection(const physis_BGInstanceObject &bg);
    void addEventSection(const physis_EventInstanceObject &eobj);
    void addPopRangeSection(const physis_PopRangeInstanceObject &pop);
    void addEventNPCSection(const physis_ENPCInstanceObject &enpc);
    void addMapRangeSection(const physis_MapRangeInstanceObject &mapRange);
    void addTriggerBoxSection(const physis_TriggerBoxInstanceObject &triggerBox);
    void addNPCSection(const physis_NPCInstanceObject &npc);
    void addGameObjectSection(const physis_GameInstanceObject &object);
    void addSharedGroupSection(const physis_SharedGroupInstanceObject &sharedGroup);
    void addAetheryteSection(const physis_AetheryteInstanceObject &aetheryte);
    void addExitRangeSection(const physis_ExitRangeInstanceObject &exitRange);
    void addEventRangeSection(const physis_EventRangeInstanceObject &eventRange);
    void addChairMarkerSection(const physis_ChairMarkerInstanceObject &chairMarker);
    void addPrefetchRangeSection(const physis_PrefetchRangeInstanceObject &prefetchRange);

    SceneState *m_appState = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<QWidget *> m_sections;
};
