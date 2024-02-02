# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-karuku)
target_sources(novus-karuku
        PRIVATE
        include/mainwindow.h
        include/sheetlistwidget.h

        src/main.cpp
        src/mainwindow.cpp
        src/sheetlistwidget.cpp)
target_include_directories(novus-karuku
        PUBLIC
        include)
target_link_libraries(novus-karuku
        PRIVATE
        Novus::Common
        Novus::ExdPart
        Physis::Physis
        Physis::Logger
        Qt6::Core
        Qt6::Widgets)

install(FILES zone.xiv.karaku.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES zone.xiv.karaku.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(TARGETS novus-karuku ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
