# SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(mtrlpart STATIC)
set_common_properties(mtrlpart)
target_sources(mtrlpart
        PRIVATE
        knownvalues.h
        mtrlpart.cpp
        mtrlpart.h)
target_link_libraries(mtrlpart
        PUBLIC
        Physis::Physis
        Novus::TexPart
        KF6::I18n
        Qt6::Core
        Qt6::Widgets
        Novus::Common)
target_include_directories(mtrlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(mtrlpart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::MtrlPart ALIAS mtrlpart)
