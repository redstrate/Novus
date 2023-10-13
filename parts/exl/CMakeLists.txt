# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(exlpart STATIC)
target_sources(exlpart PRIVATE exlpart.cpp)
target_link_libraries(exlpart PUBLIC physis z Qt6::Core Qt6::Widgets)
target_include_directories(exlpart PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})