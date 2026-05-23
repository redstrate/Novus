// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include "novuscommon_export.h"

NOVUSCOMMON_EXPORT QString fromCString(const char *string);
NOVUSCOMMON_EXPORT const char *toCString(const QString &value);
