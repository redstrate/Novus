// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecache.h"

#include "settings.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <physis.hpp>

using namespace Qt::StringLiterals;

FileCache::FileCache(physis_SqPackResource &data)
    : data(data)
{
    // TODO: move this into a generic parser
    if (gameModsEnabled()) {
        const auto mods = getGameMods();
        for (const auto &mod : std::as_const(mods)) {
            QFile defaultModFile(QDir(mod.path).absoluteFilePath(QStringLiteral("default_mod.json")));
            if (defaultModFile.open(QIODevice::ReadOnly)) {
                const auto document = QJsonDocument::fromJson(defaultModFile.readAll());
                const auto files = document.object()["Files"_L1].toObject();
                for (const auto [gamePath, filePath] : files.asKeyValueRange()) {
                    QString localPath = filePath.toString();
                    localPath.replace("\\"_L1, "/"_L1);
                    m_modFileOverrides[gamePath.toString()] = QDir(mod.path).absoluteFilePath(localPath);
                }
            } else {
                qWarning() << "Failed to find default_mod.json for" << mod.path;
            }
        }
    }
}

FileCache::~FileCache()
{
    for (const auto &buffer : cachedBuffers) {
        physis_free_file(&buffer);
    }
}

physis_Buffer &FileCache::lookupFile(const QString &path)
{
    QMutexLocker locker(&bufferMutex);

    if (!cachedBuffers.contains(path)) {
        if (m_modFileOverrides.contains(path)) {
            QFile file(m_modFileOverrides[path]);
            if (file.open(QIODevice::ReadOnly)) {
                const auto data = file.readAll();

                // TODO: how does this work with the free above?
                physis_Buffer buffer{};
                buffer.size = data.size();
                buffer.data = new uint8_t[data.size()];
                std::memcpy(buffer.data, data.data(), data.size());

                cachedBuffers[path] = buffer;
                return cachedBuffers[path];
            }
            qWarning() << "Failed to read supposed mod file" << m_modFileOverrides[path] << "and will fall back to game data";
        }

        const std::string pathstd = path.toStdString();
        cachedBuffers[path] = physis_sqpack_read(&data, pathstd.c_str());
    }

    return cachedBuffers[path];
}

Platform FileCache::platform() const
{
    return data.platform;
}

physis_SqPackResource &FileCache::resource() const
{
    return data;
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
