# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

# config file used for app executable names
# TODO: haha this is horrible, make this easier pls
file (GENERATE
        OUTPUT "launcherconfig_$<CONFIG>.h.out"
        CONTENT "\
#pragma once\n\
#define GEAREDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-armoury>\")\n\
#define EXCELEDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-karuku>\")\n\
#define MAPEDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-mapeditor>\")\n\
#define MATEDITOR_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-mateditor>\")\n\
#define MDLVIEWER_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-mdlviewer>\")\n\
#define DATAEXPLORER_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-sagasu>\")\n\
#define GAMELAUNCHER_EXECUTABLE QStringLiteral(\"$<TARGET_FILE_NAME:novus-gamelauncher>\")\n\
        "
)

# copy the new file to something standard we'll plug into the C++
add_custom_command(
        COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "launcherconfig_$<CONFIG>.h.out" "launcherconfig.h"
        VERBATIM
        PRE_BUILD
        DEPENDS  "launcherconfig_$<CONFIG>.h.out"
        OUTPUT   "launcherconfig.h"
        COMMENT  "creating config.out file ({event: PRE_BUILD}, {filename: config.out})"
)

add_executable(novus-launcher)
target_sources(novus-launcher
        PRIVATE
        include/mainwindow.h

        src/main.cpp
        src/mainwindow.cpp

        launcherconfig.h)
target_link_libraries(novus-launcher
        PRIVATE
        Novus::Common
        Physis::Physis
        KF6::ConfigCore
        Qt6::Widgets)
target_include_directories(novus-launcher PUBLIC include PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS novus-launcher ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

if (WIN32)
    set_target_properties(novus-launcher PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "NovusSDK")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-launcher> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.novus.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.novus.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
endif()

ecm_add_app_icon(novus-launcher
        ICONS
        ${CMAKE_SOURCE_DIR}/resources/16-launcher.png
        ${CMAKE_SOURCE_DIR}/resources/32-launcher.png
        ${CMAKE_SOURCE_DIR}/resources/48-launcher.png
        ${CMAKE_SOURCE_DIR}/resources/256-launcher.png
)

# We only need to call deploy on one executable
qt_generate_deploy_app_script(
        TARGET novus-launcher
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})