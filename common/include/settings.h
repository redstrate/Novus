// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include "novuscommon_export.h"

#include <QUuid>
#include <physis.hpp>

struct GameInstall {
    QUuid uuid;
    QString label;
    QString path;
    Language language;
};

NOVUSCOMMON_EXPORT QList<GameInstall> getGameInstalls();
NOVUSCOMMON_EXPORT void saveGameInstalls(QList<GameInstall> installs);
NOVUSCOMMON_EXPORT QString getGameDirectory(bool prompt = true);
NOVUSCOMMON_EXPORT bool addNewInstall();
NOVUSCOMMON_EXPORT Language getLanguage();
