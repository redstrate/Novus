// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KAboutData>
#include <QString>

#include "novuscommon_export.h"

NOVUSCOMMON_EXPORT void
customizeAboutData(const QString &componentName, const QString &desktopFilename, const QString &applicationTitle, const QString &applicationDescription);