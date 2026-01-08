# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(exlpart STATIC)
set_common_properties(exlpart)
target_sources(exlpart PRIVATE exlpart.cpp)
target_link_libraries(exlpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(exlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(exlpart PRIVATE TRANSLATION_DOMAIN="novus")

add_library(Novus::ExlPart ALIAS exlpart)
