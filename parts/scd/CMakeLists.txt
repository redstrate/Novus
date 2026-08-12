# SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(scdpart STATIC)
set_common_properties(scdpart)
target_sources(scdpart PRIVATE scdpart.cpp scdpart.h)
target_link_libraries(scdpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia)
target_include_directories(scdpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(scdpart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::ScdPart ALIAS scdpart)
