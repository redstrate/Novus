// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColorDialog>
#include <QWidget>
#include <physis.hpp>

#include "editwidget.h"

class NOVUSCOMMON_EXPORT ColorEdit : public EditWidget
{
    Q_OBJECT

public:
    explicit ColorEdit(QWidget *parent = nullptr);

    void setColor(ColorIntensity &color)
    {
        reference = &color;
        rebuild();
    }

private:
    void rebuild() const;

    ColorIntensity *reference = nullptr;
    QPushButton *colorButton = nullptr;
};
