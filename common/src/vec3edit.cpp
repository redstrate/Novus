// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vec3edit.h"

#include <QHBoxLayout>
#include <QTimer>

Vector3Edit::Vector3Edit(glm::vec3 &vec, QWidget *parent)
    : EditWidget(parent)
    , m_vec(vec)
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

    m_spinBoxes.x->setValue(vec.x);
    m_spinBoxes.y->setValue(vec.y);
    m_spinBoxes.z->setValue(vec.z);

    connect(m_spinBoxes.x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.x = d;
        Q_EMIT onValueChanged();
    });
    connect(m_spinBoxes.y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.y = d;
        Q_EMIT onValueChanged();
    });
    connect(m_spinBoxes.z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, &vec](double d) {
        vec.z = d;
        Q_EMIT onValueChanged();
    });
}

void Vector3Edit::setVector(glm::vec3 &vec)
{
    this->m_vec = vec;
    m_spinBoxes.x->setValue(vec.x);
    m_spinBoxes.y->setValue(vec.y);
    m_spinBoxes.z->setValue(vec.z);
}

void Vector3Edit::setReadOnly(const bool readOnly)
{
    m_spinBoxes.x->setReadOnly(readOnly);
    m_spinBoxes.y->setReadOnly(readOnly);
    m_spinBoxes.z->setReadOnly(readOnly);
}

#include "moc_vec3edit.cpp"
