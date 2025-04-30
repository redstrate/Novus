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
                                                     {QStringLiteral("sklb"), FileType::Skeleton},
                                                     {QStringLiteral("dic"), FileType::Dictionary},
                                                     {QStringLiteral("mtrl"), FileType::Material},
                                                     {QStringLiteral("luab"), FileType::LuaBytecode}};

const static QMap<FileType, QString> typeToName{{FileType::Unknown, i18n("Unknown")},
                                                {FileType::ExcelList, i18n("Excel List")},
                                                {FileType::ExcelHeader, i18n("Excel Header")},
                                                {FileType::ExcelData, i18n("Excel Data")},
                                                {FileType::Model, i18n("Model")},
                                                {FileType::Texture, i18n("Texture")},
                                                {FileType::ShaderPackage, i18n("Shader Package")},
                                                {FileType::CharaMakeParams, i18n("Chara Make Params")},
                                                {FileType::Skeleton, i18n("Skeleton")},
                                                {FileType::Dictionary, i18n("Dictionary")},
                                                {FileType::Material, i18n("Material")},
                                                {FileType::LuaBytecode, i18n("Lua Bytecode")}};

const static QMap<FileType, QString> typeToIcon{{FileType::Unknown, QStringLiteral("unknown")},
                                                {FileType::ExcelList, QStringLiteral("x-office-spreadsheet")},
                                                {FileType::ExcelHeader, QStringLiteral("x-office-spreadsheet")},
                                                {FileType::ExcelData, QStringLiteral("x-office-spreadsheet")},
                                                {FileType::Model, QStringLiteral("shape-cuboid-symbolic")},
                                                {FileType::Texture, QStringLiteral("viewimage-symbolic")},
                                                {FileType::ShaderPackage, QStringLiteral("paint-pattern-symbolic")},
                                                {FileType::CharaMakeParams, QStringLiteral("step_object_SoftBody-symbolic")},
                                                {FileType::Skeleton, QStringLiteral("user-symbolic")},
                                                {FileType::Dictionary, QStringLiteral("accessories-dictionary-symbolic")},
                                                {FileType::Material, QStringLiteral("map-globe-symbolic")},
                                                {FileType::LuaBytecode, QStringLiteral("text-x-lua")}};

FileType FileTypes::getFileType(const QString &extension)
{
    return extensionToType.value(extension, FileType::Unknown);
}

QString FileTypes::getFiletypeName(FileType fileType)
{
    return typeToName.value(fileType);
}

QString FileTypes::getFiletypeIcon(FileType fileType)
{
    return typeToIcon.value(fileType);
}
