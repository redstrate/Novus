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
    // Custom resource used for reading and parsing Excel files and other nonsense
    m_customResource = physis_custom_initialize(
        this,
        [](void *userData, const char *path) -> physis_Buffer {
            const auto cache = static_cast<FileCache *>(userData);
            return cache->lookupFile(QString::fromUtf8(path));
        },
        [](void *userData, const char *path) -> bool {
            const auto cache = static_cast<FileCache *>(userData);
            return cache->fileExists(QString::fromUtf8(path));
        });

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

    const QString normalizedPath = path.toLower();

    if (!cachedBuffers.contains(normalizedPath)) {
        if (m_modFileOverrides.contains(normalizedPath)) {
            QFile file(m_modFileOverrides[normalizedPath]);
            if (file.open(QIODevice::ReadOnly)) {
                const auto data = file.readAll();

                // TODO: how does this work with the free above?
                physis_Buffer buffer{};
                buffer.size = data.size();
                buffer.data = new uint8_t[data.size()];
                std::memcpy(buffer.data, data.data(), data.size());

                cachedBuffers[normalizedPath] = buffer;
                return cachedBuffers[normalizedPath];
            }
            qWarning() << "Failed to read supposed mod file" << m_modFileOverrides[normalizedPath] << "and will fall back to game data";
        }

        const std::string pathstd = normalizedPath.toStdString();
        cachedBuffers[normalizedPath] = physis_sqpack_read(&data, pathstd.c_str());
    }

    return cachedBuffers[normalizedPath];
}

physis_ExcelSheet FileCache::readExcelSheet(const QString &name, const physis_EXH *exh, const Language language)
{
    // Pass through our custom file loading mechanism
    return physis_custom_read_excel_sheet(&m_customResource, name.toStdString().c_str(), exh, language);
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
