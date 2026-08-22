# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(cutbpart STATIC)
set_common_properties(cutbpart)
target_sources(cutbpart PRIVATE cutbpart.cpp cutbpart.h)
target_link_libraries(cutbpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        magic_enum
        Qt6::Core
        Qt6::Widgets
        Novus::ScenePart)
target_include_directories(cutbpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(cutbpart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::CutbPart ALIAS cutbpart)
