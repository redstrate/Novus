// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectpropertieswidget.h"

#include "booledit.h"
#include "collapsesection.h"
#include "pathedit.h"
#include "scenestate.h"
#include "vec3edit.h"

#include <KLocalizedString>
#include <QCheckBox>

#include <QFormLayout>
#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>

#include <glm/gtc/type_ptr.hpp>

#include "enumedit.h"
#include "exceledit.h"
#include "objectidedit.h"
#include "uintedit.h"
#include "utility.h"

#include <QLabel>

ObjectPropertiesWidget::ObjectPropertiesWidget(SceneState *appState, QWidget *parent)
    : QWidget(parent)
    , m_appState(appState)
{
    m_layout = new QVBoxLayout();
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    connect(appState, &SceneState::selectionChanged, this, [this] {
        resetSections();
        if (m_appState->selectedObject) {
            refreshObjectData(*m_appState->selectedObject.value());
        }
        if (m_appState->selectedLayer) {
            refreshLayerData(*m_appState->selectedLayer.value());
        }
        if (m_appState->selectedTimeline) {
            refreshTimelineData(*m_appState->selectedTimeline.value());
        }
        if (m_appState->selectedAction) {
            refreshActionData(*m_appState->selectedAction.value());
        }
        if (m_appState->selectedLgb) {
            refreshLgbData(m_appState->selectedLgb.value());
        }
        if (m_appState->selectedTera) {
            refreshTeraData(m_appState->selectedTera.value());
        }
        if (m_appState->selectedDropInObject) {
            refreshDropInData(m_appState->selectedDropInObject.value());
        }
    });
}

void ObjectPropertiesWidget::resetSections()
{
    for (const auto section : m_sections) {
        m_layout->removeWidget(section);
        delete section;
    }
    m_sections.clear();
}

void ObjectPropertiesWidget::refreshObjectData(physis_InstanceObject &object)
{
    addCommonSection(object);

    switch (object.data.tag) {
    case physis_LayerEntry::Tag::BgPart:
        addBgPartSection(object.data.bg_part._0);
        break;
    case physis_LayerEntry::Tag::Light:
        addLightSection(object.data.light._0);
        break;
    case physis_LayerEntry::Tag::Vfx:
        addVfxSection(object.data.vfx._0);
        break;
    case physis_LayerEntry::Tag::EventObject:
        addEventObjectSection(object.data.event_object._0);
        break;
    case physis_LayerEntry::Tag::PopRange:
        addPopRangeSection(object.data.pop_range._0);
        break;
    case physis_LayerEntry::Tag::EventNpc:
        addEventNpcSection(object.data.event_npc._0);
        break;
    case physis_LayerEntry::Tag::MapRange:
        addMapRangeSection(object.data.map_range._0);
        break;
    case physis_LayerEntry::Tag::SharedGroup:
        addSharedGroupSection(object.data.shared_group._0);
        break;
    case physis_LayerEntry::Tag::Aetheryte:
        addAetheryteSection(object.data.aetheryte._0);
        break;
    case physis_LayerEntry::Tag::ExitRange:
        addExitRangeSection(object.data.exit_range._0);
        break;
    case physis_LayerEntry::Tag::EventRange:
        addEventRangeSection(object.data.event_range._0);
        break;
    case physis_LayerEntry::Tag::ChairMarker:
        addChairMarkerSection(object.data.chair_marker._0);
        break;
    case physis_LayerEntry::Tag::PrefetchRange:
        addPrefetchRangeSection(object.data.prefetch_range._0);
        break;
    case physis_LayerEntry::Tag::EnvSet:
        addEnvSetSection(object.data.env_set._0);
        break;
    case physis_LayerEntry::Tag::EnvLocation:
        addEnvLocationSection(object.data.env_location._0);
        break;
    case physis_LayerEntry::Tag::Sound:
        addSoundSection(object.data.sound._0);
        break;
    case physis_LayerEntry::Tag::CollisionBox:
        addCollisionBoxSection(object.data.collision_box._0);
        break;
    case physis_LayerEntry::Tag::DoorRange:
        addDoorRangeSection(object.data.door_range._0);
        break;
    case physis_LayerEntry::Tag::LineVFX:
        addLineVFXSection(object.data.line_vfx._0);
        break;
    case physis_LayerEntry::Tag::Treasure:
        addTreasureSection(object.data.treasure._0);
        break;
    case physis_LayerEntry::Tag::TargetMarker:
        addTargetMarkerSection(object.data.target_marker._0);
        break;
    case physis_LayerEntry::Tag::CullingBox:
        addCullingBoxSection(object.data.culling_box._0);
        break;
    case physis_LayerEntry::Tag::ClickableRange:
        addClickableRangeSection(object.data.clickable_range._0);
        break;
    case physis_LayerEntry::Tag::BattleNpc:
        addBattleNpcSection(object.data.battle_npc._0);
        break;
    default:
        break;
    }
}

void ObjectPropertiesWidget::refreshLayerData(const physis_Layer &layer)
{
    const auto section = new CollapseSection(i18n("Layer"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto idEdit = new QLineEdit();
    idEdit->setText(QString::number(layer.id));
    idEdit->setReadOnly(true);
    layout->addRow(i18n("ID"), idEdit);

    const auto nameEdit = new QLineEdit();
    nameEdit->setText(QString::fromLatin1(layer.name));
    nameEdit->setReadOnly(true);
    layout->addRow(i18n("Name"), nameEdit);

    const auto festivalIdEdit = new QLineEdit();
    festivalIdEdit->setText(QString::number(layer.festival_id));
    festivalIdEdit->setReadOnly(true);
    layout->addRow(i18n("Festival ID"), festivalIdEdit);

    const auto festivalPhaseEdit = new QLineEdit();
    festivalPhaseEdit->setText(QString::number(layer.festival_phase_id));
    festivalPhaseEdit->setReadOnly(true);
    layout->addRow(i18n("Festival Phase"), festivalPhaseEdit);
}

void ObjectPropertiesWidget::refreshTimelineData(const physis_ScnTimeline &timeline)
{
    const auto section = new CollapseSection(i18n("Timeline"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto instanceWidget = new QTableWidget();
    instanceWidget->setColumnCount(2);
    instanceWidget->setRowCount(timeline.instance_count);
    instanceWidget->setHorizontalHeaderLabels({i18n("Instance ID"), i18n("TMAC Time")});
    layout->addWidget(instanceWidget);

    for (uint32_t i = 0; i < timeline.instance_count; i++) {
        const auto instance = timeline.instances[i];

        instanceWidget->setItem(i, 0, new QTableWidgetItem(QString::number(instance.instance_id)));
        instanceWidget->setItem(i, 1, new QTableWidgetItem(QString::number(instance.tmac_time)));
    }
}

void ObjectPropertiesWidget::refreshActionData(const ScnSGActionControllerDescriptor &action)
{
    const auto section = new CollapseSection(i18n("Action"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto typeEdit = new EnumEdit<ScnSGActionControllerDescriptor::Tag>();
    typeEdit->setValue(action.tag);
    typeEdit->setEnabled(false);
    layout->addRow(i18n("Type"), typeEdit);

    switch (action.tag) {
    case ScnSGActionControllerDescriptor::Tag::Door: {
        const auto &door = action.door._0;

        const auto doorObjectId0Edit = new ObjectIdEdit(m_appState);
        doorObjectId0Edit->setObjectId(door.door_object_0);
        layout->addRow(i18n("Door Object 0"), doorObjectId0Edit);

        const auto doorObjectId1Edit = new ObjectIdEdit(m_appState);
        doorObjectId1Edit->setObjectId(door.door_object_1);
        layout->addRow(i18n("Doro Object 1"), doorObjectId1Edit);

        const auto doorTypeEdit = new QLineEdit();
        doorTypeEdit->setText(QString::number(door.door_type));
        doorTypeEdit->setReadOnly(true);
        layout->addRow(i18n("Door Type"), doorTypeEdit);

        const auto maxRotationEdit = new QLineEdit();
        maxRotationEdit->setText(QString::number(door.max_rotation));
        maxRotationEdit->setReadOnly(true);
        layout->addRow(i18n("Max Rotation"), maxRotationEdit);

        const auto maxTranslationEdit = new QLineEdit();
        maxTranslationEdit->setText(QString::number(door.max_translation));
        maxTranslationEdit->setReadOnly(true);
        layout->addRow(i18n("Max Translation"), maxTranslationEdit);

        const auto sound0IdEdit = new ObjectIdEdit(m_appState);
        sound0IdEdit->setObjectId(door.sound_0);
        layout->addRow(i18n("Sound Id 0"), sound0IdEdit);

        const auto sound1IdEdit = new ObjectIdEdit(m_appState);
        sound1IdEdit->setObjectId(door.sound_1);
        layout->addRow(i18n("Sound Id 1"), sound1IdEdit);

        const auto doorObjectId2Edit = new ObjectIdEdit(m_appState);
        doorObjectId2Edit->setObjectId(door.door_object_2);
        layout->addRow(i18n("Door Object 2"), doorObjectId2Edit);

        const auto doorObjectId3Edit = new ObjectIdEdit(m_appState);
        doorObjectId3Edit->setObjectId(door.door_object_3);
        layout->addRow(i18n("Door Object 3"), doorObjectId3Edit);
    } break;
    case ScnSGActionControllerDescriptor::Tag::Rotation: {
        const auto &rotation = action.rotation._0;

        const auto bgPartIdEdit = new ObjectIdEdit(m_appState);
        bgPartIdEdit->setObjectId(rotation.bg_part_id);
        layout->addRow(i18n("BG Part ID"), bgPartIdEdit);

        const auto vfxChildId1Edit = new ObjectIdEdit(m_appState);
        vfxChildId1Edit->setObjectId(rotation.vfx_child1_id);
        layout->addRow(i18n("VFX Child 2 ID"), vfxChildId1Edit);

        const auto vfxChildId2Edit = new ObjectIdEdit(m_appState);
        vfxChildId2Edit->setObjectId(rotation.vfx_child_2_id);
        layout->addRow(i18n("VFX Child 1 ID"), vfxChildId2Edit);

        const auto rotationAxisEdit = new EnumEdit<RotationAxis>();
        rotationAxisEdit->setValue(rotation.axis);
        rotationAxisEdit->setEnabled(false);
        layout->addRow(i18n("Axis"), rotationAxisEdit);

        const auto durationEdit = new QLineEdit();
        durationEdit->setText(QString::number(rotation.duration));
        durationEdit->setReadOnly(true);
        layout->addRow(i18n("Duration"), durationEdit);

        const auto valueEdit = new QLineEdit();
        valueEdit->setText(QString::number(rotation.value));
        valueEdit->setReadOnly(true);
        layout->addRow(i18n("Value"), valueEdit);
    } break;
    case ScnSGActionControllerDescriptor::Tag::Unknown:
        break;
    }
}

void ObjectPropertiesWidget::refreshLgbData(const QString &path)
{
    const auto section = new CollapseSection(i18n("LGB"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto pathWidget = new PathEdit();
    pathWidget->setPath(path);
    layout->addRow(i18n("Path"), pathWidget);
}

void ObjectPropertiesWidget::refreshTeraData(const QString &path)
{
    const auto section = new CollapseSection(i18n("Terrain"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto pathWidget = new PathEdit();
    pathWidget->setPath(path);
    layout->addRow(i18n("Path"), pathWidget);
}

void ObjectPropertiesWidget::refreshDropInData(DropInObject *object)
{
    const auto section = new CollapseSection(i18n("Drop-In Object"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto positionEdit = new Vector3Edit(object->position);
    layout->addRow(i18n("Position"), positionEdit);

    const auto idEdit = new UIntEdit(object->instanceId);
    layout->addRow(i18n("Instance ID"), idEdit);

    if (const auto data = std::get_if<DropInGatheringPoint>(&object->data)) {
        const auto baseIdEdit = new ExcelEdit(m_appState, {QStringLiteral("GatheringPoint")}, data->baseId);
        layout->addRow(i18n("Base ID"), baseIdEdit);
    } else if (auto data = std::get_if<DropInBattleNpc>(&object->data)) {
        const auto baseIdEdit = new ExcelEdit(m_appState, {QStringLiteral("BNpcBase")}, data->baseId);
        layout->addRow(i18n("Base ID"), baseIdEdit);

        const auto nameIdEdit = new ExcelEdit(m_appState, {QStringLiteral("BNpcName")}, data->nameId);
        layout->addRow(i18n("Name ID"), nameIdEdit);

        const auto hpEdit = new UIntEdit(data->hp);
        layout->addRow(i18n("HP"), hpEdit);

        const auto levelEdit = new UIntEdit(data->level);
        layout->addRow(i18n("Level"), levelEdit);

        const auto nonPopEdit = new BoolEdit(data->nonpop);
        layout->addRow(i18n("Nonpop"), nonPopEdit);

        const auto aggressionModeEdit = new UIntEdit(data->aggressionMode);
        layout->addRow(i18n("Aggression Mode"), aggressionModeEdit);

        const auto gimmickId = new ObjectIdEdit(m_appState);
        gimmickId->setObjectId(data->gimmickId);
        layout->addRow(i18n("Gimmick Id"), gimmickId);

        const auto maxLinksEdit = new UIntEdit(data->maxLinks);
        layout->addRow(i18n("Max Links"), maxLinksEdit);

        const auto linkFamilyEdit = new UIntEdit(data->linkFamily);
        layout->addRow(i18n("Link Family"), linkFamilyEdit);

        const auto linkRangeEdit = new UIntEdit(data->linkRange);
        layout->addRow(i18n("Link Range"), linkRangeEdit);
    }
}

void ObjectPropertiesWidget::addCommonSection(physis_InstanceObject &object)
{
    const auto section = new CollapseSection(i18n("Common"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto positionEdit = new Vector3Edit(reinterpret_cast<glm::vec3 &>(object.transform.translation));
    layout->addRow(i18n("Position"), positionEdit);

    const auto rotationEdit = new Vector3Edit(reinterpret_cast<glm::vec3 &>(object.transform.rotation));
    layout->addRow(i18n("Rotation"), rotationEdit);

    const auto scaleEdit = new Vector3Edit(reinterpret_cast<glm::vec3 &>(object.transform.scale));
    layout->addRow(i18n("Scale"), scaleEdit);

    const auto idEdit = new QLineEdit();
    idEdit->setText(QString::number(object.instance_id));
    idEdit->setReadOnly(true);
    layout->addRow(i18n("Instance ID"), idEdit);
}

void ObjectPropertiesWidget::addBgPartSection(const physis_BgPartInstanceObject &bg)
{
    const auto section = new CollapseSection(i18n("BG"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto bgEdit = new PathEdit();
    bgEdit->setPath(QString::fromLatin1(bg.asset_path));
    bgEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), bgEdit);

    const auto collisionEdit = new PathEdit();
    collisionEdit->setPath(QString::fromLatin1(bg.collision_asset_path));
    collisionEdit->setReadOnly(true);
    layout->addRow(i18n("Collision Asset Path"), collisionEdit);

    const auto collisionTypeEdit = new EnumEdit<ModelCollisionType>();
    collisionTypeEdit->setValue(bg.collision_type);
    collisionTypeEdit->setEnabled(false);
    layout->addRow(i18n("Collision Type"), collisionTypeEdit);

    const auto visibleCheck = new QCheckBox();
    visibleCheck->setChecked(bg.visible);
    layout->addRow(i18n("Visible"), visibleCheck);

    const auto worldShadowModeEdit = new EnumEdit<ShadowMode>();
    worldShadowModeEdit->setValue(bg.world_light_shadow_mode);
    worldShadowModeEdit->setEnabled(false);
    layout->addRow(i18n("World Light Shadows"), worldShadowModeEdit);

    const auto objectShadowModeEdit = new EnumEdit<ShadowMode>();
    objectShadowModeEdit->setValue(bg.object_light_shadow_mode);
    objectShadowModeEdit->setEnabled(false);
    layout->addRow(i18n("Object Light Shadows"), objectShadowModeEdit);

    const auto fadeOutDistanceLabel = new QLineEdit();
    fadeOutDistanceLabel->setText(QString::number(bg.fade_out_distance));
    fadeOutDistanceLabel->setReadOnly(true);
    layout->addRow(i18n("Fade out distance"), fadeOutDistanceLabel);

    const auto boundingSphereSizeLabel = new QLineEdit();
    boundingSphereSizeLabel->setText(QString::number(bg.fade_out_distance));
    boundingSphereSizeLabel->setReadOnly(true);
    layout->addRow(i18n("Bounding sphere size"), boundingSphereSizeLabel);
}

void ObjectPropertiesWidget::addEventObjectSection(physis_EventObjectInstanceObject &eobj)
{
    addGameObjectSection(eobj.parent_data);

    const auto section = new CollapseSection(i18n("Event Object"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto boundIdEdit = new ObjectIdEdit(m_appState);
    boundIdEdit->setObjectId(eobj.bound_instance_id);
    layout->addRow(i18n("Bound ID"), boundIdEdit);
}

void ObjectPropertiesWidget::addPopRangeSection(const physis_PopRangeInstanceObject &pop)
{
    const auto section = new CollapseSection(i18n("Pop Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto typeEdit = new EnumEdit<PopType>();
    typeEdit->setValue(pop.pop_type);
    typeEdit->setEnabled(false);
    layout->addRow(i18n("Type"), typeEdit);

    const auto innerRadiusRatioEdit = new QLineEdit();
    innerRadiusRatioEdit->setText(QString::number(pop.inner_radius_ratio));
    innerRadiusRatioEdit->setReadOnly(true);
    layout->addRow(i18n("Inner Radius Ratio"), innerRadiusRatioEdit);
}

void ObjectPropertiesWidget::addEventNpcSection(physis_EventNpcInstanceObject &enpc)
{
    addCharacterSection(enpc.parent_data);

    const auto section = new CollapseSection(i18n("Event NPC"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addMapRangeSection(physis_MapRangeInstanceObject &mapRange)
{
    addTriggerBoxSection(mapRange.parent_data);

    const auto section = new CollapseSection(i18n("Map Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto mapEdit = new ExcelEdit(m_appState, {QStringLiteral("Map")}, mapRange.map);
    mapEdit->setReadOnly(true);
    layout->addRow(i18n("Map"), mapEdit);

    const auto placeNameBlock = new ExcelEdit(m_appState, {QStringLiteral("PlaceName")}, mapRange.place_name_block);
    placeNameBlock->setReadOnly(true);
    layout->addRow(i18n("PlaceName Block"), placeNameBlock);

    const auto placeNameSpot = new ExcelEdit(m_appState, {QStringLiteral("PlaceName")}, mapRange.place_name_spot);
    placeNameSpot->setReadOnly(true);
    layout->addRow(i18n("PlaceName Spot"), placeNameSpot);

    const auto weatherEdit = new QLineEdit();
    weatherEdit->setReadOnly(true);
    weatherEdit->setText(QString::number(mapRange.weather));
    layout->addRow(i18n("Weather"), weatherEdit);

    const auto bgmEdit = new QLineEdit();
    bgmEdit->setReadOnly(true);
    bgmEdit->setText(QString::number(mapRange.bgm));
    layout->addRow(i18n("BGM"), bgmEdit);

    const auto unk1Edit = new QLineEdit();
    unk1Edit->setReadOnly(true);
    unk1Edit->setText(QString::number(mapRange.unk1));
    layout->addRow(i18n("UNK1"), unk1Edit);

    const auto unk2Edit = new QLineEdit();
    unk2Edit->setReadOnly(true);
    unk2Edit->setText(QString::number(mapRange.unk2));
    layout->addRow(i18n("UNK2"), unk2Edit);

    const auto housingBlockIdEdit = new QLineEdit();
    housingBlockIdEdit->setReadOnly(true);
    housingBlockIdEdit->setText(QString::number(mapRange.housing_block_id));
    layout->addRow(i18n("Housing block ID"), housingBlockIdEdit);

    const auto restBonusEffectiveCheckbox = new QCheckBox();
    restBonusEffectiveCheckbox->setChecked(mapRange.rest_bonus_effective);
    restBonusEffectiveCheckbox->setEnabled(false);
    layout->addRow(i18n("Rest Bonus Effective"), restBonusEffectiveCheckbox);

    const auto discoveryIdEdit = new QLineEdit();
    discoveryIdEdit->setReadOnly(true);
    discoveryIdEdit->setText(QString::number(mapRange.discovery_id));
    layout->addRow(i18n("Discovery ID"), discoveryIdEdit);

    const auto mapEnabledCheckbox = new QCheckBox();
    mapEnabledCheckbox->setChecked(mapRange.map_enabled);
    mapEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Map Enabled"), mapEnabledCheckbox);

    const auto placeNameEnabledCheckbox = new QCheckBox();
    placeNameEnabledCheckbox->setChecked(mapRange.place_name_enabled);
    placeNameEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Place Name Enabled"), placeNameEnabledCheckbox);

    const auto discoveryEnabledCheckbox = new QCheckBox();
    discoveryEnabledCheckbox->setChecked(mapRange.discovery_enabled);
    discoveryEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Discovery Enabled"), discoveryEnabledCheckbox);

    const auto bgmEnabledCheckbox = new QCheckBox();
    bgmEnabledCheckbox->setChecked(mapRange.bgm_enabled);
    bgmEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("BGM Enabled"), bgmEnabledCheckbox);

    const auto weatherEnabled = new QCheckBox();
    weatherEnabled->setChecked(mapRange.weather_enabled);
    weatherEnabled->setEnabled(false);
    layout->addRow(i18n("Weather Enabled"), weatherEnabled);

    const auto restBonusEnabledCheckbox = new QCheckBox();
    restBonusEnabledCheckbox->setChecked(mapRange.rest_bonus_enabled);
    restBonusEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Rest Bonus Enabled"), restBonusEnabledCheckbox);

    const auto bgmPlayZoneInEnabled = new QCheckBox();
    bgmPlayZoneInEnabled->setChecked(mapRange.bgm_play_zone_in_only);
    bgmPlayZoneInEnabled->setEnabled(false);
    layout->addRow(i18n("BGM Play Zone In Enabled"), bgmPlayZoneInEnabled);

    const auto liftEnabledCheckbox = new QCheckBox();
    liftEnabledCheckbox->setChecked(mapRange.lift_enabled);
    liftEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Lift Enabled"), liftEnabledCheckbox);

    const auto housingEnabledCheckbox = new QCheckBox();
    housingEnabledCheckbox->setChecked(mapRange.housing_enabled);
    housingEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Housing Enabled"), housingEnabledCheckbox);

    const auto unk3Checkbox = new QCheckBox();
    unk3Checkbox->setChecked(mapRange.log_flying_height_max_err);
    unk3Checkbox->setEnabled(false);
    layout->addRow(i18n("Log Flying Height Max Err"), unk3Checkbox);

    const auto unk4Checkbox = new QCheckBox();
    unk4Checkbox->setChecked(mapRange.unk4);
    unk4Checkbox->setEnabled(false);
    layout->addRow(i18n("UNK4"), unk4Checkbox);

    const auto mountsAndOrnamentsDisabledCheckbox = new QCheckBox();
    mountsAndOrnamentsDisabledCheckbox->setChecked(mapRange.mounts_and_ornaments_disabled);
    mountsAndOrnamentsDisabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Mounts and Ornaments Disabled"), mountsAndOrnamentsDisabledCheckbox);

    const auto lalafellsOnlyCheckbox = new QCheckBox();
    lalafellsOnlyCheckbox->setChecked(mapRange.lalafells_only);
    lalafellsOnlyCheckbox->setEnabled(false);
    layout->addRow(i18n("Lalafells Only"), lalafellsOnlyCheckbox);
}

void ObjectPropertiesWidget::addTriggerBoxSection(const physis_TriggerBoxInstanceObject &triggerBox)
{
    const auto section = new CollapseSection(i18n("Trigger Box"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto shapeEdit = new EnumEdit<TriggerBoxShape>();
    shapeEdit->setValue(triggerBox.trigger_box_shape);
    shapeEdit->setEnabled(false);
    layout->addRow(i18n("Shape"), shapeEdit);

    const auto priorityEdit = new QLineEdit();
    priorityEdit->setText(QString::number(triggerBox.priority));
    priorityEdit->setReadOnly(true);
    layout->addRow(i18n("Priority"), priorityEdit);

    const auto enabledCheckBox = new QCheckBox();
    enabledCheckBox->setChecked(triggerBox.enabled);
    enabledCheckBox->setEnabled(false);
    layout->addRow(i18n("Enabled"), enabledCheckBox);
}

void ObjectPropertiesWidget::addCharacterSection(physis_CharacterInstanceObject &character)
{
    addGameObjectSection(character.parent_data);

    const auto section = new CollapseSection(i18n("Character"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addGameObjectSection(physis_GameObjectInstanceObject &object)
{
    const auto section = new CollapseSection(i18n("Game Object"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto baseIdEdit = new ExcelEdit(m_appState,
                                          {QStringLiteral("ENpcResident"), QStringLiteral("ENpcBase"), QStringLiteral("EObj"), QStringLiteral("Treasure")},
                                          object.base_id);
    layout->addRow(i18n("Base ID"), baseIdEdit);
}

void ObjectPropertiesWidget::addSharedGroupSection(physis_SharedGroupInstanceObject &sharedGroup)
{
    const auto section = new CollapseSection(i18n("Shared Group"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto assetPathEdit = new PathEdit();
    assetPathEdit->setPath(QString::fromLatin1(sharedGroup.asset_path));
    connect(assetPathEdit, &PathEdit::editingFinished, this, [assetPathEdit, &sharedGroup] {
        sharedGroup.asset_path = toCString(assetPathEdit->path());
    });
    layout->addRow(i18n("Asset Path"), assetPathEdit);
}

void ObjectPropertiesWidget::addAetheryteSection(physis_AetheryteInstanceObject &aetheryte)
{
    addGameObjectSection(aetheryte.parent_data);

    const auto section = new CollapseSection(i18n("Aetheryte"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto boundInstanceIdEdit = new ObjectIdEdit(m_appState);
    boundInstanceIdEdit->setObjectId(aetheryte.bound_instance_id);
    layout->addRow(i18n("Bound Instance ID"), boundInstanceIdEdit);
}

void ObjectPropertiesWidget::addExitRangeSection(const physis_ExitRangeInstanceObject &exitRange)
{
    addTriggerBoxSection(exitRange.parent_data);

    const auto section = new CollapseSection(i18n("Exit Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto exitTypeEdit = new EnumEdit<ExitType>();
    exitTypeEdit->setValue(exitRange.exit_type);
    exitTypeEdit->setEnabled(false);
    layout->addRow(i18n("Exit Type"), exitTypeEdit);

    const auto zoneIdEdit = new QLineEdit();
    zoneIdEdit->setText(QString::number(exitRange.zone_id));
    zoneIdEdit->setReadOnly(true);
    layout->addRow(i18n("Zone ID"), zoneIdEdit);

    const auto territoryTypeEdit = new QLineEdit();
    territoryTypeEdit->setText(QString::number(exitRange.territory_type));
    territoryTypeEdit->setReadOnly(true);
    layout->addRow(i18n("Territory Type"), territoryTypeEdit);

    const auto destinationInstanceIdEdit = new ObjectIdEdit(m_appState);
    destinationInstanceIdEdit->setObjectId(exitRange.destination_instance_id);
    layout->addRow(i18n("Destination Instance ID"), destinationInstanceIdEdit);

    const auto returnInstanceIdEdit = new ObjectIdEdit(m_appState);
    returnInstanceIdEdit->setObjectId(exitRange.return_instance_id);
    layout->addRow(i18n("Return Instance ID"), returnInstanceIdEdit);
}

void ObjectPropertiesWidget::addEventRangeSection(const physis_EventRangeInstanceObject &eventRange)
{
    addTriggerBoxSection(eventRange.parent_data);

    const auto section = new CollapseSection(i18n("Event Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addChairMarkerSection(const physis_ChairMarkerInstanceObject &chairMarker)
{
    const auto section = new CollapseSection(i18n("Chair Marker"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto leftEnableCheckBox = new QCheckBox();
    leftEnableCheckBox->setChecked(chairMarker.left_enable);
    leftEnableCheckBox->setEnabled(false);
    layout->addRow(i18n("Left Enable"), leftEnableCheckBox);

    const auto rightEnableCheckBox = new QCheckBox();
    rightEnableCheckBox->setChecked(chairMarker.right_enable);
    rightEnableCheckBox->setEnabled(false);
    layout->addRow(i18n("Right Enable"), rightEnableCheckBox);

    const auto backEnableCheckBox = new QCheckBox();
    backEnableCheckBox->setChecked(chairMarker.back_enable);
    backEnableCheckBox->setEnabled(false);
    layout->addRow(i18n("Back Enable"), backEnableCheckBox);

    const auto chairTypeEdit = new EnumEdit<ChairType>();
    chairTypeEdit->setValue(chairMarker.chair_type);
    chairTypeEdit->setEnabled(false);
    layout->addRow(i18n("Chair Type"), chairTypeEdit);
}

void ObjectPropertiesWidget::addPrefetchRangeSection(const physis_PrefetchRangeInstanceObject &prefetchRange)
{
    addTriggerBoxSection(prefetchRange.parent_data);

    const auto section = new CollapseSection(i18n("Prefetch Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto boundInstanceIdEdit = new ObjectIdEdit(m_appState);
    boundInstanceIdEdit->setObjectId(prefetchRange.bound_instance_id);
    layout->addRow(i18n("Bound Instance ID"), boundInstanceIdEdit);
}

void ObjectPropertiesWidget::addLightSection(const physis_LightInstanceObject &light)
{
    const auto section = new CollapseSection(i18n("Light"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto shapeEdit = new EnumEdit<LightShape>();
    shapeEdit->setValue(light.shape);
    shapeEdit->setEnabled(false);
    layout->addRow(i18n("Shape"), shapeEdit);

    const auto attenuationLabel = new QLabel();
    attenuationLabel->setText(QString::number(light.range));
    layout->addRow(i18n("Attenuation"), attenuationLabel);

    const auto rangeLabel = new QLabel();
    rangeLabel->setText(QString::number(light.range));
    layout->addRow(i18n("Range"), rangeLabel);

    const auto coefficientLabel = new QLabel();
    coefficientLabel->setText(QString::number(light.attenuation_cone_coefficient));
    layout->addRow(i18n("Cone Coefficient"), coefficientLabel);

    const auto spotAngleLabel = new QLabel();
    spotAngleLabel->setText(QString::number(light.spot_angle));
    layout->addRow(i18n("Spot Angle"), spotAngleLabel);

    const auto texturePathEdit = new PathEdit();
    texturePathEdit->setPath(QString::fromStdString(light.texture_path));
    layout->addRow(i18n("Texture Path"), texturePathEdit);

    const auto colorLabel = new QLabel();
    colorLabel->setText(QStringLiteral("%1 %2 %3 %4").arg(light.color.red).arg(light.color.green).arg(light.color.blue).arg(light.color.intensity));
    layout->addRow(i18n("Color"), colorLabel);

    const auto specularHighlightsCheck = new QCheckBox();
    specularHighlightsCheck->setChecked(light.enable_specular_highlights);
    layout->addRow(i18n("Specular Highlights"), specularHighlightsCheck);

    const auto bgPartsShadowsCheck = new QCheckBox();
    bgPartsShadowsCheck->setChecked(light.enable_bg_parts_shadows);
    layout->addRow(i18n("BgPart Shadows"), bgPartsShadowsCheck);

    const auto characterShadowsCheck = new QCheckBox();
    characterShadowsCheck->setChecked(light.enable_character_shadows);
    layout->addRow(i18n("Character Shadows"), characterShadowsCheck);

    const auto shadowPlaneNearLabel = new QLabel();
    shadowPlaneNearLabel->setText(QString::number(light.shadow_plane_near));
    layout->addRow(i18n("Shadow Plane Near"), shadowPlaneNearLabel);

    const auto flatSkewAngleLabel = new QLabel();
    flatSkewAngleLabel->setText(QStringLiteral("%1 %2").arg(light.flat_light_skew_angle[0]).arg(light.flat_light_skew_angle[1]));
    layout->addRow(i18n("Flat Skew Angle"), flatSkewAngleLabel);
}

void ObjectPropertiesWidget::addVfxSection(const physis_VfxInstanceObject &vfx)
{
    const auto section = new CollapseSection(i18n("VFX"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto assetPathEdit = new PathEdit();
    assetPathEdit->setPath(QString::fromLatin1(vfx.asset_path));
    assetPathEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), assetPathEdit);
}

void ObjectPropertiesWidget::addEnvSetSection(const physis_EnvSetInstanceObject &envSet)
{
    const auto section = new CollapseSection(i18n("Env Set"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto assetPathEdit = new PathEdit();
    assetPathEdit->setPath(QString::fromLatin1(envSet.asset_path));
    assetPathEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), assetPathEdit);

    const auto boundInstanceIdEdit = new ObjectIdEdit(m_appState);
    boundInstanceIdEdit->setObjectId(envSet.bound_instance_id);
    layout->addRow(i18n("Bound Instance ID"), boundInstanceIdEdit);

    const auto shapeEdit = new EnumEdit<EnvSetShape>();
    shapeEdit->setValue(envSet.shape);
    shapeEdit->setEnabled(false);
    layout->addRow(i18n("Shape"), shapeEdit);

    const auto isEnvMapShootingPoint = new QCheckBox();
    isEnvMapShootingPoint->setChecked(envSet.is_env_map_shooting_point);
    isEnvMapShootingPoint->setEnabled(false);
    layout->addRow(i18n("Is Shooting Point"), isEnvMapShootingPoint);

    const auto priorityEdit = new QLineEdit();
    priorityEdit->setText(QString::number(envSet.priority));
    priorityEdit->setReadOnly(true);
    layout->addRow(i18n("Priority"), priorityEdit);

    const auto effectiveRangeEdit = new QLineEdit();
    effectiveRangeEdit->setText(QString::number(envSet.effective_range));
    effectiveRangeEdit->setReadOnly(true);
    layout->addRow(i18n("Effective Range"), effectiveRangeEdit);

    const auto interpolationTime = new QLineEdit();
    interpolationTime->setText(QString::number(envSet.interpolation_time));
    interpolationTime->setReadOnly(true);
    layout->addRow(i18n("Interpolation Time"), interpolationTime);

    const auto reverbEdit = new QLineEdit();
    reverbEdit->setText(QString::number(envSet.reverb));
    reverbEdit->setReadOnly(true);
    layout->addRow(i18n("Reverb"), reverbEdit);

    const auto filterEdit = new QLineEdit();
    filterEdit->setText(QString::number(envSet.filter));
    filterEdit->setReadOnly(true);
    layout->addRow(i18n("Filter"), filterEdit);

    const auto soundAssetPath = new PathEdit();
    soundAssetPath->setPath(QString::fromLatin1(envSet.sound_asset_path));
    soundAssetPath->setReadOnly(true);
    layout->addRow(i18n("Sound Asset Path"), soundAssetPath);
}

void ObjectPropertiesWidget::addEnvLocationSection(const physis_EnvLocationObject &envLocation)
{
    const auto section = new CollapseSection(i18n("Env Location"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto ambientLightAssetPath = new PathEdit();
    ambientLightAssetPath->setPath(QString::fromLatin1(envLocation.ambient_light_asset_path));
    ambientLightAssetPath->setReadOnly(true);
    layout->addRow(i18n("Ambient Light Asset Path"), ambientLightAssetPath);

    const auto envMapAssetPath = new PathEdit();
    envMapAssetPath->setPath(QString::fromLatin1(envLocation.env_map_asset_path));
    envMapAssetPath->setReadOnly(true);
    layout->addRow(i18n("Env Map Asset Path"), envMapAssetPath);
}

void ObjectPropertiesWidget::addSoundSection(const physis_SoundInstanceObject &sound)
{
    const auto section = new CollapseSection(i18n("Sound"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto soundAssetPathEdit = new PathEdit();
    soundAssetPathEdit->setPath(QString::fromLatin1(sound.asset_path));
    soundAssetPathEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), soundAssetPathEdit);
}

void ObjectPropertiesWidget::addCollisionBoxSection(const physis_CollisionBoxInstanceObject &collisionBox)
{
    addTriggerBoxSection(collisionBox.parent_data);

    const auto section = new CollapseSection(i18n("Collision Box"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto assetPathEdit = new PathEdit();
    assetPathEdit->setPath(QString::fromStdString(collisionBox.collision_asset_path));
    layout->addRow(i18n("Collision Asset Path"), assetPathEdit);
}

void ObjectPropertiesWidget::addDoorRangeSection(const physis_DoorRangeInstanceObject &doorRange)
{
    addRangeSection(doorRange.parent_data);

    const auto section = new CollapseSection(i18n("Door Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addLineVFXSection(const physis_LineVFXInstanceObject &lineVfx)
{
    const auto section = new CollapseSection(i18n("Line VFX"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto lineStyleEdit = new EnumEdit<LineStyle>();
    lineStyleEdit->setValue(lineVfx.line_style);
    lineStyleEdit->setEnabled(false);
    layout->addRow(i18n("Line Style"), lineStyleEdit);
}

void ObjectPropertiesWidget::addTreasureSection(physis_TreasureInstanceObject &treasure)
{
    addGameObjectSection(treasure.parent_data);

    const auto section = new CollapseSection(i18n("Treasure"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addTargetMarkerSection(const physis_TargetMarkerInstanceObject &targetMarker)
{
    const auto section = new CollapseSection(i18n("Target Marker"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto nameplateOffsetYEdit = new QLineEdit();
    nameplateOffsetYEdit->setText(QString::number(targetMarker.nameplate_offset_y));
    nameplateOffsetYEdit->setReadOnly(true);
    layout->addRow(i18n("Nameplate Offset Y(?)"), nameplateOffsetYEdit);

    const auto targetMarkerTypeEdit = new EnumEdit<TargetMarkerType>();
    targetMarkerTypeEdit->setValue(targetMarker.target_market_type);
    targetMarkerTypeEdit->setEnabled(false);
    layout->addRow(i18n("Type"), targetMarkerTypeEdit);
}

void ObjectPropertiesWidget::addClientPathSection(const physis_ClientPathInstanceObject &clientPath)
{
    addPathSection(clientPath.parent_data);

    const auto section = new CollapseSection(i18n("Client Path"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addPathSection(const physis_PathInstanceObject &)
{
    const auto section = new CollapseSection(i18n("Path"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addRangeSection(const physis_RangeInstanceObject &range)
{
    Q_UNUSED(range)

    const auto section = new CollapseSection(i18n("Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addCullingBoxSection(const physis_CullingBoxInstanceObject &cullingBox)
{
    Q_UNUSED(cullingBox)

    const auto section = new CollapseSection(i18n("Culling Box"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addClickableRangeSection(const physis_ClickableRangeInstanceObject &clickableRange)
{
    addRangeSection(clickableRange.parent_data);

    const auto section = new CollapseSection(i18n("Clickable Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addBattleNpcSection(physis_BattleNpcInstanceObject &battleNpc)
{
    addCharacterSection(battleNpc.parent_data);

    const auto section = new CollapseSection(i18n("Battle Npc"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    const auto nameIdEdit = new ExcelEdit(m_appState, {QStringLiteral("BNpcName")}, battleNpc.name_id);
    layout->addRow(i18n("Name ID"), nameIdEdit);
}

#include "moc_objectpropertieswidget.cpp"
