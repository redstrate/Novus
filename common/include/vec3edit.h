// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSpinBox>
#include <QWidget>
#include <glm/glm.hpp>

#include "novuscommon_export.h"

class NOVUSCOMMON_EXPORT Vector3Edit : public QWidget
{
    Q_OBJECT

public:
    explicit Vector3Edit(glm::vec3 &vec, QWidget *parent = nullptr);

    void setVector(glm::vec3 &vec);

Q_SIGNALS:
    void onValueChanged();

private:
    struct {
        QDoubleSpinBox *x = nullptr, *y = nullptr, *z = nullptr;
    } spinBoxes;

    glm::vec3 &vec;
};
