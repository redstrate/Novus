# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

find_package(spirv_cross_core REQUIRED)
find_package(spirv_cross_glsl REQUIRED)
find_package(SPIRV-Headers REQUIRED)

add_library(shcdpart STATIC)
set_common_properties(shcdpart)
target_sources(shcdpart PRIVATE shcdpart.cpp shcdpart.h)
target_link_libraries(shcdpart
        PUBLIC
        Novus::Common
        Physis::Physis
        dxbc
        spirv-cross-core
        spirv-cross-glsl
        Qt6::Core
        Qt6::Widgets
        KF6::SyntaxHighlighting)
target_include_directories(shcdpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(shcdpart PRIVATE -fexceptions) # Needed for SPIRV-Cross
target_compile_definitions(shcdpart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::ShcdPart ALIAS shcdpart)
