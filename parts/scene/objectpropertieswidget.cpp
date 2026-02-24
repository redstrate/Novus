// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectpropertieswidget.h"

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
#include "objectidedit.h"

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
    });
}

void ObjectPropertiesWidget::resetSections()
{
    for (auto section : m_sections) {
        m_layout->removeWidget(section);
        delete section;
    }
    m_sections.clear();
}

void ObjectPropertiesWidget::refreshObjectData(const physis_InstanceObject &object)
{
    addCommonSection(object);

    switch (object.data.tag) {
    case physis_LayerEntry::Tag::BG:
        addBGSection(object.data.bg._0);
        break;
    case physis_LayerEntry::Tag::LayLight:
        addLightSection(object.data.lay_light._0);
        break;
    case physis_LayerEntry::Tag::Vfx:
        addVfxSection(object.data.vfx._0);
        break;
    case physis_LayerEntry::Tag::EventObject:
        addEventSection(object.data.event_object._0);
        break;
    case physis_LayerEntry::Tag::PopRange:
        addPopRangeSection(object.data.pop_range._0);
        break;
    case physis_LayerEntry::Tag::EventNPC:
        addEventNPCSection(object.data.event_npc._0);
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

    auto instanceWidget = new QTableWidget();
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

    auto typeEdit = new EnumEdit<ScnSGActionControllerDescriptor::Tag>();
    typeEdit->setValue(action.tag);
    typeEdit->setEnabled(false);
    layout->addRow(i18n("Type"), typeEdit);

    switch (action.tag) {
    case ScnSGActionControllerDescriptor::Tag::Rotation: {
        const auto &rotation = action.rotation._0;

        auto bgPartIdEdit = new QLineEdit();
        bgPartIdEdit->setText(QString::number(rotation.bg_part_id));
        bgPartIdEdit->setReadOnly(true);
        layout->addRow(i18n("BG Part ID"), bgPartIdEdit);

        auto vfxChildId1Edit = new QLineEdit();
        vfxChildId1Edit->setText(QString::number(rotation.vfx_child1_id));
        vfxChildId1Edit->setReadOnly(true);
        layout->addRow(i18n("VFX Child 2 ID"), vfxChildId1Edit);

        auto vfxChildId2Edit = new QLineEdit();
        vfxChildId2Edit->setText(QString::number(rotation.vfx_child_2_id));
        vfxChildId2Edit->setReadOnly(true);
        layout->addRow(i18n("VFX Child 1 ID"), vfxChildId2Edit);

        auto rotationAxisEdit = new EnumEdit<RotationAxis>();
        rotationAxisEdit->setValue(rotation.axis);
        rotationAxisEdit->setEnabled(false);
        layout->addRow(i18n("Axis"), rotationAxisEdit);

        auto durationEdit = new QLineEdit();
        durationEdit->setText(QString::number(rotation.duration));
        durationEdit->setReadOnly(true);
        layout->addRow(i18n("Duration"), durationEdit);

        auto valueEdit = new QLineEdit();
        valueEdit->setText(QString::number(rotation.value));
        valueEdit->setReadOnly(true);
        layout->addRow(i18n("Value"), valueEdit);
    } break;
    case ScnSGActionControllerDescriptor::Tag::Unknown:
        break;
    }
}

void ObjectPropertiesWidget::addCommonSection(const physis_InstanceObject &object)
{
    const auto section = new CollapseSection(i18n("Common"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    const auto layout = new QFormLayout();
    section->setLayout(layout);

    auto pos = glm::make_vec3(object.transform.translation);
    const auto positionEdit = new Vector3Edit(pos);
    positionEdit->setReadOnly(true);
    layout->addRow(i18n("Position"), positionEdit);

    auto rotation = glm::make_vec3(object.transform.rotation);
    const auto rotationEdit = new Vector3Edit(rotation);
    rotationEdit->setReadOnly(true);
    layout->addRow(i18n("Rotation"), rotationEdit);

    auto scale = glm::make_vec3(object.transform.scale);
    const auto scaleEdit = new Vector3Edit(scale);
    scaleEdit->setReadOnly(true);
    layout->addRow(i18n("Scale"), scaleEdit);

    const auto idEdit = new QLineEdit();
    idEdit->setText(QString::number(object.instance_id));
    idEdit->setReadOnly(true);
    layout->addRow(i18n("Instance ID"), idEdit);
}

void ObjectPropertiesWidget::addBGSection(const physis_BGInstanceObject &bg)
{
    auto section = new CollapseSection(i18n("BG"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto bgEdit = new PathEdit();
    bgEdit->setPath(QString::fromLatin1(bg.asset_path));
    bgEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), bgEdit);

    auto collisionEdit = new PathEdit();
    collisionEdit->setPath(QString::fromLatin1(bg.collision_asset_path));
    collisionEdit->setReadOnly(true);
    layout->addRow(i18n("Collision Asset Path"), collisionEdit);
}

void ObjectPropertiesWidget::addEventSection(const physis_EventInstanceObject &eobj)
{
    addGameObjectSection(eobj.parent_data);

    auto section = new CollapseSection(i18n("Event Object"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto boundIdEdit = new ObjectIdEdit(m_appState);
    boundIdEdit->setObjectId(eobj.bound_instance_id);
    layout->addRow(i18n("Bound ID"), boundIdEdit);

    auto instanceIdEdit = new ObjectIdEdit(m_appState);
    instanceIdEdit->setObjectId(eobj.linked_instance_id);
    layout->addRow(i18n("Linked ID"), instanceIdEdit);
}

void ObjectPropertiesWidget::addPopRangeSection(const physis_PopRangeInstanceObject &pop)
{
    auto section = new CollapseSection(i18n("Pop Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto typeEdit = new QLineEdit();
    switch (pop.pop_type) {
    case PopType::PC:
        typeEdit->setText(i18n("PC"));
        break;
    case PopType::Npc:
        typeEdit->setText(i18n("NPC"));
        break;
    case PopType::Content:
        typeEdit->setText(i18n("Content"));
        break;
    }
    typeEdit->setReadOnly(true);
    layout->addRow(i18n("Type"), typeEdit);

    auto indexEdit = new QLineEdit();
    indexEdit->setText(QString::number(pop.index));
    indexEdit->setReadOnly(true);
    layout->addRow(i18n("Index"), indexEdit);
}

void ObjectPropertiesWidget::addEventNPCSection(const physis_ENPCInstanceObject &enpc)
{
    addNPCSection(enpc.parent_data);

    auto section = new CollapseSection(i18n("Event NPC"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addMapRangeSection(const physis_MapRangeInstanceObject &mapRange)
{
    addTriggerBoxSection(mapRange.parent_data);

    auto section = new CollapseSection(i18n("Map Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto placeNameBlock = new QLineEdit();
    placeNameBlock->setText(QString::number(mapRange.place_name_block));
    placeNameBlock->setReadOnly(true);
    layout->addRow(i18n("PlaceName Block"), placeNameBlock);

    auto placeNameSpot = new QLineEdit();
    placeNameSpot->setReadOnly(true);
    placeNameSpot->setText(QString::number(mapRange.place_name_spot));
    layout->addRow(i18n("PlaceName Spot"), placeNameSpot);

    auto restBonusEffectiveCheckbox = new QCheckBox();
    restBonusEffectiveCheckbox->setChecked(mapRange.rest_bonus_effective);
    restBonusEffectiveCheckbox->setEnabled(false);
    layout->addRow(i18n("Rest Bonus Effective"), restBonusEffectiveCheckbox);

    auto discoveryIdEdit = new QLineEdit();
    discoveryIdEdit->setReadOnly(true);
    discoveryIdEdit->setText(QString::number(mapRange.discovery_id));
    layout->addRow(i18n("Discovery ID"), discoveryIdEdit);

    auto placeNameEnabledCheckbox = new QCheckBox();
    placeNameEnabledCheckbox->setChecked(mapRange.place_name_enabled);
    placeNameEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Place Name Enabled"), placeNameEnabledCheckbox);

    auto discoveryEnabledCheckbox = new QCheckBox();
    discoveryEnabledCheckbox->setChecked(mapRange.discovery_enabled);
    discoveryEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Discovery Enabled"), discoveryEnabledCheckbox);

    auto restBonusEnabledCheckbox = new QCheckBox();
    restBonusEnabledCheckbox->setChecked(mapRange.rest_bonus_enabled);
    restBonusEnabledCheckbox->setEnabled(false);
    layout->addRow(i18n("Rest Bonus Enabled"), restBonusEnabledCheckbox);
}

void ObjectPropertiesWidget::addTriggerBoxSection(const physis_TriggerBoxInstanceObject &triggerBox)
{
    auto section = new CollapseSection(i18n("Trigger Box"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto shapeEdit = new EnumEdit<TriggerBoxShape>();
    shapeEdit->setValue(triggerBox.trigger_box_shape);
    shapeEdit->setEnabled(false);
    layout->addRow(i18n("Shape"), shapeEdit);

    auto priorityEdit = new QLineEdit();
    priorityEdit->setText(QString::number(triggerBox.priority));
    priorityEdit->setReadOnly(true);
    layout->addRow(i18n("Priority"), priorityEdit);

    auto enabledCheckBox = new QCheckBox();
    enabledCheckBox->setChecked(triggerBox.enabled);
    enabledCheckBox->setEnabled(false);
    layout->addRow(i18n("Enabled"), enabledCheckBox);
}

void ObjectPropertiesWidget::addNPCSection(const physis_NPCInstanceObject &npc)
{
    addGameObjectSection(npc.parent_data);

    auto section = new CollapseSection(i18n("NPC"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);
}

void ObjectPropertiesWidget::addGameObjectSection(const physis_GameInstanceObject &object)
{
    auto section = new CollapseSection(i18n("Game Object"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto baseIdEdit = new QLineEdit();
    baseIdEdit->setText(QString::number(object.base_id));
    baseIdEdit->setReadOnly(true);
    layout->addRow(i18n("Base ID"), baseIdEdit);
}

void ObjectPropertiesWidget::addSharedGroupSection(const physis_SharedGroupInstanceObject &sharedGroup)
{
    auto section = new CollapseSection(i18n("Shared Group"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto assetPathEdit = new PathEdit();
    assetPathEdit->setPath(QString::fromLatin1(sharedGroup.asset_path));
    assetPathEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), assetPathEdit);
}

void ObjectPropertiesWidget::addAetheryteSection(const physis_AetheryteInstanceObject &aetheryte)
{
    addGameObjectSection(aetheryte.parent_data);

    auto section = new CollapseSection(i18n("Aetheryte"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto boundInstanceIdEdit = new ObjectIdEdit(m_appState);
    boundInstanceIdEdit->setObjectId(aetheryte.bound_instance_id);
    layout->addRow(i18n("Bound Instance ID"), boundInstanceIdEdit);
}

void ObjectPropertiesWidget::addExitRangeSection(const physis_ExitRangeInstanceObject &exitRange)
{
    addTriggerBoxSection(exitRange.parent_data);

    auto section = new CollapseSection(i18n("Exit Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto exitTypeEdit = new EnumEdit<ExitType>();
    exitTypeEdit->setValue(exitRange.exit_type);
    exitTypeEdit->setEnabled(false);
    layout->addRow(i18n("Exit Type"), exitTypeEdit);

    auto zoneIdEdit = new QLineEdit();
    zoneIdEdit->setText(QString::number(exitRange.zone_id));
    zoneIdEdit->setReadOnly(true);
    layout->addRow(i18n("Zone ID"), zoneIdEdit);

    auto territoryTypeEdit = new QLineEdit();
    territoryTypeEdit->setText(QString::number(exitRange.territory_type));
    territoryTypeEdit->setReadOnly(true);
    layout->addRow(i18n("Territory Type"), territoryTypeEdit);

    auto destinationInstanceIdEdit = new ObjectIdEdit(m_appState);
    destinationInstanceIdEdit->setObjectId(exitRange.destination_instance_id);
    layout->addRow(i18n("Destination Instance ID"), destinationInstanceIdEdit);

    auto returnInstanceIdEdit = new ObjectIdEdit(m_appState);
    returnInstanceIdEdit->setObjectId(exitRange.return_instance_id);
    layout->addRow(i18n("Return Instance ID"), returnInstanceIdEdit);
}

void ObjectPropertiesWidget::addEventRangeSection(const physis_EventRangeInstanceObject &eventRange)
{
    addTriggerBoxSection(eventRange.parent_data);

    auto section = new CollapseSection(i18n("Event Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    setLayout(layout);
}

void ObjectPropertiesWidget::addChairMarkerSection(const physis_ChairMarkerInstanceObject &chairMarker)
{
    Q_UNUSED(chairMarker);

    auto section = new CollapseSection(i18n("Chair Marker"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    setLayout(layout);
}

void ObjectPropertiesWidget::addPrefetchRangeSection(const physis_PrefetchRangeInstanceObject &prefetchRange)
{
    addTriggerBoxSection(prefetchRange.parent_data);

    auto section = new CollapseSection(i18n("Prefetch Range"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto boundInstanceIdEdit = new ObjectIdEdit(m_appState);
    boundInstanceIdEdit->setObjectId(prefetchRange.bound_instance_id);
    layout->addRow(i18n("Bound Instance ID"), boundInstanceIdEdit);
}

void ObjectPropertiesWidget::addLightSection(const physis_LightInstanceObject &light)
{
    auto section = new CollapseSection(i18n("Light"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto lightTypeEdit = new EnumEdit<LightType>();
    lightTypeEdit->setValue(light.light_type);
    lightTypeEdit->setEnabled(false);
    layout->addRow(i18n("Light Type"), lightTypeEdit);
}

void ObjectPropertiesWidget::addVfxSection(const physis_VfxInstanceObject &vfx)
{
    auto section = new CollapseSection(i18n("VFX"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto assetPathEdit = new PathEdit();
    assetPathEdit->setPath(QString::fromLatin1(vfx.asset_path));
    assetPathEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), assetPathEdit);
}

#include "moc_objectpropertieswidget.cpp"
