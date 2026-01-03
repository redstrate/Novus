// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include <glm/glm.hpp>
#include <physis.hpp>

// TODO: lol got rid of this
struct BoneData {
    glm::mat4 localTransform, finalTransform, inversePose;
};

void exportModel(const QString &name, const physis_MDL &model, const physis_Skeleton *skeleton, const std::vector<BoneData> &boneData, const QString &fileName);
