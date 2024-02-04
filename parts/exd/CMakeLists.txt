# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(exdpart STATIC)
target_sources(exdpart PRIVATE exdpart.cpp)
target_link_libraries(exdpart
        PUBLIC
        KF6::I18n
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(exdpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Novus::ExdPart ALIAS exdpart)