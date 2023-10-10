# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(mdlpart STATIC)
target_sources(mdlpart PRIVATE mdlpart.cpp)
target_link_libraries(mdlpart PUBLIC physis z Qt6::Core Qt6::Widgets renderer novus-common tinygltf)
target_include_directories(mdlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})