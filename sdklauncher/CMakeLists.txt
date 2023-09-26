# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(sdklauncher
        src/main.cpp
        src/mainwindow.cpp)
target_link_libraries(sdklauncher PUBLIC Qt5::Widgets KF5::ConfigCore physis z)
target_include_directories(sdklauncher PUBLIC include)
