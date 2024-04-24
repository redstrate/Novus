# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

set(BUILD_SHARED_LIBS OFF)
add_compile_options(-w)

find_package(Corrosion REQUIRED)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/Cargo.toml)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libphysis/logger)

target_include_directories(physis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/target/public)

add_library(Physis::Physis ALIAS physis)
add_library(Physis::Logger ALIAS physis-logger)

add_subdirectory(magic_enum EXCLUDE_FROM_ALL)
add_subdirectory(tinygltf EXCLUDE_FROM_ALL)
add_subdirectory(imgui EXCLUDE_FROM_ALL)
add_subdirectory(dxbc EXCLUDE_FROM_ALL)
