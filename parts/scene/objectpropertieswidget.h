// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScrollArea>

struct physis_ScnLayerSet;
struct physis_ColliderLayer9InstanceObject;
struct physis_ColliderLayer7InstanceObject;
struct physis_ColliderLayer10InstanceObject;
struct physis_ColliderLayer8InstanceObject;
struct physis_VolumetricCloudInstanceObject;
struct physis_DecalInstanceObject;
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

class ObjectPropertiesWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit ObjectPropertiesWidget(SceneState *appState, QWidget *parent = nullptr);

private:
    void resetSections();
    void refreshObjectData(physis_InstanceObject &object);
    void refreshLayerData(physis_Layer &layer);
    void refreshTimelineData(const physis_ScnTimeline &timeline);
    void refreshActionData(const ScnSGActionControllerDescriptor &action);
    void refreshLgbData(const QString &path);
    void refreshTeraData(const QString &path);
    void refreshDropInData(DropInObject *object);
    void refreshPlateData(int index);
    void refreshLayerSetData(physis_ScnLayerSet &layerSet);

    void addCommonSection(physis_InstanceObject &object);
    void addBgPartSection(physis_BgPartInstanceObject &bg);
    void addEventObjectSection(physis_EventObjectInstanceObject &eobj);
    void addPopRangeSection(physis_PopRangeInstanceObject &pop);
    void addEventNpcSection(physis_EventNpcInstanceObject &enpc);
    void addMapRangeSection(physis_MapRangeInstanceObject &mapRange);
    void addTriggerBoxSection(physis_TriggerBoxInstanceObject &triggerBox);
    void addCharacterSection(physis_CharacterInstanceObject &character);
    void addGameObjectSection(physis_GameObjectInstanceObject &object);
    void addSharedGroupSection(physis_SharedGroupInstanceObject &sharedGroup);
    void addAetheryteSection(physis_AetheryteInstanceObject &aetheryte);
    void addExitRangeSection(physis_ExitRangeInstanceObject &exitRange);
    void addEventRangeSection(physis_EventRangeInstanceObject &eventRange);
    void addChairMarkerSection(physis_ChairMarkerInstanceObject &chairMarker);
    void addPrefetchRangeSection(physis_PrefetchRangeInstanceObject &prefetchRange);
    void addLightSection(physis_LightInstanceObject &light);
    void addVfxSection(physis_VfxInstanceObject &vfx);
    void addEnvSetSection(physis_EnvSetInstanceObject &envSet);
    void addEnvLocationSection(physis_EnvLocationObject &envLocation);
    void addSoundSection(physis_SoundInstanceObject &sound);
    void addCollisionBoxSection(physis_CollisionBoxInstanceObject &collisionBox);
    void addDoorRangeSection(physis_DoorRangeInstanceObject &doorRange);
    void addLineVFXSection(physis_LineVFXInstanceObject &lineVfx);
    void addTreasureSection(physis_TreasureInstanceObject &treasure);
    void addTargetMarkerSection(physis_TargetMarkerInstanceObject &targetMarker);
    void addClientPathSection(physis_ClientPathInstanceObject &clientPath);
    void addPathSection(physis_PathInstanceObject &path);
    void addRangeSection(physis_RangeInstanceObject &range);
    void addCullingBoxSection(physis_CullingBoxInstanceObject &cullingBox);
    void addClickableRangeSection(physis_ClickableRangeInstanceObject &clickableRange);
    void addBattleNpcSection(physis_BattleNpcInstanceObject &battleNpc);
    void addDecalSection(physis_DecalInstanceObject &decal);
    void addVolumetricCloudSection(physis_VolumetricCloudInstanceObject &cloud);
    void addColliderLayer8Section(physis_ColliderLayer8InstanceObject &collider);
    void addColliderLayer10Section(physis_ColliderLayer10InstanceObject &collider);
    void addColliderLayer7Section(physis_ColliderLayer7InstanceObject &collider);
    void addColliderLayer9Section(physis_ColliderLayer9InstanceObject &collider);
    void addFateRangeSection();

    SceneState *m_appState = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<QWidget *> m_sections;
};
