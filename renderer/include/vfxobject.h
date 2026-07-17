// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <physis.hpp>
#include <string>

#include "buffer.h"

struct VfxModel {
    size_t numIndices = 0;

    Buffer vertexBuffer;
    Buffer indexBuffer;
};

struct VfxObject {
    std::string name;
    physis_Avfx vfx;

    std::vector<VfxModel> models;
    std::vector<physis_Texture> textures;
    std::vector<Texture> gameTextures;
};

struct VfxObjectInstance {
    QString name;
    VfxObject *sourceObject = nullptr;
    Transformation transformation;
};
