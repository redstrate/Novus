# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.25)
project(Novus)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt5 COMPONENTS Core Widgets Concurrent CONFIG REQUIRED)

include(FetchContent)

add_subdirectory(extern)

# macos
if(APPLE)
    set(USE_STANDALONE_WINDOW TRUE)
endif()

add_subdirectory(renderer)
add_subdirectory(exdviewer)
add_subdirectory(armoury)
add_subdirectory(argcracker)
add_subdirectory(explorer)
#add_subdirectory(bonedecomp) # not yet ported to physis
add_subdirectory(parts)
add_subdirectory(common)
add_subdirectory(mdlviewer)
add_subdirectory(sdklauncher)