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
