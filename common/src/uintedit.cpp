// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uintedit.h"

#include <QHBoxLayout>
#include <QTimer>

UIntEdit::UIntEdit(uint32_t &value, QWidget *parent)
    : EditWidget(parent)
    , m_value(value)
{
    auto itemsLayout = new QHBoxLayout(this);
    itemsLayout->setContentsMargins(0, 0, 0, 0);

    m_spinBox = new QSpinBox();

    m_spinBox->setMinimum(0);
    m_spinBox->setMaximum(std::numeric_limits<int>::max());

    itemsLayout->addWidget(m_spinBox);

    m_spinBox->setValue(value);

    connect(m_spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](const int d) {
        this->m_value = d;
        Q_EMIT onValueChanged();
    });
    connect(m_spinBox, &QSpinBox::editingFinished, this, &EditWidget::editingFinished);
}

#include "moc_uintedit.cpp"
