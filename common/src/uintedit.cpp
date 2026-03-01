// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uintedit.h"

#include <QHBoxLayout>
#include <QTimer>

UIntEdit::UIntEdit(uint32_t &value, QWidget *parent)
    : EditWidget(parent)
    , value(value)
{
    auto itemsLayout = new QHBoxLayout(this);
    itemsLayout->setContentsMargins(0, 0, 0, 0);

    spinBox = new QSpinBox();

    spinBox->setMinimum(-10000);
    spinBox->setMaximum(10000);

    itemsLayout->addWidget(spinBox);

    spinBox->setValue(value);

    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](const int d) {
        this->value = d;
        Q_EMIT onValueChanged();
    });
    connect(spinBox, &QSpinBox::editingFinished, this, &EditWidget::editingFinished);
}

#include "moc_uintedit.cpp"
