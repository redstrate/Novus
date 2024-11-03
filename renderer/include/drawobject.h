// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "texture.h"

struct RenderPart {
    size_t numIndices;

    Buffer vertexBuffer; // Only used in the simple renderer
    Buffer indexBuffer;
    std::vector<Buffer> streamBuffer; // Only used in the game renderer

    int materialIndex = 0;
    physis_Part originalPart;
};

enum class MaterialType { Object, Skin };

struct RenderMaterial {
    physis_Material mat;
    MaterialType type = MaterialType::Object;
    physis_SHPK shaderPackage{};

    std::optional<Texture> diffuseTexture;
    std::optional<Texture> normalTexture;
    std::optional<Texture> specularTexture;
    std::optional<Texture> multiTexture;

    std::optional<Texture> tableTexture;

    Buffer materialBuffer;
};

struct DrawObject {
    QString name;

    physis_MDL model;
    std::vector<RenderPart> parts;
    std::array<glm::mat4, 768> boneData; // JOINT_MATRIX_SIZE_DAWNTRAIL
    std::vector<RenderMaterial> materials;
    glm::vec3 position;
    bool skinned = false;

    uint16_t from_body_id = 101;
    uint16_t to_body_id = 101;

    Buffer boneInfoBuffer;
};
