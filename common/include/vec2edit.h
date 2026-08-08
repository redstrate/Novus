// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "editwidget.h"

#include <QSpinBox>
#include <QWidget>
#include <glm/glm.hpp>

#include "novuscommon_export.h"

class NOVUSCOMMON_EXPORT Vector2Edit : public EditWidget
{
    Q_OBJECT

public:
    explicit Vector2Edit(glm::vec2 &vec, QWidget *parent = nullptr);
    ~Vector2Edit() override = default;

    void setVector(const glm::vec2 &vec) const;
    void setReadOnly(bool readOnly) const;

private:
    struct {
        QDoubleSpinBox *x = nullptr, *y = nullptr;
    } m_spinBoxes;

    glm::vec2 &m_vec;
};
