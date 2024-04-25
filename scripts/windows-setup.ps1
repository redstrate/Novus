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
git clone "https://github.com/madler/zlib.git" "$LocalDir/zlib"
Configure "zlib" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-zlib" --config Debug --target install

# Build Extra CMake Modules
git clone https://invent.kde.org/frameworks/extra-cmake-modules.git "$LocalDir/extra-cmake-modules"
Configure "extra-cmake-modules" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-extra-cmake-modules" --config Debug --target install
cmake --install "$BuildDir-extra-cmake-modules" --config Debug

# Build KI18n
git clone https://invent.kde.org/frameworks/ki18n.git "$LocalDir/ki18n"
# Workaround for Windows
Configure "ki18n" "-DBUILD_TESTING=OFF"

(Get-Content -ReadCount 0 "$BuildDir-ki18n/cmake/build-pofiles.cmake") -replace 'FATAL_ERROR', 'WARNING' | Set-Content "$BuildDir-ki18n/cmake/build-pofiles.cmake"
cmake --build "$BuildDir-ki18n" --config Debug --target install

# Build KCoreAddons
git clone https://invent.kde.org/frameworks/kcoreaddons.git "$LocalDir/kcoreaddons"
Configure "kcoreaddons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kcoreaddons" --config Debug --target install

# Build KConfig
git clone https://invent.kde.org/frameworks/kconfig.git "$LocalDir/kconfig"
Configure "kconfig" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kconfig" --config Debug --target install

# Build KArchive
git clone https://invent.kde.org/frameworks/karchive.git "$LocalDir/karchive"
Configure "karchive" "-DBUILD_TESTING=OFF" "-DWITH_BZIP2=OFF" "-DWITH_LIBLZMA=OFF" "-DWITH_LIBZSTD=OFF"
cmake --build "$BuildDir-karchive" --config Debug --target install

# Build KItemViews
git clone https://invent.kde.org/frameworks/kitemviews.git "$LocalDir/kitemviews"
Configure "kitemviews" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kitemviews" --config Debug --target install

# Build KCodecs
git clone https://invent.kde.org/frameworks/kcodecs.git "$LocalDir/kcodecs"
Configure "kcodecs" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kcodecs" --config Debug --target install

# Build KGuiAddons
git clone https://invent.kde.org/frameworks/kguiaddons.git "$LocalDir/kguiaddons"
Configure "kguiaddons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kguiaddons" --config Debug --target install

# Build KWidgetsAddons
git clone https://invent.kde.org/frameworks/kwidgetsaddons.git "$LocalDir/kwidgetsaddons"
Configure "kwidgetsaddons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kwidgetsaddons" --config Debug --target install

# Build KColorScheme
git clone https://invent.kde.org/frameworks/kcolorscheme.git "$LocalDir/kcolorscheme"
Configure "kcolorscheme" "-DBUILD_TESTING=OFF"
# Workaround for Windows
(Get-Content -ReadCount 0 "$PrefixDir/lib/cmake/KF6I18n/build-pofiles.cmake") -replace 'FATAL_ERROR', 'WARNING' | Set-Content "$PrefixDir/lib/cmake/KF6I18n/build-pofiles.cmake"
cmake --build "$BuildDir-kcolorscheme" --config Debug --target install

# Build KConfigWidgets
git clone https://invent.kde.org/frameworks/kconfigwidgets.git "$LocalDir/kconfigwidgets"
Configure "kconfigwidgets" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kconfigwidgets" --config Debug --target install

# Build KIconThemes
git clone https://invent.kde.org/frameworks/kiconthemes.git "$LocalDir/kiconthemes"
Configure "kiconthemes" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kiconthemes" --config Debug --target install

# Build Sonnet
git clone https://invent.kde.org/frameworks/sonnet.git "$LocalDir/sonnet"
Configure "sonnet" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-sonnet" --config Debug --target install

# Build KCompletion
git clone https://invent.kde.org/frameworks/kcompletion.git "$LocalDir/kcompletion"
Configure "kcompletion" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kcompletion" --config Debug --target install

# Build KTextWidgets
git clone https://invent.kde.org/frameworks/ktextwidgets.git "$LocalDir/ktextwidgets"
Configure "ktextwidgets" "-DBUILD_TESTING=OFF" "-DWITH_TEXT_TO_SPEECH=OFF"
cmake --build "$BuildDir-ktextwidgets" --config Debug --target install

# Build KXmlGui
git clone https://invent.kde.org/frameworks/kxmlgui.git "$LocalDir/kxmlgui"
Configure "kxmlgui" "-DBUILD_TESTING=OFF" "-DFORCE_DISABLE_KGLOBALACCEL=ON"
cmake --build "$BuildDir-kxmlgui" --config Debug --target install

# Build glm
git clone https://github.com/g-truc/glm.git "$LocalDir/glm"
Configure "glm" "-DGLM_BUILD_TESTS=OFF"
cmake --build "$BuildDir-glm" --config Debug --target install

# Build Corrosion
git clone https://github.com/corrosion-rs/corrosion.git "$LocalDir/corrosion"
Configure "corrosion" "-DCORROSION_BUILD_TESTS=OFF"
cmake --build "$BuildDir-corrosion" --config Debug --target install

# Build nlohmann
git clone https://github.com/nlohmann/json.git "$LocalDir/json"
Configure "json" "-DJSON_BuildTests=OFF"
cmake --build "$BuildDir-json" --config Debug --target install

# Build stb
git clone https://github.com/nothings/stb.git "$LocalDir/stb"
mv $LocalDir/stb/* $PrefixDir/include

# Build SPIRV-Cross
git clone https://github.com/KhronosGroup/SPIRV-Cross.git "$LocalDir/SPIRV-Cross"
Configure "SPIRV-Cross"
cmake --build "$BuildDir-SPIRV-Cross" --config Debug --target install

# Build SPIRV-Headers
git clone https://github.com/KhronosGroup/SPIRV-Headers.git "$LocalDir/SPIRV-Headers"
Configure "SPIRV-Headers"
cmake --build "$BuildDir-SPIRV-Headers" --config Debug --target install
