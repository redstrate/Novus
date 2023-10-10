// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility.h"

#include <QFile>

physis_Buffer utility::readFromQrc(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    auto contents = file.readAll();

    // TODO: kill me. later.
    physis_Buffer buffer;
    buffer.size = contents.size();
    buffer.data = static_cast<uint8_t *>(malloc(buffer.size));
    memcpy(buffer.data, contents.data(), buffer.size);

    return buffer;
}