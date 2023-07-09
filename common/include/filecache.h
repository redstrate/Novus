#pragma once

#include <physis.hpp>
#include <QString>
#include <QMap>

struct GameData;

class FileCache {
public:
    explicit FileCache(GameData& data);

    physis_Buffer& lookupFile(const QString& path);

private:
    QMap<QString, physis_Buffer> cachedBuffers;
    GameData& data;
};