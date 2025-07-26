// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectpropertieswidget.h"

#include "appstate.h"
#include "collapsesection.h"

#include <KLocalizedString>

#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

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
    default:
        break;
    }
}

void ObjectPropertiesWidget::addCommonSection(const physis_InstanceObject &object)
{
    auto section = new CollapseSection(i18n("Common"));
    m_layout->addWidget(section);
    m_sections.push_back(section);

    auto layout = new QFormLayout();
    section->setLayout(layout);

    auto idEdit = new QLineEdit();
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

    auto bgEdit = new QLineEdit();
    bgEdit->setText(QString::fromLatin1(bg.asset_path));
    bgEdit->setReadOnly(true);
    layout->addRow(i18n("Asset Path"), bgEdit);
}

#include "moc_objectpropertieswidget.cpp"
