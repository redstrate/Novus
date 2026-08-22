# SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-cutsceneeditor)
set_common_properties(novus-cutsceneeditor)
target_sources(novus-cutsceneeditor
        PRIVATE
        include/mainwindow.h
        include/cutscenelistwidget.h

        src/main.cpp
        src/mainwindow.cpp
        src/cutscenelistwidget.cpp)
target_include_directories(novus-cutsceneeditor
        PUBLIC
        include)
target_link_libraries(novus-cutsceneeditor
        PRIVATE
        Novus::Common
        Novus::MdlPart
        Novus::ScenePart
        Novus::CutbPart
        Physis::Physis
        Qt6::Core
        Qt6::Widgets
        KF6::ColorScheme)

install(TARGETS novus-cutsceneeditor ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES cutsceneeditor.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/novus)

if (WIN32)
    set_target_properties(novus-cutsceneeditor PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "CutsceneEditor")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-cutsceneeditor> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.novus.cutsceneeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
endif()
