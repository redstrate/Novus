// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QElapsedTimer>
#include <QWindow>

#include "mdlpart.h"

class VulkanWindow : public QWindow
{
public:
    VulkanWindow(MDLPart *part, RenderManager *renderer, QVulkanInstance *instance);

    bool eventFilter(QObject *watched, QEvent *event) override;

    void render();

    std::vector<DrawObjectInstance> models;
    std::unordered_map<QString, DrawObject *> sourceModels;
    bool freeMode = false;
    std::vector<VfxObjectInstance> vfx;
    std::unordered_map<QString, VfxObject *> sourceVfx;

protected:
    void exposeEvent(QExposeEvent *) override;
    bool event(QEvent *e) override;

    friend class MDLPart;

private:
    bool m_initialized = false;
    RenderManager *m_renderer = nullptr;
    QVulkanInstance *m_instance = nullptr;
    MDLPart *m_part = nullptr;
    bool m_pressedKeys[7] = {};
    QElapsedTimer m_timer;
};
