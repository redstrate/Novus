// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCheckBox>
#include <QWidget>

#include "editwidget.h"

class BoolEdit : public EditWidget
{
    Q_OBJECT
public:
    explicit BoolEdit(QWidget *parent = nullptr);
    explicit BoolEdit(bool &value, QWidget *parent = nullptr);

    void setValue(bool &value);
    void resetValue();

private:
    QCheckBox *checkBox = nullptr;

    bool *value = nullptr;
};
