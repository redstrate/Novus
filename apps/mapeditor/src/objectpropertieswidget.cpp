// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectpropertieswidget.h"

#include "appstate.h"
#include "collapsesection.h"
#include "pathedit.h"
#include "vec3edit.h"

#include <KLocalizedString>

#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

#include <glm/gtc/type_ptr.hpp>

ObjectPropertiesWidget::ObjectPropertiesWidget(AppState *appState, QWidget *parent)
    : QWidget(parent)
    , m_appState(appState)
{
    m_layout = new QVBoxLayout();
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    connect(appState, &AppState::selectedObjectChanged, this, [this] {
        resetObjectData();
        if (m_appState->selectedObject) {
            refreshObjectData(*m_appState->selectedObject.value());
        }
    });
}

void ObjectPropertiesWidget::resetObjectData()
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
    default:
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
    auto section = new CollapseSection(i18n("Event Object"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto baseIdEdit = new QLineEdit();
    baseIdEdit->setText(QString::number(eobj.parent_data.base_id));
    baseIdEdit->setReadOnly(true);
    layout->addRow(i18n("Base ID"), baseIdEdit);

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
    auto section = new CollapseSection(i18n("Event NPC "));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto baseIdEdit = new QLineEdit();
    baseIdEdit->setText(QString::number(enpc.parent_data.parent_data.base_id));
    baseIdEdit->setReadOnly(true);
    layout->addRow(i18n("Base ID"), baseIdEdit);
}

#include "moc_objectpropertieswidget.cpp"
