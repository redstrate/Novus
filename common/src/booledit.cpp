// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "booledit.h"

#include <QHBoxLayout>

BoolEdit::BoolEdit(QWidget *parent)
    : EditWidget(parent)
{
    auto itemsLayout = new QHBoxLayout(this);
    itemsLayout->setContentsMargins(0, 0, 0, 0);

    checkBox = new QCheckBox();
    itemsLayout->addWidget(checkBox);

    connect(checkBox, &QCheckBox::stateChanged, this, [this](int state) {
        if (this->value) {
            *this->value = state;
            Q_EMIT onValueChanged();
            Q_EMIT editingFinished();
        }
    });
}

BoolEdit::BoolEdit(bool &value, QWidget *parent)
    : BoolEdit(parent)
{
    setValue(value);
}

void BoolEdit::setValue(bool &value)
{
    this->value = &value;

    checkBox->setChecked(value);
}

void BoolEdit::resetValue()
{
    this->value = nullptr;
}

#include "moc_booledit.cpp"
