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
    install(FILES $<TARGET_RUNTIME_DLLS:novus-launcher> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.novus.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.novus.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
endif()

# Qt 6.7 brings DEPLOY_TOOL_OPTIONS
if(QT_VERSION_MINOR LESS 7)
    set(deploy_tool_options_arg "")
    if (WIN32)
        set(deploy_tool_options_arg --no-quick-import --no-virtualkeyboard --no-opengl-sw --plugindir "${CMAKE_INSTALL_PREFIX}/bin/bin/plugins" --libdir "${CMAKE_INSTALL_PREFIX}/bin/bin/" --no-translations)
    endif()

    # We only need to call windeployqt on one executable
    qt_generate_deploy_app_script(
            TARGET novus-launcher
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
            DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
    )
    install(SCRIPT ${deploy_script})
else()
    message(INFO "App deployment requires Qt 6.7")
endif()
