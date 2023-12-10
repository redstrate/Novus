# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-mdlviewer)
target_sources(novus-mdlviewer
        PRIVATE
        include/mainwindow.h

        src/main.cpp
        src/mainwindow.cpp)
target_include_directories(novus-mdlviewer
        PUBLIC
        include)
target_link_libraries(novus-mdlviewer
        PRIVATE
        Novus::MdlPart
        Novus::Common
        Physis::Physis
        Physis::Logger
        KF6::XmlGui
        Qt6::Core
        Qt6::Widgets)

install(TARGETS novus-mdlviewer ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
