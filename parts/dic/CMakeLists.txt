# SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(dicpart STATIC)
target_sources(dicpart PRIVATE dicpart.cpp)
target_link_libraries(dicpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(dicpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(dicpart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::DicPart ALIAS dicpart)