# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-sagasu)
target_sources(novus-sagasu PRIVATE
        src/main.cpp
        src/mainwindow.cpp
        src/filetreewindow.cpp
        src/filepropertieswindow.cpp)
target_include_directories(novus-sagasu PRIVATE include)
target_link_libraries(novus-sagasu PRIVATE physis z Qt6::Core Qt6::Widgets novus-common)

install(TARGETS novus-sagasu ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
