// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSpinBox>
#include <QWidget>
#include <glm/glm.hpp>

class Vector3Edit : public QWidget {
    Q_OBJECT
public:
    explicit Vector3Edit(glm::vec3& vec, QWidget* parent = nullptr);
    ~Vector3Edit();

    void setVector(glm::vec3& vec);

Q_SIGNALS:
    void onValueChanged();

private:
    struct {
        QDoubleSpinBox *x, *y, *z;
    } spinBoxes;

    glm::vec3& vec;
    QTimer* updateTimer;
};
