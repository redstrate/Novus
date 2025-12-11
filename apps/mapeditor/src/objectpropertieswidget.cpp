// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectpropertieswidget.h"

#include "appstate.h"
#include "collapsesection.h"
#include "pathedit.h"
#include "vec3edit.h"

#include <KLocalizedString>
#include <QCheckBox>

#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

#include <glm/gtc/type_ptr.hpp>

#include "enumedit.h"

ObjectPropertiesWidget::ObjectPropertiesWidget(AppState *appState, QWidget *parent)
    : QWidget(parent)
    , m_appState(appState)
{
    m_layout = new QVBoxLayout();
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    connect(appState, &AppState::selectionChanged, this, [this] {
        resetSections();
        if (m_appState->selectedObject) {
            refreshObjectData(*m_appState->selectedObject.value());
        }
        if (m_appState->selectedLayer) {
            refreshLayerData(*m_appState->selectedLayer.value());
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

    auto boundIdEdit = new QLineEdit();
    boundIdEdit->setText(QString::number(eobj.bound_instance_id));
    boundIdEdit->setReadOnly(true);
    layout->addRow(i18n("Bound ID"), boundIdEdit);

    auto instanceIdEdit = new QLineEdit();
    instanceIdEdit->setText(QString::number(eobj.linked_instance_id));
    instanceIdEdit->setReadOnly(true);
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

#include "moc_objectpropertieswidget.cpp"
