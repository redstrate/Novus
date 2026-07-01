// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <glm/fwd.hpp>
#include <physis.hpp>

#include "novuscommon_export.h"

NOVUSCOMMON_EXPORT QString fromCString(const char *string);
NOVUSCOMMON_EXPORT const char *toCString(const QString &value);

NOVUSCOMMON_EXPORT glm::mat4 transformToMat4(const Transformation &transformation);
NOVUSCOMMON_EXPORT Transformation fromMat4(const glm::mat4 &m);
NOVUSCOMMON_EXPORT Transformation addTransformation(const Transformation &a, const Transformation &b);
