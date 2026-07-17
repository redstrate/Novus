// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frustum.h"

#include "camera.h"

#include <glm/gtc/type_ptr.hpp>

CameraFrustum extract_frustum(glm::mat4 combined)
{
    CameraFrustum frustum;

    // left plane
    frustum.planes[0].a = combined[0][3] + combined[0][0];
    frustum.planes[0].b = combined[1][3] + combined[1][0];
    frustum.planes[0].c = combined[2][3] + combined[2][0];
    frustum.planes[0].d = combined[3][3] + combined[3][0];

    // right plane
    frustum.planes[1].a = combined[0][3] - combined[0][0];
    frustum.planes[1].b = combined[1][3] - combined[1][0];
    frustum.planes[1].c = combined[2][3] - combined[2][0];
    frustum.planes[1].d = combined[3][3] - combined[3][0];

    // top plane
    frustum.planes[2].a = combined[0][3] - combined[0][1];
    frustum.planes[2].b = combined[1][3] - combined[1][1];
    frustum.planes[2].c = combined[2][3] - combined[2][1];
    frustum.planes[2].d = combined[3][3] - combined[3][1];

    // bottom plane
    frustum.planes[3].a = combined[0][3] + combined[0][1];
    frustum.planes[3].b = combined[1][3] + combined[1][1];
    frustum.planes[3].c = combined[2][3] + combined[2][1];
    frustum.planes[3].d = combined[3][3] + combined[3][1];

    // near plane
    frustum.planes[4].a = combined[0][2];
    frustum.planes[4].b = combined[1][2];
    frustum.planes[4].c = combined[2][2];
    frustum.planes[4].d = combined[3][2];

    // far plane
    frustum.planes[5].a = combined[0][3] - combined[0][2];
    frustum.planes[5].b = combined[1][3] - combined[1][2];
    frustum.planes[5].c = combined[2][3] - combined[2][2];
    frustum.planes[5].d = combined[3][3] - combined[3][2];

    return frustum;
}

CameraFrustum camera_extract_frustum(const Camera &cam)
{
    const glm::mat4 combined = cam.perspective * cam.view;

    return extract_frustum(combined);
}

std::array<glm::vec3, 8> get_points(const BoundingBox &aabb)
{
    return {glm::vec3(aabb.min[0], aabb.min[1], aabb.min[2]),
            glm::vec3(aabb.max[0], aabb.min[1], aabb.min[2]),
            glm::vec3(aabb.min[0], aabb.max[1], aabb.min[2]),
            glm::vec3(aabb.max[0], aabb.max[1], aabb.min[2]),
            glm::vec3(aabb.min[0], aabb.min[1], aabb.max[2]),
            glm::vec3(aabb.max[0], aabb.min[1], aabb.max[2]),
            glm::vec3(aabb.min[0], aabb.max[1], aabb.max[2]),
            glm::vec3(aabb.max[0], aabb.max[1], aabb.max[2])};
}

CameraFrustum normalize_frustum(const CameraFrustum &frustum)
{
    CameraFrustum normalized_frustum;
    for (std::size_t i = 0; i < 6; i++)
        normalized_frustum.planes[i] = normalize(frustum.planes[i]);

    return normalized_frustum;
}

bool test_aabb_frustum(const CameraFrustum &frustum, const BoundingBox &aabb)
{
    for (std::size_t i = 0; i < 6; i++) {
        int out = 0;

        for (const auto point : get_points(aabb))
            out += distance_to_point(frustum.planes[i], point) < 0.0 ? 1 : 0;

        if (out == 8)
            return false;
    }

    return true;
}

bool intersects(const BoundingBox &a, const BoundingBox &b)
{
    return glm::all(glm::lessThanEqual(glm::make_vec3(a.min), glm::make_vec3(b.max)))
        && glm::all(glm::greaterThanEqual(glm::make_vec3(a.max), glm::make_vec3(b.min)));
}

bool contains(const BoundingBox &box, const glm::vec3 &point)
{
    return intersects(box, BoundingBox{.min = {point[0], point[1], point[2]}, .max = {point[0], point[1], point[2]}});
}
