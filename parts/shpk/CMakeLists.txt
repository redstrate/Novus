# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

find_package(spirv_cross_core REQUIRED)
find_package(spirv_cross_glsl REQUIRED)
find_package(SPIRV-Headers REQUIRED)

add_library(shpkpart STATIC)
target_sources(shpkpart PRIVATE shpkpart.cpp)
target_link_libraries(shpkpart PUBLIC physis z dxbc spirv-cross-core spirv-cross-glsl Qt6::Core Qt6::Widgets novus-common)
target_include_directories(shpkpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(shpkpart PRIVATE -fexceptions)