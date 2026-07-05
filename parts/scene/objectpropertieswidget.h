// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

struct physis_BattleNpcInstanceObject;
struct physis_ClickableRangeInstanceObject;
struct physis_CullingBoxInstanceObject;
struct physis_RangeInstanceObject;
struct physis_PathInstanceObject;
struct physis_ClientPathInstanceObject;
struct physis_TargetMarkerInstanceObject;
struct physis_TreasureInstanceObject;
struct physis_LineVFXInstanceObject;
struct physis_DoorRangeInstanceObject;
struct physis_CollisionBoxInstanceObject;
struct physis_SoundInstanceObject;
struct physis_EnvLocationObject;
struct physis_EnvSetInstanceObject;
struct DropInObject;
struct physis_ScnTimeline;
struct ScnSGActionControllerDescriptor;
struct physis_VfxInstanceObject;
struct physis_LightInstanceObject;
struct physis_PrefetchRangeInstanceObject;
struct physis_ChairMarkerInstanceObject;
struct physis_EventRangeInstanceObject;
struct physis_ExitRangeInstanceObject;
struct physis_AetheryteInstanceObject;
struct physis_SharedGroupInstanceObject;
struct physis_GameObjectInstanceObject;
struct physis_CharacterInstanceObject;
struct physis_TriggerBoxInstanceObject;
struct physis_MapRangeInstanceObject;
struct physis_Layer;
struct physis_PopRangeInstanceObject;
struct physis_EventObjectInstanceObject;
class QVBoxLayout;
struct physis_InstanceObject;
struct physis_BgPartInstanceObject;
class SceneState;
class QLineEdit;
struct physis_EventNpcInstanceObject;

class ObjectPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectPropertiesWidget(SceneState *appState, QWidget *parent = nullptr);

private:
    void resetSections();
    void refreshObjectData(physis_InstanceObject &object);
    void refreshLayerData(const physis_Layer &layer);
    void refreshTimelineData(const physis_ScnTimeline &timeline);
    void refreshActionData(const ScnSGActionControllerDescriptor &action);
    void refreshLgbData(const QString &path);
    void refreshTeraData(const QString &path);
    void refreshDropInData(DropInObject *object);

    void addCommonSection(physis_InstanceObject &object);
    void addBgPartSection(const physis_BgPartInstanceObject &bg);
    void addEventObjectSection(physis_EventObjectInstanceObject &eobj);
    void addPopRangeSection(const physis_PopRangeInstanceObject &pop);
    void addEventNpcSection(physis_EventNpcInstanceObject &enpc);
    void addMapRangeSection(physis_MapRangeInstanceObject &mapRange);
    void addTriggerBoxSection(const physis_TriggerBoxInstanceObject &triggerBox);
    void addCharacterSection(physis_CharacterInstanceObject &character);
    void addGameObjectSection(physis_GameObjectInstanceObject &object);
    void addSharedGroupSection(physis_SharedGroupInstanceObject &sharedGroup);
    void addAetheryteSection(physis_AetheryteInstanceObject &aetheryte);
    void addExitRangeSection(const physis_ExitRangeInstanceObject &exitRange);
    void addEventRangeSection(const physis_EventRangeInstanceObject &eventRange);
    void addChairMarkerSection(const physis_ChairMarkerInstanceObject &chairMarker);
    void addPrefetchRangeSection(const physis_PrefetchRangeInstanceObject &prefetchRange);
    void addLightSection(const physis_LightInstanceObject &light);
    void addVfxSection(const physis_VfxInstanceObject &vfx);
    void addEnvSetSection(const physis_EnvSetInstanceObject &envSet);
    void addEnvLocationSection(const physis_EnvLocationObject &envLocation);
    void addSoundSection(const physis_SoundInstanceObject &sound);
    void addCollisionBoxSection(const physis_CollisionBoxInstanceObject &collisionBox);
    void addDoorRangeSection(const physis_DoorRangeInstanceObject &doorRange);
    void addLineVFXSection(const physis_LineVFXInstanceObject &lineVfx);
    void addTreasureSection(physis_TreasureInstanceObject &treasure);
    void addTargetMarkerSection(const physis_TargetMarkerInstanceObject &targetMarker);
    void addClientPathSection(const physis_ClientPathInstanceObject &clientPath);
    void addPathSection(const physis_PathInstanceObject &path);
    void addRangeSection(const physis_RangeInstanceObject &range);
    void addCullingBoxSection(const physis_CullingBoxInstanceObject &cullingBox);
    void addClickableRangeSection(const physis_ClickableRangeInstanceObject &clickableRange);
    void addBattleNpcSection(physis_BattleNpcInstanceObject &battleNpc);

    SceneState *m_appState = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<QWidget *> m_sections;
};
