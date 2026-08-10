// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scene.h"

#include "physis.hpp"

#include <glm/vec3.hpp>

Scene::Scene()
{
    resetLights();
}

void Scene::resetLights()
{
    lights.clear();
}
