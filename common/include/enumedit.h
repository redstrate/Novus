// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "editwidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QWidget>

#include <magic_enum.hpp>

template<class E>
class EnumEdit : public EditWidget
{
public:
    explicit EnumEdit()
    {
        auto itemsLayout = new QHBoxLayout(this);
        itemsLayout->setContentsMargins(0, 0, 0, 0);

        m_comboBox = new QComboBox();
        itemsLayout->addWidget(m_comboBox);

        for (auto [type, name] : magic_enum::enum_entries<E>()) {
            m_comboBox->addItem(QString::fromLatin1(name.data()), static_cast<int>(type));
        }

        connect(m_comboBox, &QComboBox::currentIndexChanged, this, &EnumEdit::editingFinished);
    }
    ~EnumEdit() override = default;

    void setValue(const E &value)
    {
        QSignalBlocker blocker(m_comboBox);
        m_comboBox->setCurrentIndex(m_comboBox->findData(static_cast<int>(value)));
    }

    void resetValue() const
    {
        m_comboBox->setCurrentIndex(-1);
    }

    E value() const
    {
        return static_cast<E>(m_comboBox->currentIndex());
    }

private:
    QComboBox *m_comboBox = nullptr;
};
