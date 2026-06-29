// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "buffer.h"
#include "shaderstructs.h"
#include "texture.h"

#include <QString>
#include <array>
#include <optional>
#include <physis.hpp>
#include <string>
#include <vector>

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
    std::string path;
    physis_Material mat;
    MaterialType type = MaterialType::Object;
    physis_SHPK shaderPackage{};

    std::optional<Texture> diffuseTexture;
    std::string diffuseTexturePath;
    std::optional<Texture> normalTexture;
    std::optional<Texture> specularTexture;
    std::optional<Texture> maskTexture;
    std::optional<Texture> indexTexture;

    std::optional<Texture> tableTexture;

    Buffer materialBuffer;
};

struct RenderLod {
    float range = 0.0f;
    std::vector<RenderPart> parts;
};

struct DrawObject {
    std::string name;
    physis_MDL model;
    std::vector<RenderLod> lods;
    std::array<glm::mat3x4, JOINT_MATRIX_SIZE_DAWNTRAIL> boneData; // JOINT_MATRIX_SIZE_DAWNTRAIL
    std::vector<RenderMaterial> materials;
    bool skinned = false;

    uint16_t from_body_id = 101;
    uint16_t to_body_id = 101;

    Buffer boneInfoBuffer;

    size_t chooseLod(const float distance) const
    {
        for (size_t i = 0; i < lods.size(); i++) {
            if (distance < lods[i].range) {
                return i;
            }
        }

        return lods.size() - 1;
    }
};

struct DrawObjectInstance {
    QString name;
    DrawObject *sourceObject = nullptr;
    Transformation transformation;
    BoundingBox lastBoundingBox{};
};
