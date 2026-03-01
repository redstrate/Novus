// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSpinBox>
#include <QWidget>

#include "editwidget.h"

class NOVUSCOMMON_EXPORT UIntEdit : public EditWidget
{
    Q_OBJECT
public:
    explicit UIntEdit(uint32_t &value, QWidget *parent = nullptr);
    ~UIntEdit() override = default;

private:
    QSpinBox *spinBox = nullptr;

    uint32_t &value;
};
