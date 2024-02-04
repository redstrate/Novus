# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(sklbpart STATIC)
target_sources(sklbpart
        PRIVATE
        sklbpart.cpp
        sklbpart.h)
target_link_libraries(sklbpart
        PUBLIC
        KF6::I18n
        Novus::Common
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(sklbpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Novus::SklbPart ALIAS sklbpart)