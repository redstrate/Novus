// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "floatedit.h"

#include <QHBoxLayout>
#include <QTimer>

FloatEdit::FloatEdit(QWidget *parent)
    : EditWidget(parent)
{
    const auto itemsLayout = new QHBoxLayout(this);
    itemsLayout->setContentsMargins(0, 0, 0, 0);

    spinBox = new QDoubleSpinBox();
    connect(spinBox, &QDoubleSpinBox::valueChanged, [this](double d) {
        if (this->value) {
            *this->value = d;
            Q_EMIT onValueChanged();
        }
    });
    connect(spinBox, &QDoubleSpinBox::editingFinished, this, &EditWidget::editingFinished);

    spinBox->setMinimum(-10000.0);
    spinBox->setMaximum(10000.0);

    itemsLayout->addWidget(spinBox);
}

void FloatEdit::setValue(float &value)
{
    this->value = &value;

    spinBox->setValue(value);
}

void FloatEdit::resetValue()
{
    this->value = nullptr;
}

void FloatEdit::setSingleStep(const float step) const
{
    spinBox->setSingleStep(step);
}

void FloatEdit::setMinimum(const float minimum) const
{
    spinBox->setMinimum(minimum);
}

void FloatEdit::setMaximum(const float maximum) const
{
    spinBox->setMaximum(maximum);
}

#include "moc_floatedit.cpp"
