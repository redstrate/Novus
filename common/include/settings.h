// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include "novuscommon_export.h"

#include <QUuid>

struct GameInstall {
    QUuid uuid;
    QString label;
    QString path;
};

NOVUSCOMMON_EXPORT QList<GameInstall> getGameInstalls();
NOVUSCOMMON_EXPORT void saveGameInstalls(QList<GameInstall> installs);
NOVUSCOMMON_EXPORT QString getGameDirectory(bool prompt = true);
NOVUSCOMMON_EXPORT bool addNewInstall();
