// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <glm/vec3.hpp>
#include <physis.hpp>
#include <vector>

struct SceneLight {
    LightType type;
    glm::vec3 position;
    glm::vec3 color{1.0f};
    float intensity = 2.0f;
};

class Scene
{
public:
    Scene();

    void resetLights();

    std::vector<SceneLight> lights;
};
