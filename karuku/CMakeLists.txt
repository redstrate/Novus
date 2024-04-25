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
        KF6::Archive
        Novus::Common
        Novus::ExdPart
        Physis::Physis
        Physis::Logger
        Qt6::Core
        Qt6::Widgets
        Qt6::Network)

install(TARGETS novus-karuku ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_add_app_icon(novus-karuku
        ICONS
        zone.xiv.karaku.svg
        ${CMAKE_SOURCE_DIR}/resources/16-excel.png
        ${CMAKE_SOURCE_DIR}/resources/32-excel.png
        ${CMAKE_SOURCE_DIR}/resources/48-excel.png
        ${CMAKE_SOURCE_DIR}/resources/256-excel.png
)

if (WIN32)
    set_target_properties(novus-karuku PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "ExcelEditor")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-karuku> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.karaku.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.karaku.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
endif()