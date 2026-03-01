// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vec3edit.h"

#include <QHBoxLayout>
#include <QTimer>

Vector3Edit::Vector3Edit(glm::vec3 &vec, QWidget *parent)
    : EditWidget(parent)
    , vec(vec)
{
    auto itemsLayout = new QHBoxLayout(this);

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

    spinBoxes.x->setValue(vec.x);
    spinBoxes.y->setValue(vec.y);
    spinBoxes.z->setValue(vec.z);

    connect(spinBoxes.x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.x = d;
        Q_EMIT onValueChanged();
    });
    connect(spinBoxes.y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.y = d;
        Q_EMIT onValueChanged();
    });
    connect(spinBoxes.z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.z = d;
        Q_EMIT onValueChanged();
    });
}

void Vector3Edit::setVector(glm::vec3 &vec)
{
    this->vec = vec;
    spinBoxes.x->setValue(vec.x);
    spinBoxes.y->setValue(vec.y);
    spinBoxes.z->setValue(vec.z);
}

void Vector3Edit::setReadOnly(const bool readOnly)
{
    spinBoxes.x->setReadOnly(readOnly);
    spinBoxes.y->setReadOnly(readOnly);
    spinBoxes.z->setReadOnly(readOnly);
}

#include "moc_vec3edit.cpp"
