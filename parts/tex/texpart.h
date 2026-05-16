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
    explicit TexPart(QWidget *parent = nullptr);

    /**
     * @brief Loads a .tex file.
     */
    bool loadTex(Platform platform, physis_Buffer file);

    /**
     * @brief Loads a .hwc file.
     */
    void loadHwc(Platform platform, physis_Buffer file);

    /**
     * @brief Loads a .png file.
     */
    void loadPng(physis_Buffer file);

    QAction *saveImageAction();

private:
    ImageLabel *m_label = nullptr;
    QAction *m_saveImage = nullptr;
};
