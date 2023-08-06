// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMap>
#include <QString>
#include <physis.hpp>

struct GameData;

class FileCache {
public:
    explicit FileCache(GameData& data);

    physis_Buffer& lookupFile(const QString& path);

private:
    QMap<QString, physis_Buffer> cachedBuffers;
    GameData& data;
};