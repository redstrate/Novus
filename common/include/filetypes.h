// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

enum class FileType { Unknown, ExcelList, ExcelHeader, ExcelData, Model, Texture, ShaderPackage, CharaMakeParams, Skeleton };

class FileTypes
{
public:
    static FileType getFileType(const QString &extension);

    static QString getFiletypeName(FileType fileType);
};