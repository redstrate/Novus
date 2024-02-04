# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(cmppart STATIC)
target_sources(cmppart PRIVATE cmppart.cpp cmppart.h)
target_link_libraries(cmppart
        PUBLIC
        KF6::I18n
        Physis::Physis
        magic_enum
        Qt6::Core
        Qt6::Widgets)
target_include_directories(cmppart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(cmppart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::CmpPart ALIAS cmppart)