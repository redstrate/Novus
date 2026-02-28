// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include "novuscommon_export.h"

#include <physis.hpp>

enum class FileType {
    Unknown,
    ExcelList,
    ExcelHeader,
    ExcelData,
    Model,
    Texture,
    ShaderPackage,
    CharaMakeParams,
    Skeleton,
    Dictionary,
    Material,
    LuaBytecode,
    HardwareCursor,
    SharedGroup,
    TimelineMotion,
    Shader,
    LayerGroupBinary,
    LayerVariableBinary,
    UILayoutDefinition,
    CutsceneBinary,
    AnimatedVisualEffect,
    PlayerCollisionBinary,
    SoundCompressedData,
    UIGeneratedData,
    LevelCollisionBinary,
    UWB, // TODO: what would this be called?
    SkyVisibilityBinary,
    Terrain,
    StainingTemplate,
    PreBoneDeformer,
    EnvironmentBinary,
    EnvironmentSoundScapeBinary,
    AmbientSet,
};

class NOVUSCOMMON_EXPORT FileTypes
{
public:
    static FileType getFileType(const QString &extension);

    static QString getFiletypeName(FileType fileType);
    static QString getFiletypeIcon(FileType fileType);
    static QString printDebugInformation(FileType fileType, Platform platform, physis_Buffer buffer);
};
