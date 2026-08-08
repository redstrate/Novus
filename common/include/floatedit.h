// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSpinBox>
#include <QWidget>

#include "editwidget.h"

class NOVUSCOMMON_EXPORT FloatEdit : public EditWidget
{
    Q_OBJECT

public:
    explicit FloatEdit(QWidget *parent = nullptr);

    void setValue(float &value);
    void resetValue();

    void setSingleStep(float step) const;
    void setMinimum(float minimum) const;
    void setMaximum(float maximum) const;

private:
    QDoubleSpinBox *spinBox = nullptr;

    float *value = nullptr;
};
