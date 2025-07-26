// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "objectpropertieswidget.h"

#include <QLineEdit>
#include <QVBoxLayout>

#include "appstate.h"

ObjectPropertiesWidget::ObjectPropertiesWidget(AppState *appState, QWidget *parent)
    : QWidget(parent)
    , m_appState(appState)
{
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_idField = new QLineEdit();
    layout->addWidget(m_idField);

    connect(appState, &AppState::selectedObjectChanged, this, [this] {
        if (m_appState->selectedObject) {
            refreshObjectData(*m_appState->selectedObject.value());
        } else {
            resetObjectData();
        }
    });
}

void ObjectPropertiesWidget::resetObjectData()
{
    m_idField->clear();
}

void ObjectPropertiesWidget::refreshObjectData(const physis_InstanceObject &object)
{
    m_idField->setText(QString::number(object.instance_id));
}

#include "moc_objectpropertieswidget.cpp"
