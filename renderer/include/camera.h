// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <glm/mat4x4.hpp>

struct Camera {
    /// Field of view in degrees
    float fieldOfView = 45.0f;

    /// The aspect ratio of the camera, set automatically by @p RenderManager
    float aspectRatio = 0.0f;

    /// Near plane
    float nearPlane = 0.1f;

    /// Far plane
    float farPlane = 100.0f;

    glm::mat4 perspective, view;
};