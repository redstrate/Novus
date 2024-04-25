# SPDX-FileCopyrightText: 2024 NotNite <hi@notnite.com>
# SPDX-License-Identifier: CC0-1.0

$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

$LocalDir = "./local"
$BuildDir = "$LocalDir/build"
$PrefixDir = (Get-Location).Path + "/prefix"

function Configure($Name, $Args = "") {
    cmake -B "$BuildDir-$Name" "-DCMAKE_PREFIX_PATH=$PrefixDir" "-DCMAKE_CXX_COMPILER=cl" "-DCMAKE_C_COMPILER=cl" "-DCMAKE_BUILD_TYPE=Debug" "-S" "$LocalDir/$Name" "-DCMAKE_INSTALL_PREFIX=$PrefixDir" $Args
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

# Setup Windows dependencies
Invoke-WebRequest https://xiv.zone/distrib/dependencies/gettext.zip -OutFile "$LocalDir/gettext.zip"
Expand-Archive -Path "$LocalDir/gettext.zip" -DestinationPath $PrefixDir -Force

Invoke-WebRequest https://xiv.zone/distrib/dependencies/iconv.zip -OutFile "$LocalDir/iconv.zip"
Expand-Archive -Path "$LocalDir/iconv.zip" -DestinationPath $PrefixDir -Force

Invoke-WebRequest https://cfhcable.dl.sourceforge.net/project/gnuwin32/gperf/3.0.1/gperf-3.0.1-bin.zip -OutFile "$LocalDir/gperf.zip"
Expand-Archive -Path "$LocalDir/gperf.zip" -DestinationPath $PrefixDir -Force

# Build zlib
Clone "zlib" "https://github.com/madler/zlib.git"
Configure "zlib" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-zlib" --config Debug --target install
CheckCompileResult "zlib"

# Build Extra CMake Modules
Clone "extra-cmake-modules" "https://invent.kde.org/frameworks/extra-cmake-modules.git"
Configure "extra-cmake-modules" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-extra-cmake-modules" --config Debug --target install
cmake --install "$BuildDir-extra-cmake-modules" --config Debug
CheckCompileResult "extra-cmake-modules"

# Build KI18n
Clone "ki18n" "https://invent.kde.org/frameworks/ki18n.git"
# Workaround for Windows
Configure "ki18n" "-DBUILD_TESTING=OFF"

(Get-Content -ReadCount 0 "$BuildDir-ki18n/cmake/build-pofiles.cmake") -replace 'FATAL_ERROR', 'WARNING' | Set-Content "$BuildDir-ki18n/cmake/build-pofiles.cmake"
cmake --build "$BuildDir-ki18n" --config Debug --target install
CheckCompileResult "ki18n"

# Build KCoreAddons
Clone "kcoreaddons" "https://invent.kde.org/frameworks/kcoreaddons.git"
Configure "kcoreaddons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kcoreaddons" --config Debug --target install
CheckCompileResult "kcoreaddons"

# Build KConfig
Clone "kconfig" "https://invent.kde.org/frameworks/kconfig.git"
Configure "kconfig" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kconfig" --config Debug --target install
CheckCompileResult "kconfig"

# Build KArchive
Clone "karchive" "https://invent.kde.org/frameworks/karchive.git"
Configure "karchive" "-DBUILD_TESTING=OFF" "-DWITH_BZIP2=OFF" "-DWITH_LIBLZMA=OFF" "-DWITH_LIBZSTD=OFF"
cmake --build "$BuildDir-karchive" --config Debug --target install
CheckCompileResult "karchive"

# Build KItemViews
Clone "kitemviews" "https://invent.kde.org/frameworks/kitemviews.git"
Configure "kitemviews" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kitemviews" --config Debug --target install
CheckCompileResult "kitemviews"

# Build KCodecs
Clone "kcodecs" "https://invent.kde.org/frameworks/kcodecs.git"
Configure "kcodecs" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kcodecs" --config Debug --target install
CheckCompileResult "kcodecs"

# Build KGuiAddons
Clone "kguiaddons" "https://invent.kde.org/frameworks/kguiaddons.git"
Configure "kguiaddons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kguiaddons" --config Debug --target install
CheckCompileResult "kguiaddons"

# Build KWidgetsAddons
Clone "kwidgetsaddons" "https://invent.kde.org/frameworks/kwidgetsaddons.git"
Configure "kwidgetsaddons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kwidgetsaddons" --config Debug --target install
CheckCompileResult "kwidgetsaddons"

# Build KColorScheme
Clone "kcolorscheme" "https://invent.kde.org/frameworks/kcolorscheme.git"
Configure "kcolorscheme" "-DBUILD_TESTING=OFF"
# Workaround for Windows
(Get-Content -ReadCount 0 "$PrefixDir/lib/cmake/KF6I18n/build-pofiles.cmake") -replace 'FATAL_ERROR', 'WARNING' | Set-Content "$PrefixDir/lib/cmake/KF6I18n/build-pofiles.cmake"
cmake --build "$BuildDir-kcolorscheme" --config Debug --target install
CheckCompileResult "kcolorscheme"

# Build KConfigWidgets
Clone "kconfigwidgets" "https://invent.kde.org/frameworks/kconfigwidgets.git"
Configure "kconfigwidgets" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kconfigwidgets" --config Debug --target install
CheckCompileResult "kconfigwidgets"

# Build KIconThemes
Clone "kiconthemes" "https://invent.kde.org/frameworks/kiconthemes.git"
Configure "kiconthemes" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kiconthemes" --config Debug --target install
CheckCompileResult "kiconthemes"

# Build Sonnet
Clone "sonnet" "https://invent.kde.org/frameworks/sonnet.git"
Configure "sonnet" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-sonnet" --config Debug --target install
CheckCompileResult "sonnet"

# Build KCompletion
Clone "kcompletion" "https://invent.kde.org/frameworks/kcompletion.git"
Configure "kcompletion" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kcompletion" --config Debug --target install
CheckCompileResult "kcompletion"

# Build KTextWidgets
Clone "ktextwidgets" "https://invent.kde.org/frameworks/ktextwidgets.git"
Configure "ktextwidgets" "-DBUILD_TESTING=OFF" "-DWITH_TEXT_TO_SPEECH=OFF"
cmake --build "$BuildDir-ktextwidgets" --config Debug --target install
CheckCompileResult "ktextwidgets"

# Build KXmlGui
Clone "kxmlgui" "https://invent.kde.org/frameworks/kxmlgui.git"
Configure "kxmlgui" "-DBUILD_TESTING=OFF" "-DFORCE_DISABLE_KGLOBALACCEL=ON"
cmake --build "$BuildDir-kxmlgui" --config Debug --target install
CheckCompileResult "kxmlgui"

# Build glm
Clone "glm" "https://github.com/g-truc/glm.git"
Configure "glm" "-DGLM_BUILD_TESTS=OFF"
cmake --build "$BuildDir-glm" --config Debug --target install
CheckCompileResult "glm"

# Build Corrosion
Clone "corrosion" "https://github.com/corrosion-rs/corrosion.git"
Configure "corrosion" "-DCORROSION_BUILD_TESTS=OFF"
cmake --build "$BuildDir-corrosion" --config Debug --target install
CheckCompileResult "corrosion"

# Build nlohmann
Clone "json" "https://github.com/nlohmann/json.git"
Configure "json" "-DJSON_BuildTests=OFF"
cmake --build "$BuildDir-json" --config Debug --target install
CheckCompileResult "json"

# Build stb
Clone "stb" "https://github.com/nothings/stb.git"
mv $LocalDir/stb/* $PrefixDir/include
CheckCompileResult "stb"

# Build SPIRV-Cross
Clone "SPIRV-Cross" "https://github.com/KhronosGroup/SPIRV-Cross.git"
Configure "SPIRV-Cross"
cmake --build "$BuildDir-SPIRV-Cross" --config Debug --target install
CheckCompileResult "SPIRV-Cross"

# Build SPIRV-Headers
Clone "SPIRV-Headers" "https://github.com/KhronosGroup/SPIRV-Headers.git"
Configure "SPIRV-Headers"
cmake --build "$BuildDir-SPIRV-Headers" --config Debug --target install
CheckCompileResult "SPIRV-Headers"
