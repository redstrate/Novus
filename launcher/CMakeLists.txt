# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-launcher)
target_sources(novus-launcher PRIVATE
        src/main.cpp
        src/mainwindow.cpp)
target_link_libraries(novus-launcher PRIVATe Qt6::Widgets KF6::ConfigCore physis z novus-common)
target_include_directories(novus-launcher PUBLIC include)
