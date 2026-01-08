# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-mdlviewer)
set_common_properties(novus-mdlviewer)
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
        KF6::XmlGui
        Qt6::Core
        Qt6::Widgets)

install(TARGETS novus-mdlviewer ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mdlviewer.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/novus)

ecm_add_app_icon(novus-mdlviewer
        ICONS
        ${CMAKE_SOURCE_DIR}/resources/16-model.png
        ${CMAKE_SOURCE_DIR}/resources/32-model.png
        ${CMAKE_SOURCE_DIR}/resources/48-model.png
        ${CMAKE_SOURCE_DIR}/resources/256-model.png
)

if (WIN32)
    set_target_properties(novus-mdlviewer PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "ModelViewer")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-mdlviewer> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.mdlviewer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.mdlviewer.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
endif()
