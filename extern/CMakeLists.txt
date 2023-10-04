# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

find_package(Corrosion REQUIRED)


corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/Cargo.toml)

target_include_directories(physis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libphysis/target/public)
target_link_libraries(physis INTERFACE unshield)

add_subdirectory(magic_enum)
add_subdirectory(tinygltf)
add_subdirectory(imgui)
