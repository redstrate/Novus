// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "primitives.h"
#include "rendermanager.h"

#include <complex>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

Sphere Primitives::sphere;

void Primitives::Initialize(RenderManager *renderer)
{
    // sphere
    {
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;

        unsigned int xResolution = 2;
        unsigned int yResolution = 2;
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
}

void Primitives::Cleanup(RenderManager *renderer)
{
    // TODO: stub
}

void Primitives::DrawSphere(VkCommandBuffer commandBuffer)
{
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &sphere.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, sphere.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, sphere.indexCount, 1, 0, 0, 0);
}
