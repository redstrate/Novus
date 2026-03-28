// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QMap>
#include <QMutex>
#include <QString>
#include <physis.hpp>

#include "novuscommon_export.h"

struct physis_SqPackResource;

class NOVUSCOMMON_EXPORT FileCache
{
public:
    explicit FileCache(physis_SqPackResource &data);
    ~FileCache();

    bool fileExists(const QString &path);
    physis_Buffer &lookupFile(const QString &path);

    Platform platform() const;

    // NOTE: This is only a porting aid, and usages should eventually be removed!
    physis_SqPackResource &resource() const;

private:
    QMap<QString, physis_Buffer> cachedBuffers;
    QHash<QString, bool> cachedExist;
    physis_SqPackResource &data;
    QMutex bufferMutex, existMutex;
};
