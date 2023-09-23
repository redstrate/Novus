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
target_link_libraries(NovusCommon PUBLIC Qt5::Core Qt5::Widgets KF5::ConfigCore KF5::XmlGui physis)
