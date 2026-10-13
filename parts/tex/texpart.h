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
    bool loadTex(Platform platform, physis_Buffer file);

    /**
     * @brief Loads a .hwc file.
     */
    void loadHwc(Platform platform, physis_Buffer file) const;

    /**
     * @brief Loads a .png file.
     */
    void loadPng(physis_Buffer file) const;

    QAction *saveImageAction() const;
    QAction *layerAction() const;
    QAction *levelAction() const;

private:
    ImageLabel *m_label = nullptr;
    QAction *m_saveImageAction = nullptr;
    QAction *m_layerAction = nullptr;
    QMenu *m_layerMenu = nullptr;
    QActionGroup *m_layerGroup = nullptr;
    QAction *m_levelAction = nullptr;
    QMenu *m_levelMenu = nullptr;
    QActionGroup *m_levelGroup = nullptr;
    int m_level = 0;
    int m_layer = 0;
    QList<QList<QImage>> m_levels;
};
