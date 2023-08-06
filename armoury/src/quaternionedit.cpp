// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quaternionedit.h"

#include <QHBoxLayout>
#include <QTimer>
#include <glm/gtc/quaternion.hpp>

QuaternionEdit::QuaternionEdit(glm::quat& quat, QWidget* parent) : QWidget(parent), quat(quat) {
    auto itemsLayout = new QHBoxLayout(this);
    itemsLayout->setMargin(0);

    spinBoxes.x = new QDoubleSpinBox();
    spinBoxes.y = new QDoubleSpinBox();
    spinBoxes.z = new QDoubleSpinBox();

    spinBoxes.x->setMinimum(-10000.0);
    spinBoxes.x->setMaximum(10000.0);

    spinBoxes.y->setMinimum(-10000.0);
    spinBoxes.y->setMaximum(10000.0);

    spinBoxes.z->setMinimum(-10000.0);
    spinBoxes.z->setMaximum(10000.0);

    itemsLayout->addWidget(spinBoxes.x);
    itemsLayout->addWidget(spinBoxes.y);
    itemsLayout->addWidget(spinBoxes.z);

    auto euler = glm::eulerAngles(quat);
    euler.x = glm::degrees(euler.x);
    euler.y = glm::degrees(euler.y);
    euler.z = glm::degrees(euler.z);

    spinBoxes.x->setValue(euler.x);
    spinBoxes.y->setValue(euler.y);
    spinBoxes.z->setValue(euler.z);

    connect(
        spinBoxes.x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d) {
            auto euler = glm::eulerAngles(this->quat);
            euler.x = glm::radians(d);

            this->quat = glm::quat(euler);

            Q_EMIT onValueChanged();
        });
    connect(
        spinBoxes.y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d) {
            auto euler = glm::eulerAngles(this->quat);
            euler.y = glm::radians(d);

            this->quat = glm::quat(euler);

            Q_EMIT onValueChanged();
        });
    connect(
        spinBoxes.z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d) {
            auto euler = glm::eulerAngles(this->quat);
            euler.z = glm::radians(d);

            this->quat = glm::quat(euler);

            Q_EMIT onValueChanged();
        });
}

void QuaternionEdit::setQuat(glm::quat& quat) {
    this->quat = quat;
    auto euler = glm::eulerAngles(quat);
    euler.x = glm::degrees(euler.x);
    euler.y = glm::degrees(euler.y);
    euler.z = glm::degrees(euler.z);

    spinBoxes.x->setValue(euler.x);
    spinBoxes.y->setValue(euler.y);
    spinBoxes.z->setValue(euler.z);
}

#include "moc_quaternionedit.cpp"