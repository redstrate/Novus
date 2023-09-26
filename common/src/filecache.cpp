// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecache.h"

#include <physis.hpp>

FileCache::FileCache(GameData& data) : data(data) {}

physis_Buffer& FileCache::lookupFile(const QString& path) {
    if (!cachedBuffers.contains(path)) {
        cachedBuffers[path] = physis_gamedata_extract_file(&data, path.toStdString().c_str());
    }

    return cachedBuffers[path];
}

bool FileCache::fileExists(const QLatin1String &path)
{
    if (!cachedExist.contains(path)) {
        cachedExist[path] = physis_gamedata_exists(&data, path.data());
    }

    return cachedExist[path];
}
