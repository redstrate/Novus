// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetypes.h"

#include <KLocalizedString>
#include <QMap>

const static QMap<QString, FileType> extensionToType{{QStringLiteral("exl"), FileType::ExcelList},
                                                     {QStringLiteral("exh"), FileType::ExcelHeader},
                                                     {QStringLiteral("exd"), FileType::ExcelData},
                                                     {QStringLiteral("mdl"), FileType::Model},
                                                     {QStringLiteral("tex"), FileType::Texture},
                                                     {QStringLiteral("shpk"), FileType::ShaderPackage},
                                                     {QStringLiteral("cmp"), FileType::CharaMakeParams},
                                                     {QStringLiteral("sklb"), FileType::Skeleton}};

const static QMap<FileType, QString> typeToName{{FileType::Unknown, i18n("Unknown")},
                                                {FileType::ExcelList, i18n("Excel List")},
                                                {FileType::ExcelHeader, i18n("Excel Header")},
                                                {FileType::ExcelData, i18n("Excel Data")},
                                                {FileType::Model, i18n("Model")},
                                                {FileType::Texture, i18n("Texture")},
                                                {FileType::ShaderPackage, i18n("Shader Package")},
                                                {FileType::CharaMakeParams, i18n("Chara Make Params")},
                                                {FileType::Skeleton, i18n("Skeleton")}};

FileType FileTypes::getFileType(const QString &extension)
{
    return extensionToType.value(extension, FileType::Unknown);
}

QString FileTypes::getFiletypeName(FileType fileType)
{
    return typeToName.value(fileType);
}
