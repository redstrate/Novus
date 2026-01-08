# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(tmbpart STATIC)
set_common_properties(tmbpart)
target_sources(tmbpart PRIVATE tmbpart.cpp)
target_link_libraries(tmbpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(tmbpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(tmbpart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::TmbPart ALIAS tmbpart)
