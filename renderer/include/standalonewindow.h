// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "renderer.hpp"

struct SDL_Window;

class StandaloneWindow {
public:
    StandaloneWindow(Renderer* renderer);

    VkSurfaceKHR getSurface(VkInstance instance);

    void render();

    std::vector<RenderModel> models;

private:
    Renderer* m_renderer;
    SDL_Window* m_window;
};