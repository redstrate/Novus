# SPDX-FileCopyrightText: 2024 NotNite <hi@notnite.com>
# SPDX-License-Identifier: CC0-1.0

$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

$LocalDir = "./local"
$BuildDir = "./build"
$PrefixDir = (Get-Location).Path + "/prefix"

cmake -B "$BuildDir" "-DCMAKE_PREFIX_PATH=$PrefixDir" "-DCMAKE_CXX_COMPILER=cl" "-DCMAKE_C_COMPILER=cl" "-DCMAKE_BUILD_TYPE=Debug" "-S" . "-DCMAKE_INSTALL_PREFIX=$BuildDir"
cmake --build "$BuildDir" --config Debug --target install

Copy-Item -Path "$PrefixDir/bin/KF6ItemViews.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$PrefixDir/bin/KF6IconWidgets.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$PrefixDir/bin/KF6GuiAddons.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$PrefixDir/bin/KF6ColorScheme.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$PrefixDir/bin/intl-8.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$PrefixDir/bin/KF6IconThemes.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$PrefixDir/bin/iconv.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$PrefixDir/bin/zd.dll" -Destination "$BuildDir/bin"
Copy-Item -Path "$env:QTDIR/bin/Qt6PrintSupportd.dll" -Destination "$BuildDir/bin"
if (!(Test-Path "$BuildDir/plugins/sqldrivers")) {
    New-Item -ItemType Directory -Path "$BuildDir/plugins/sqldrivers"
}
Copy-Item -Path "$env:QTDIR/plugins/sqldrivers/qsqlited.dll" -Destination "$BuildDir/plugins/sqldrivers"
