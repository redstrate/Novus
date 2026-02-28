// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filetypes.h"

#include <KLocalizedString>
#include <QMap>
#include <physis.hpp>

const static QMap<QString, FileType> extensionToType{{QStringLiteral("exl"), FileType::ExcelList},
                                                     {QStringLiteral("exh"), FileType::ExcelHeader},
                                                     {QStringLiteral("exd"), FileType::ExcelData},
                                                     {QStringLiteral("mdl"), FileType::Model},
                                                     {QStringLiteral("tex"), FileType::Texture},
                                                     {QStringLiteral("atex"), FileType::Texture},
                                                     {QStringLiteral("shpk"), FileType::ShaderPackage},
                                                     {QStringLiteral("cmp"), FileType::CharaMakeParams},
                                                     {QStringLiteral("sklb"), FileType::Skeleton},
                                                     {QStringLiteral("dic"), FileType::Dictionary},
                                                     {QStringLiteral("mtrl"), FileType::Material},
                                                     {QStringLiteral("luab"), FileType::LuaBytecode},
                                                     {QStringLiteral("hwc"), FileType::HardwareCursor},
                                                     {QStringLiteral("sgb"), FileType::SharedGroup},
                                                     {QStringLiteral("tmb"), FileType::TimelineMotion},
                                                     {QStringLiteral("shcd"), FileType::Shader},
                                                     {QStringLiteral("lgb"), FileType::LayerGroupBinary},
                                                     {QStringLiteral("lvb"), FileType::LayerVariableBinary},
                                                     {QStringLiteral("uld"), FileType::UILayoutDefinition},
                                                     {QStringLiteral("cutb"), FileType::CutsceneBinary},
                                                     {QStringLiteral("avfx"), FileType::AnimatedVisualEffect},
                                                     {QStringLiteral("pcb"), FileType::PlayerCollisionBinary},
                                                     {QStringLiteral("scd"), FileType::SoundCompressedData},
                                                     {QStringLiteral("ugd"), FileType::UIGeneratedData},
                                                     {QStringLiteral("lcb"), FileType::LevelCollisionBinary},
                                                     {QStringLiteral("uwb"), FileType::UWB},
                                                     {QStringLiteral("svb"), FileType::SkyVisibilityBinary},
                                                     {QStringLiteral("tera"), FileType::Terrain},
                                                     {QStringLiteral("stm"), FileType::StainingTemplate},
                                                     {QStringLiteral("pbd"), FileType::PreBoneDeformer},
                                                     {QStringLiteral("envb"), FileType::EnvironmentBinary},
                                                     {QStringLiteral("essb"), FileType::EnvironmentSoundScapeBinary},
                                                     {QStringLiteral("amb"), FileType::AmbientSet}};

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
                                                {FileType::LuaBytecode, i18n("Lua Bytecode")},
                                                {FileType::HardwareCursor, i18n("Hardware Cursor")},
                                                {FileType::SharedGroup, i18n("Shared Group")},
                                                {FileType::TimelineMotion, i18n("Timeline Motion")},
                                                {FileType::Shader, i18n("Shader")},
                                                {FileType::LayerGroupBinary, i18n("Layer Group Binary")},
                                                {FileType::LayerVariableBinary, i18n("Layer Variable Binary")},
                                                {FileType::UILayoutDefinition, i18n("UI Layout Definition")},
                                                {FileType::CutsceneBinary, i18n("Cutscene Binary")},
                                                {FileType::AnimatedVisualEffect, i18n("Animated Visual Effect")},
                                                {FileType::PlayerCollisionBinary, i18n("Player Collision Binary")}, // TODO: list.pcb needs special handling
                                                {FileType::SoundCompressedData, i18n("Sound Compressed Data")},
                                                {FileType::UIGeneratedData, i18n("UI Generated Data")},
                                                {FileType::LevelCollisionBinary, i18n("Level Collision Binary")},
                                                {FileType::UWB, i18n("UWB")},
                                                {FileType::SkyVisibilityBinary, i18n("Sky Visibility Binary")},
                                                {FileType::Terrain, i18n("Terrain")},
                                                {FileType::StainingTemplate, i18n("Staining Template")},
                                                {FileType::PreBoneDeformer, i18n("Pre Bone Deformer")},
                                                {FileType::EnvironmentBinary, i18n("Environment Binary")},
                                                {FileType::EnvironmentSoundScapeBinary, i18n("Environment Sound Scape Binary")},
                                                {FileType::AmbientSet, i18n("Ambient Set")}};

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
                                                {FileType::LuaBytecode, QStringLiteral("text-x-lua")},
                                                {FileType::HardwareCursor, QStringLiteral("cursor-arrow-symbolic")},
                                                {FileType::SharedGroup, QStringLiteral("object-group-symbolic")},
                                                {FileType::TimelineMotion, QStringLiteral("preferences-desktop-animations")},
                                                {FileType::Shader, QStringLiteral("paint-pattern-symbolic")}};

const static QMap<FileType, std::function<const char *(Platform, physis_Buffer)>> typeToDebug{{FileType::LayerGroupBinary, physis_lgb_debug},
                                                                                              {FileType::LayerVariableBinary, physis_lvb_debug},
                                                                                              {FileType::SharedGroup, physis_sgb_debug},
                                                                                              {FileType::UILayoutDefinition, physis_uld_debug},
                                                                                              {FileType::CutsceneBinary, physis_cutb_debug},
                                                                                              {FileType::Material, physis_mtrl_debug},
                                                                                              {FileType::AnimatedVisualEffect, physis_avfx_debug},
                                                                                              {FileType::PlayerCollisionBinary, physis_pcb_debug},
                                                                                              {FileType::SoundCompressedData, physis_scd_debug},
                                                                                              {FileType::Shader, physis_shcd_debug},
                                                                                              {FileType::ShaderPackage, physis_shpk_debug},
                                                                                              {FileType::ExcelData, physis_exd_debug},
                                                                                              {FileType::ExcelHeader, physis_exh_debug},
                                                                                              {FileType::Dictionary, physis_dic_debug},
                                                                                              {FileType::Model, physis_mdl_debug},
                                                                                              {FileType::LevelCollisionBinary, physis_lcb_debug},
                                                                                              {FileType::UWB, physis_uwb_debug},
                                                                                              {FileType::SkyVisibilityBinary, physis_svb_debug},
                                                                                              {FileType::Terrain, physis_tera_debug},
                                                                                              {FileType::Skeleton, physis_skeleton_debug},
                                                                                              {FileType::StainingTemplate, physis_stm_debug},
                                                                                              {FileType::PreBoneDeformer, physis_pbd_debug},
                                                                                              {FileType::CharaMakeParams, physis_cmp_debug},
                                                                                              {FileType::TimelineMotion, physis_tmb_debug},
                                                                                              {FileType::EnvironmentBinary, physis_envb_debug},
                                                                                              {FileType::EnvironmentSoundScapeBinary, physis_essb_debug},
                                                                                              {FileType::AmbientSet, physis_amb_debug}};

FileType FileTypes::getFileType(const QString &extension)
{
    return extensionToType.value(extension, FileType::Unknown);
}

QString FileTypes::getFiletypeName(FileType fileType)
{
    if (typeToName.contains(fileType)) {
        return typeToName.value(fileType);
    }
    return typeToName.value(FileType::Unknown);
}

QString FileTypes::getFiletypeIcon(FileType fileType)
{
    if (typeToIcon.contains(fileType)) {
        return typeToIcon.value(fileType);
    }
    return typeToIcon.value(FileType::Unknown);
}

QString FileTypes::printDebugInformation(FileType fileType, Platform platform, physis_Buffer buffer)
{
    if (typeToDebug.contains(fileType)) {
        const auto stringPtr = typeToDebug[fileType](platform, buffer);
        if (stringPtr != nullptr) {
            return QString::fromLatin1(stringPtr);
        }
    }
    return i18n("No debug information available. If this is unexpected, please report this a bug (and include the file path!)");
}
