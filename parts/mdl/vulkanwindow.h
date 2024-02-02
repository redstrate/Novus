// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWindow>

#include "imgui.h"
#include "mdlpart.h"

class VulkanWindow : public QWindow
{
public:
    VulkanWindow(MDLPart *part, Renderer *renderer, QVulkanInstance *instance);

    void exposeEvent(QExposeEvent *) override;

    bool event(QEvent *e) override;

    void render();

    std::vector<RenderModel> models;
    bool freeMode = false;

private:
    bool m_initialized = false;
    Renderer *m_renderer;
    QVulkanInstance *m_instance;
    MDLPart *part;
    bool pressed_keys[4] = {};
};