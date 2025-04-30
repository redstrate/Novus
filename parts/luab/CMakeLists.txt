# SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(luabpart STATIC)
target_sources(luabpart PRIVATE luabpart.cpp)
target_link_libraries(luabpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(luabpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(luabpart PRIVATE TRANSLATION_DOMAIN="novus")
add_dependencies(luabpart luadec)

add_library(Novus::LuabPart ALIAS luabpart)