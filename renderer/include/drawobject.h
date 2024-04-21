// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

struct RenderPart {
    size_t numIndices;

    Buffer vertexBuffer, indexBuffer;

    int materialIndex = 0;
};

struct RenderTexture {
    VkImage handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};

enum class MaterialType { Object, Skin };

struct RenderMaterial {
    MaterialType type = MaterialType::Object;
    physis_SHPK shaderPackage;

    RenderTexture *diffuseTexture = nullptr;
    RenderTexture *normalTexture = nullptr;
    RenderTexture *specularTexture = nullptr;
    RenderTexture *multiTexture = nullptr;
};

struct DrawObject {
    QString name;

    physis_MDL model;
    std::vector<RenderPart> parts;
    std::array<glm::mat4, 128> boneData;
    std::vector<RenderMaterial> materials;
    glm::vec3 position;
    bool skinned = false;

    uint16_t from_body_id = 101;
    uint16_t to_body_id = 101;

    Buffer boneInfoBuffer;
};