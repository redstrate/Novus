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

install(FILES zone.xiv.mdlviewer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES zone.xiv.mdlviewer.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(TARGETS novus-mdlviewer ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

qt_generate_deploy_app_script(
        TARGET novus-mdlviewer
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

if (WIN32)
    add_custom_command(TARGET novus-mdlviewer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:novus-mdlviewer> ${CMAKE_INSTALL_BINDIR}
            COMMAND_EXPAND_LISTS
    )
endif()