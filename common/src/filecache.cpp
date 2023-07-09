#include "filecache.h"

#include <physis.hpp>

FileCache::FileCache(GameData& data) : data(data) {

}

physis_Buffer& FileCache::lookupFile(const QString& path) {
    if (!cachedBuffers.contains(path)) {
        cachedBuffers[path] = physis_gamedata_extract_file(&data, path.toStdString().c_str());
    }

    return cachedBuffers[path];
}

