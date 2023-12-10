# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(novus-common STATIC)
target_sources(novus-common PRIVATE
        include/aboutdata.h
        include/filecache.h
        include/novusmainwindow.h
        include/quaternionedit.h
        include/settings.h
        include/vec3edit.h
        src/aboutdata.cpp
        src/filecache.cpp
        src/novusmainwindow.cpp
        src/quaternionedit.cpp
        src/settings.cpp
        src/vec3edit.cpp)
target_include_directories(novus-common PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(novus-common
        PUBLIC
        Physis::Physis
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::XmlGui
        Qt6::Core
        Qt6::Widgets)

add_library(Novus::Common ALIAS novus-common)
