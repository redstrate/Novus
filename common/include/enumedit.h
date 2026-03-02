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

        comboBox = new QComboBox();
        itemsLayout->addWidget(comboBox);

        for (auto [type, name] : magic_enum::enum_entries<E>()) {
            comboBox->addItem(QString::fromLatin1(name.data()));
        }
    }
    ~EnumEdit() override = default;

    void setValue(const E &value)
    {
        QSignalBlocker blocker(comboBox);
        comboBox->setCurrentIndex(static_cast<int>(value));
    }

    void resetValue()
    {
        comboBox->setCurrentIndex(-1);
    }

private:
    QComboBox *comboBox = nullptr;
};
