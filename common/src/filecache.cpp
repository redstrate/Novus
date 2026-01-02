// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecache.h"

#include <physis.hpp>

FileCache::FileCache(physis_SqPackResource &data)
    : data(data)
{
}

physis_Buffer &FileCache::lookupFile(const QString &path)
{
    QMutexLocker locker(&bufferMutex);

    if (!cachedBuffers.contains(path)) {
        std::string pathstd = path.toStdString();
        cachedBuffers[path] = physis_sqpack_read(&data, pathstd.c_str());
    }

    return cachedBuffers[path];
}

bool FileCache::fileExists(const QString &path)
{
    QMutexLocker locker(&existMutex);

    if (!cachedExist.contains(path)) {
        std::string pathstd = path.toStdString();
        cachedExist[path] = physis_sqpack_exists(&data, pathstd.c_str());
    }

    return cachedExist[path];
}
