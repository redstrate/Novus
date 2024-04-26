# SPDX-FileCopyrightText: 2024 NotNite <hi@notnite.com>
# SPDX-License-Identifier: CC0-1.0

$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

$LocalDir = "./local"
$BuildDir = "$LocalDir/build"
$PrefixDir = (Get-Location).Path + "/prefix"

$NumCores = [Environment]::ProcessorCount

function Configure($Name, $ExtraArgs = "") {
    $Command = "cmake -B $BuildDir-$Name -DCMAKE_PREFIX_PATH=$PrefixDir -DCMAKE_CXX_COMPILER=cl -DCMAKE_C_COMPILER=cl -DCMAKE_BUILD_TYPE=Debug -S $LocalDir/$Name -DCMAKE_INSTALL_PREFIX=$PrefixDir $ExtraArgs"
    Write-Output "Running $Command"
    Invoke-Expression $Command
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to configure $Name"
    }
}

function Clone($Name, $Url) {
    if (Test-Path "$LocalDir/$Name") {
        Write-Information "Skipping clone of $Name because it's source directory already exists"
    } else {
        git clone --depth=1 $Url "$LocalDir/$Name"

        if ($LASTEXITCODE -ne 0) {
            throw "Failed to clone $Name from $Url"
        }
    }
}

function CheckCompileResult($Name) {
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to build $Name!"
    }
}

if (!(Test-Path $LocalDir)) {
    New-Item -ItemType Directory -Path $LocalDir
}

# Build breeze icons
Clone "breeze-icons" "https://invent.kde.org/frameworks/breeze-icons.git"
Configure "breeze-icons" "-DICONS_LIBRARY=ON -DSKIP_INSTALL_ICONS=ON"
# Building it twice is intentional, the first time will always fail
cmake --build "$BuildDir-breeze-icons" --config Debug --target install --parallel $NumCores
cmake --build "$BuildDir-breeze-icons" --config Debug --target install --parallel $NumCores
CheckCompileResult "breeze-icons"
