# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-mdlviewer)
target_sources(novus-mdlviewer PRIVATE
        src/main.cpp
        src/mainwindow.cpp)
target_include_directories(novus-mdlviewer
        PUBLIC
        include)
target_link_libraries(novus-mdlviewer PRIVATE physis physis-logger z Qt6::Core Qt6::Widgets KF6::XmlGui mdlpart novus-common)

install(TARGETS novus-mdlviewer ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
