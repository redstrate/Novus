# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(mdlpart STATIC mdlpart.cpp)
target_link_libraries(mdlpart PUBLIC physis z Qt5::Core Qt5::Widgets renderer NovusCommon tinygltf)
target_include_directories(mdlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})