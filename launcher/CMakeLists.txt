# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-launcher)
target_sources(novus-launcher
        PRIVATE
        include/mainwindow.h

        src/main.cpp
        src/mainwindow.cpp)
target_link_libraries(novus-launcher
        PRIVATE
        Novus::Common
        Physis::Physis
        KF6::ConfigCore
        Qt6::Widgets)
target_include_directories(novus-launcher PUBLIC include)

install(FILES zone.xiv.novus.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES zone.xiv.novus.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(TARGETS novus-launcher ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if (WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:novus-launcher> DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()