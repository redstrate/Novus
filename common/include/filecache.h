// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QMap>
#include <QMutex>
#include <QString>
#include <physis.hpp>

#include "novuscommon_export.h"

struct SqPackResource;

class NOVUSCOMMON_EXPORT FileCache
{
public:
    explicit FileCache(SqPackResource &data);

    bool fileExists(const QString &path);
    physis_Buffer &lookupFile(const QString &path);

private:
    QMap<QString, physis_Buffer> cachedBuffers;
    QHash<QString, bool> cachedExist;
    SqPackResource &data;
    QMutex bufferMutex, existMutex;
};
