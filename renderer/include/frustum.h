// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <glm/glm.hpp>
#include <physis.hpp>

struct Camera;

struct Plane {
    float a = 0.0, b = 0.0, c = 0.0, d = 0.0;
};

inline Plane normalize(const Plane &plane)
{
    const float magnitude = std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);

    Plane normalized_plane = plane;
    normalized_plane.a = plane.a / magnitude;
    normalized_plane.b = plane.b / magnitude;
    normalized_plane.c = plane.c / magnitude;
    normalized_plane.d = plane.d / magnitude;

    return normalized_plane;
}

inline float distance_to_point(const Plane &plane, const glm::vec3 &point)
{
    return plane.a * point.x + plane.b * point.y + plane.c * point.z + plane.d;
}

struct CameraFrustum {
    std::array<Plane, 6> planes;
};

CameraFrustum extract_frustum(glm::mat4 combined);
CameraFrustum camera_extract_frustum(const Camera &cam);
CameraFrustum normalize_frustum(const CameraFrustum &frustum);

bool test_aabb_frustum(const CameraFrustum &frustum, const BoundingBox &aabb);
bool contains(const BoundingBox &box, const glm::vec3 &point);
