// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <physis.hpp>

#include "imagelabel.h"

class TexPart : public QWidget
{
    Q_OBJECT

public:
    explicit TexPart(SqPackResource *data, QWidget *parent = nullptr);

    /**
     * @brief Loads a .tex file.
     */
    void loadTex(physis_Buffer file);

    /**
     * @brief Loads a .hwc file.
     */
    void loadHwc(physis_Buffer file);

private:
    SqPackResource *data = nullptr;

    ImageLabel *m_label = nullptr;
};
