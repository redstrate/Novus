# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(NovusCommon STATIC)
target_sources(NovusCommon PRIVATE
        include/aboutdata.h
        include/filecache.h
        include/settings.h
        src/aboutdata.cpp
        src/filecache.cpp
        src/settings.cpp)
target_include_directories(NovusCommon PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(NovusCommon PUBLIC Qt6::Core Qt6::Widgets KF6::CoreAddons KF6::ConfigCore KF6::XmlGui physis)
