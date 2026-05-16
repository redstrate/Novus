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
    explicit FileCache(physis_SqPackResource data);
    ~FileCache();

    [[nodiscard]] bool fileExists(const QString &path);
    [[nodiscard]] physis_Buffer &lookupFile(const QString &path);
    [[nodiscard]] physis_ExcelSheet readExcelSheet(const QString &name, const physis_EXH *exh, Language language);

    [[nodiscard]] Platform platform() const;

    // NOTE: This is only a porting aid, and usages should eventually be removed!
    [[nodiscard]] physis_SqPackResource &resource();

private:
    QMap<QString, physis_Buffer> m_cachedBuffers;
    QHash<QString, bool> m_cachedExist;
    physis_SqPackResource m_data;
    QMutex m_bufferMutex, m_existMutex;
    QMap<QString, QString> m_modFileOverrides;
    physis_CustomResource m_customResource;
};
