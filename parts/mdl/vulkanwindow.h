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

    void exposeEvent(QExposeEvent *);

    bool event(QEvent *e);

    void render();

    std::vector<RenderModel> models;

private:
    bool m_initialized = false;
    Renderer *m_renderer;
    QVulkanInstance *m_instance;
    MDLPart *part;
};