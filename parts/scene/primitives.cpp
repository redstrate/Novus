// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "primitives.h"
#include "rendermanager.h"

#include <complex>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <sys/stat.h>

Sphere Primitives::sphere;
Sphere Primitives::cube;
Cylinder Primitives::cylinder;
Cylinder Primitives::plane;

void Primitives::Initialize(RenderManager *renderer)
{
    // sphere
    {
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;

        unsigned int xResolution = 8;
        unsigned int yResolution = 8;
        float PI = 3.14159265359f;

        for (unsigned int y = 0; y <= yResolution; ++y) {
            for (unsigned int x = 0; x <= xResolution; ++x) {
                float xSegment = static_cast<float>(x) / static_cast<float>(xResolution);
                float ySegment = static_cast<float>(y) / static_cast<float>(yResolution);

                float xPos = glm::cos(xSegment * 2.0f * PI) * glm::sin(ySegment * PI);
                float yPos = glm::cos(ySegment * PI);
                float zPos = glm::sin(xSegment * 2.0f * PI) * glm::sin(ySegment * PI);

                vertices.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < yResolution; ++y) {
            if (!oddRow) {
                for (unsigned int x = 0; x <= xResolution; ++x) {
                    indices.push_back(y * (xResolution + 1) + x);
                    indices.push_back((y + 1) * (xResolution + 1) + x);
                }
            } else {
                for (int x = xResolution; x >= 0; --x) {
                    indices.push_back((y + 1) * (xResolution + 1) + x);
                    indices.push_back(y * (xResolution + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        sphere.indexCount = static_cast<uint32_t>(indices.size());

        // VERTICES
        VkDeviceSize vertexSize = sizeof(glm::vec3) * vertices.size();
        sphere.vertexBuffer = renderer->device().createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        renderer->device().copyToBuffer(sphere.vertexBuffer, vertices.data(), vertexSize);

        // INDICES
        VkDeviceSize indexSize = sizeof(unsigned int) * indices.size();
        sphere.indexBuffer = renderer->device().createBuffer(indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        renderer->device().copyToBuffer(sphere.indexBuffer, indices.data(), indexSize);
    }

    // cube
    {
        constexpr std::array<uint32_t, 36> cube_indices{// Top
                                                        2,
                                                        6,
                                                        7,
                                                        2,
                                                        3,
                                                        7,

                                                        // Bottom
                                                        0,
                                                        4,
                                                        5,
                                                        0,
                                                        1,
                                                        5,

                                                        // Left
                                                        0,
                                                        2,
                                                        6,
                                                        0,
                                                        4,
                                                        6,

                                                        // Right
                                                        1,
                                                        3,
                                                        7,
                                                        1,
                                                        5,
                                                        7,

                                                        // Front
                                                        0,
                                                        2,
                                                        3,
                                                        0,
                                                        1,
                                                        3,

                                                        // Back
                                                        4,
                                                        6,
                                                        7,
                                                        4,
                                                        5,
                                                        7};

        constexpr std::array<float, 24> cube_vertices{
            -1, -1, 0.5, // 0
            1,  -1, 0.5, // 1
            -1, 1,  0.5, // 2
            1,  1,  0.5, // 3
            -1, -1, -0.5, // 4
            1,  -1, -0.5, // 5
            -1, 1,  -0.5, // 6
            1,  1,  -0.5 // 7
        };

        cube.indexCount = static_cast<uint32_t>(cube_indices.size());

        // VERTICES
        VkDeviceSize vertexSize = sizeof(glm::vec3) * 8;
        cube.vertexBuffer = renderer->device().createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        renderer->device().copyToBuffer(cube.vertexBuffer, (void *)cube_vertices.data(), vertexSize);

        // INDICES
        VkDeviceSize indexSize = sizeof(unsigned int) * cube_indices.size();
        cube.indexBuffer = renderer->device().createBuffer(indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        renderer->device().copyToBuffer(cube.indexBuffer, (void *)cube_indices.data(), indexSize);
    }

    // Cylinder
    {
        const static float vertices[] = {
            0,         -1, -1,        0,         1,  -1,        0.19509,   1,  -0.980785, 0,         -1, -1,        0.19509,   1,  -0.980785,
            0.19509,   -1, -0.980785, 0.19509,   -1, -0.980785, 0.19509,   1,  -0.980785, 0.382683,  1,  -0.92388,  0.19509,   -1, -0.980785,
            0.382683,  1,  -0.92388,  0.382683,  -1, -0.92388,  0.382683,  -1, -0.92388,  0.382683,  1,  -0.92388,  0.55557,   1,  -0.83147,
            0.382683,  -1, -0.92388,  0.55557,   1,  -0.83147,  0.55557,   -1, -0.83147,  0.55557,   -1, -0.83147,  0.55557,   1,  -0.83147,
            0.707107,  1,  -0.707107, 0.55557,   -1, -0.83147,  0.707107,  1,  -0.707107, 0.707107,  -1, -0.707107, 0.707107,  -1, -0.707107,
            0.707107,  1,  -0.707107, 0.83147,   1,  -0.55557,  0.707107,  -1, -0.707107, 0.83147,   1,  -0.55557,  0.83147,   -1, -0.55557,
            0.83147,   -1, -0.55557,  0.83147,   1,  -0.55557,  0.92388,   1,  -0.382683, 0.83147,   -1, -0.55557,  0.92388,   1,  -0.382683,
            0.92388,   -1, -0.382683, 0.92388,   -1, -0.382683, 0.92388,   1,  -0.382683, 0.980785,  1,  -0.19509,  0.92388,   -1, -0.382683,
            0.980785,  1,  -0.19509,  0.980785,  -1, -0.19509,  0.980785,  -1, -0.19509,  0.980785,  1,  -0.19509,  1,         1,  0,
            0.980785,  -1, -0.19509,  1,         1,  0,         1,         -1, 0,         1,         -1, 0,         1,         1,  0,
            0.980785,  1,  0.19509,   1,         -1, 0,         0.980785,  1,  0.19509,   0.980785,  -1, 0.19509,   0.980785,  -1, 0.19509,
            0.980785,  1,  0.19509,   0.92388,   1,  0.382683,  0.980785,  -1, 0.19509,   0.92388,   1,  0.382683,  0.92388,   -1, 0.382683,
            0.92388,   -1, 0.382683,  0.92388,   1,  0.382683,  0.83147,   1,  0.55557,   0.92388,   -1, 0.382683,  0.83147,   1,  0.55557,
            0.83147,   -1, 0.55557,   0.83147,   -1, 0.55557,   0.83147,   1,  0.55557,   0.707107,  1,  0.707107,  0.83147,   -1, 0.55557,
            0.707107,  1,  0.707107,  0.707107,  -1, 0.707107,  0.707107,  -1, 0.707107,  0.707107,  1,  0.707107,  0.55557,   1,  0.83147,
            0.707107,  -1, 0.707107,  0.55557,   1,  0.83147,   0.55557,   -1, 0.83147,   0.55557,   -1, 0.83147,   0.55557,   1,  0.83147,
            0.382683,  1,  0.92388,   0.55557,   -1, 0.83147,   0.382683,  1,  0.92388,   0.382683,  -1, 0.92388,   0.382683,  -1, 0.92388,
            0.382683,  1,  0.92388,   0.19509,   1,  0.980785,  0.382683,  -1, 0.92388,   0.19509,   1,  0.980785,  0.19509,   -1, 0.980785,
            0.19509,   -1, 0.980785,  0.19509,   1,  0.980785,  0,         1,  1,         0.19509,   -1, 0.980785,  0,         1,  1,
            0,         -1, 1,         0,         -1, 1,         0,         1,  1,         -0.19509,  1,  0.980785,  0,         -1, 1,
            -0.19509,  1,  0.980785,  -0.19509,  -1, 0.980785,  -0.19509,  -1, 0.980785,  -0.19509,  1,  0.980785,  -0.382683, 1,  0.92388,
            -0.19509,  -1, 0.980785,  -0.382683, 1,  0.92388,   -0.382683, -1, 0.92388,   -0.382683, -1, 0.92388,   -0.382683, 1,  0.92388,
            -0.55557,  1,  0.83147,   -0.382683, -1, 0.92388,   -0.55557,  1,  0.83147,   -0.55557,  -1, 0.83147,   -0.55557,  -1, 0.83147,
            -0.55557,  1,  0.83147,   -0.707107, 1,  0.707107,  -0.55557,  -1, 0.83147,   -0.707107, 1,  0.707107,  -0.707107, -1, 0.707107,
            -0.707107, -1, 0.707107,  -0.707107, 1,  0.707107,  -0.83147,  1,  0.55557,   -0.707107, -1, 0.707107,  -0.83147,  1,  0.55557,
            -0.83147,  -1, 0.55557,   -0.83147,  -1, 0.55557,   -0.83147,  1,  0.55557,   -0.92388,  1,  0.382683,  -0.83147,  -1, 0.55557,
            -0.92388,  1,  0.382683,  -0.92388,  -1, 0.382683,  -0.92388,  -1, 0.382683,  -0.92388,  1,  0.382683,  -0.980785, 1,  0.19509,
            -0.92388,  -1, 0.382683,  -0.980785, 1,  0.19509,   -0.980785, -1, 0.19509,   -0.980785, -1, 0.19509,   -0.980785, 1,  0.19509,
            -1,        1,  0,         -0.980785, -1, 0.19509,   -1,        1,  0,         -1,        -1, 0,         -1,        -1, 0,
            -1,        1,  0,         -0.980785, 1,  -0.19509,  -1,        -1, 0,         -0.980785, 1,  -0.19509,  -0.980785, -1, -0.19509,
            -0.980785, -1, -0.19509,  -0.980785, 1,  -0.19509,  -0.92388,  1,  -0.382683, -0.980785, -1, -0.19509,  -0.92388,  1,  -0.382683,
            -0.92388,  -1, -0.382683, -0.92388,  -1, -0.382683, -0.92388,  1,  -0.382683, -0.83147,  1,  -0.55557,  -0.92388,  -1, -0.382683,
            -0.83147,  1,  -0.55557,  -0.83147,  -1, -0.55557,  -0.83147,  -1, -0.55557,  -0.83147,  1,  -0.55557,  -0.707107, 1,  -0.707107,
            -0.83147,  -1, -0.55557,  -0.707107, 1,  -0.707107, -0.707107, -1, -0.707107, -0.707107, -1, -0.707107, -0.707107, 1,  -0.707107,
            -0.55557,  1,  -0.83147,  -0.707107, -1, -0.707107, -0.55557,  1,  -0.83147,  -0.55557,  -1, -0.83147,  -0.55557,  -1, -0.83147,
            -0.55557,  1,  -0.83147,  -0.382683, 1,  -0.92388,  -0.55557,  -1, -0.83147,  -0.382683, 1,  -0.92388,  -0.382683, -1, -0.92388,
            -0.382683, -1, -0.92388,  -0.382683, 1,  -0.92388,  -0.19509,  1,  -0.980785, -0.382683, -1, -0.92388,  -0.19509,  1,  -0.980785,
            -0.19509,  -1, -0.980785, -0.19509,  -1, -0.980785, -0.19509,  1,  -0.980785, 0,         1,  -1,        -0.19509,  -1, -0.980785,
            0,         1,  -1,        0,         -1, -1,
        };
        cylinder.vertexCount = 189;

        // VERTICES
        VkDeviceSize vertexSize = sizeof(glm::vec3) * 189;
        cylinder.vertexBuffer = renderer->device().createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        renderer->device().copyToBuffer(cylinder.vertexBuffer, (void *)vertices, vertexSize);
    }

    // Plane
    {
        const static float vertices[] = {
            -1,
            0,
            1,
            1,
            0,
            1,
            1,
            0,
            -1,
            -1,
            0,
            1,
            1,
            0,
            -1,
            -1,
            0,
            -1,
        };
        plane.vertexCount = 6;

        // VERTICES
        VkDeviceSize vertexSize = sizeof(glm::vec3) * 6;
        plane.vertexBuffer = renderer->device().createBuffer(vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        renderer->device().copyToBuffer(plane.vertexBuffer, (void *)vertices, vertexSize);
    }
}

void Primitives::Cleanup(RenderManager *renderer)
{
    // TODO: stub
    Q_UNUSED(renderer)
}

void Primitives::DrawSphere(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &sphere.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, sphere.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, sphere.indexCount, 1, 0, 0, 0);
}

void Primitives::DrawCube(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, cube.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, cube.indexCount, 1, 0, 0, 0);
}

void Primitives::DrawCylinder(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cylinder.vertexBuffer.buffer, offsets);

    vkCmdDraw(commandBuffer, cylinder.vertexCount, 1, 0, 0);
}

void Primitives::DrawPlane(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &plane.vertexBuffer.buffer, offsets);

    vkCmdDraw(commandBuffer, plane.vertexCount, 1, 0, 0);
}
