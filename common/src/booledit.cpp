// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "booledit.h"

#include <QHBoxLayout>

BoolEdit::BoolEdit(QWidget *parent)
    : EditWidget(parent)
{
    auto itemsLayout = new QHBoxLayout(this);
    itemsLayout->setContentsMargins(0, 0, 0, 0);

    m_checkBox = new QCheckBox();
    itemsLayout->addWidget(m_checkBox);

    connect(m_checkBox, &QCheckBox::checkStateChanged, this, [this](const Qt::CheckState state) {
        Q_UNUSED(state)

        if (this->m_value) {
            *this->m_value = state;
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
    this->m_value = &value;

    m_checkBox->setChecked(value);
}

void BoolEdit::resetValue()
{
    this->m_value = nullptr;
}

#include "moc_booledit.cpp"
