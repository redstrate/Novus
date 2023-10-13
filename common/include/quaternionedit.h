// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSpinBox>
#include <QWidget>

#include <glm/glm.hpp>

class QuaternionEdit : public QWidget
{
    Q_OBJECT

public:
    explicit QuaternionEdit(glm::quat &quat, QWidget *parent = nullptr);

    void setQuat(glm::quat &quat);

Q_SIGNALS:
    void onValueChanged();

private:
    struct {
        QDoubleSpinBox *x, *y, *z;
    } spinBoxes;

    glm::quat &quat;
};
