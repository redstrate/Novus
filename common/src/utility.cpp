// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <physis.hpp>

QString fromCString(const char *string)
{
    QString newString = QString::fromStdString(string);
    physis_free_string(string);
    return newString;
}

const char *toCString(const QString &value)
{
    const std::string stdStringData = value.toStdString();
    const auto cStringData = static_cast<char *>(malloc(stdStringData.length() + 1));
    strcpy(cStringData, stdStringData.data());

    return cStringData;
}

glm::mat4 transformToMat4(const Transformation &transformation)
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(transformation.translation[0], transformation.translation[1], transformation.translation[2]));
    m *= glm::mat4_cast(glm::quat(glm::vec3(transformation.rotation[0], transformation.rotation[1], transformation.rotation[2])));
    m = glm::scale(m, glm::vec3(transformation.scale[0], transformation.scale[1], transformation.scale[2]));

    return m;
}

Transformation fromMat4(const glm::mat4 &m)
{
    glm::vec3 translation{};
    glm::quat rotation{};
    glm::vec3 scale{};
    glm::vec3 skew{};
    glm::vec4 perspective{};
    glm::decompose(m, scale, rotation, translation, skew, perspective);

    auto eulerAngles = glm::eulerAngles(rotation);

    return Transformation{.translation = {translation[0], translation[1], translation[2]},
                          .rotation =
                              {
                                  eulerAngles[0],
                                  eulerAngles[1],
                                  eulerAngles[2],
                              },
                          .scale = {
                              scale[0],
                              scale[1],
                              scale[2],
                          }};
}

Transformation addTransformation(const Transformation &a, const Transformation &b)
{
    // NOTE: I know this is stupid, but I plan on replacing this whole system eventually.

    const auto aMat = transformToMat4(a);
    const auto bMat = transformToMat4(b);

    return fromMat4(aMat * bMat);
}
