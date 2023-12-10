# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(texpart STATIC)
target_sources(texpart PRIVATE
        imagelabel.cpp
        imagelabel.h
        texpart.cpp
        texpart.h)
target_link_libraries(texpart
        PUBLIC
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)
target_include_directories(texpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Novus::TexPart ALIAS texpart)