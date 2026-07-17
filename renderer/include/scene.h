// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <glm/vec3.hpp>
#include <physis.hpp>
#include <vector>

struct SceneLight {
    bool active = true;
    uint32_t id = 0;
    uint32_t parentSgbId = 0;
    LightShape type{};
    glm::vec3 position{};
    glm::vec3 color{1.0f};
    float intensity = 2.0f;
};

class Scene
{
public:
    Scene();

    void resetLights();

    std::vector<SceneLight> lights;

    bool wireframe = false;
    bool frustumCulling = true;
    bool debugFrustumCulling = false;

    size_t culledObjects = 0;
    size_t culledLights = 0;

    float time = 0.0f;
};
