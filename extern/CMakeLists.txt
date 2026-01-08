# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

set(BUILD_SHARED_LIBS OFF)
add_compile_options(-w)

if (ENABLE_SANITIZERS)
    set(Rust_TOOLCHAIN "nightly")
endif()

FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.5.2
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/Cargo.toml)
target_include_directories(physis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/target/public)
if (ENABLE_SANITIZERS)
    corrosion_add_target_rustflags(physis -Zsanitizer=address -Zsanitizer=leak)
endif()

add_library(Physis::Physis ALIAS physis)

add_subdirectory(magic_enum EXCLUDE_FROM_ALL)
add_subdirectory(tinygltf EXCLUDE_FROM_ALL)
add_subdirectory(imgui EXCLUDE_FROM_ALL)
add_subdirectory(dxbc EXCLUDE_FROM_ALL)
add_subdirectory(rapidyaml EXCLUDE_FROM_ALL)

# TODO: Enable in the Flatpak, it's a bit annoying though as we would have to build it separately
if (NOT BUILD_FLATPAK)
    # For some reason, FFXIV uses a *32-bit* Lua compiler. We have to build it as 32-bit or else loading the bytecode fails.
    add_compile_options(-m32)
    add_link_options(-m32)
    add_subdirectory(luadec51 EXCLUDE_FROM_ALL)
endif()
