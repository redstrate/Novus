// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vec2edit.h"

#include <QHBoxLayout>
#include <QTimer>

Vector2Edit::Vector2Edit(glm::vec2 &vec, QWidget *parent)
    : EditWidget(parent)
    , m_vec(vec)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    const auto itemsLayout = new QHBoxLayout(this);
    itemsLayout->setContentsMargins({0, 0, 0, 0});
    itemsLayout->setSizeConstraints(QLayout::SetMinAndMaxSize, QLayout::SetMinAndMaxSize);

    m_spinBoxes.x = new QDoubleSpinBox();
    m_spinBoxes.y = new QDoubleSpinBox();

    m_spinBoxes.x->setMinimum(-10000.0);
    m_spinBoxes.x->setMaximum(10000.0);

    m_spinBoxes.y->setMinimum(-10000.0);
    m_spinBoxes.y->setMaximum(10000.0);

    itemsLayout->addWidget(m_spinBoxes.x);
    itemsLayout->addWidget(m_spinBoxes.y);

    m_spinBoxes.x->setValue(vec.x);
    m_spinBoxes.y->setValue(vec.y);

    connect(m_spinBoxes.x, &QDoubleSpinBox::valueChanged, [this, &vec](const double d) {
        vec.x = d;
        Q_EMIT onValueChanged();
    });
    connect(m_spinBoxes.y, &QDoubleSpinBox::valueChanged, [this, &vec](const double d) {
        vec.y = d;
        Q_EMIT onValueChanged();
    });
}

void Vector2Edit::setVector(const glm::vec2 &vec) const
{
    this->m_vec = vec;
    m_spinBoxes.x->setValue(vec.x);
    m_spinBoxes.y->setValue(vec.y);
}

void Vector2Edit::setReadOnly(const bool readOnly) const
{
    m_spinBoxes.x->setReadOnly(readOnly);
    m_spinBoxes.y->setReadOnly(readOnly);
}

#include "moc_vec2edit.cpp"
