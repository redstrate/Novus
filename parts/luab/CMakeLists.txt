# SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(luabpart STATIC)
set_common_properties(luabpart)
target_sources(luabpart PRIVATE luabpart.cpp luabpart.h scriptprocessor.cpp scriptprocessor.h)
target_link_libraries(luabpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        Qt6::Core
        Qt6::Widgets
        KF6::SyntaxHighlighting)
target_include_directories(luabpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(luabpart PRIVATE TRANSLATION_DOMAIN="novus")
if (NOT BUILD_FLATPAK)
    add_dependencies(luabpart luadec)
endif()

add_library(Novus::LuabPart ALIAS luabpart)
