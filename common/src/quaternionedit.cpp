// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quaternionedit.h"

#include <QHBoxLayout>
#include <QTimer>
#include <glm/gtc/quaternion.hpp>

QuaternionEdit::QuaternionEdit(glm::quat &quat, QWidget *parent)
    : EditWidget(parent)
    , m_quat(quat)
{
    auto itemsLayout = new QHBoxLayout(this);

    m_spinBoxes.x = new QDoubleSpinBox();
    m_spinBoxes.y = new QDoubleSpinBox();
    m_spinBoxes.z = new QDoubleSpinBox();

    m_spinBoxes.x->setMinimum(-10000.0);
    m_spinBoxes.x->setMaximum(10000.0);

    m_spinBoxes.y->setMinimum(-10000.0);
    m_spinBoxes.y->setMaximum(10000.0);

    m_spinBoxes.z->setMinimum(-10000.0);
    m_spinBoxes.z->setMaximum(10000.0);

    itemsLayout->addWidget(m_spinBoxes.x);
    itemsLayout->addWidget(m_spinBoxes.y);
    itemsLayout->addWidget(m_spinBoxes.z);

    auto euler = glm::eulerAngles(quat);
    euler.x = glm::degrees(euler.x);
    euler.y = glm::degrees(euler.y);
    euler.z = glm::degrees(euler.z);

    m_spinBoxes.x->setValue(euler.x);
    m_spinBoxes.y->setValue(euler.y);
    m_spinBoxes.z->setValue(euler.z);

    connect(m_spinBoxes.x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d) {
        auto euler = glm::eulerAngles(this->m_quat);
        euler.x = glm::radians(d);

        this->m_quat = glm::quat(euler);

        Q_EMIT onValueChanged();
    });
    connect(m_spinBoxes.y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d) {
        auto euler = glm::eulerAngles(this->m_quat);
        euler.y = glm::radians(d);

        this->m_quat = glm::quat(euler);

        Q_EMIT onValueChanged();
    });
    connect(m_spinBoxes.z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d) {
        auto euler = glm::eulerAngles(this->m_quat);
        euler.z = glm::radians(d);

        this->m_quat = glm::quat(euler);

        Q_EMIT onValueChanged();
    });
}

void QuaternionEdit::setQuat(glm::quat &quat)
{
    this->m_quat = quat;
    auto euler = glm::eulerAngles(quat);
    euler.x = glm::degrees(euler.x);
    euler.y = glm::degrees(euler.y);
    euler.z = glm::degrees(euler.z);

    m_spinBoxes.x->setValue(euler.x);
    m_spinBoxes.y->setValue(euler.y);
    m_spinBoxes.z->setValue(euler.z);
}

#include "moc_quaternionedit.cpp"
