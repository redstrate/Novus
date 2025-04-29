# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

set(BUILD_SHARED_LIBS OFF)
add_compile_options(-w)

FetchContent_Declare(
        Corrosion
        GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
        GIT_TAG v0.5.2
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/Cargo.toml FEATURES logging)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libphysis/logger)

target_include_directories(physis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/target/public)
target_link_libraries(physis INTERFACE z)

add_library(Physis::Physis ALIAS physis)
add_library(Physis::Logger ALIAS physis-logger)

add_subdirectory(magic_enum EXCLUDE_FROM_ALL)
add_subdirectory(tinygltf EXCLUDE_FROM_ALL)
add_subdirectory(imgui EXCLUDE_FROM_ALL)
add_subdirectory(dxbc EXCLUDE_FROM_ALL)
