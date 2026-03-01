// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "editwidget.h"

#include <QSpinBox>
#include <QWidget>

#include <glm/glm.hpp>

#include "novuscommon_export.h"

class NOVUSCOMMON_EXPORT QuaternionEdit : public EditWidget
{
    Q_OBJECT

public:
    explicit QuaternionEdit(glm::quat &quat, QWidget *parent = nullptr);
    ~QuaternionEdit() override = default;

    void setQuat(glm::quat &quat);

private:
    struct {
        QDoubleSpinBox *x = nullptr, *y = nullptr, *z = nullptr;
    } spinBoxes;

    glm::quat &quat;
};
