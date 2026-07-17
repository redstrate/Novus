// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

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
    bool loadTex(Platform platform, physis_Buffer file) const;

    /**
     * @brief Loads a .hwc file.
     */
    void loadHwc(Platform platform, physis_Buffer file) const;

    /**
     * @brief Loads a .png file.
     */
    void loadPng(physis_Buffer file) const;

    QAction *saveImageAction() const;

private:
    ImageLabel *m_label = nullptr;
    QAction *m_saveImage = nullptr;
};
