# SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-mapeditor)
target_sources(novus-mapeditor
        PRIVATE
        include/mainwindow.h
        include/maplistwidget.h
        include/mapview.h
        include/objectpass.h
        include/objectlistwidget.h
        include/appstate.h
        include/objectlistmodel.h
        include/primitives.h

        src/main.cpp
        src/mainwindow.cpp
        src/maplistwidget.cpp
        src/mapview.cpp
        src/objectpass.cpp
        src/objectlistwidget.cpp
        src/appstate.cpp
        src/objectlistmodel.cpp
        src/primitives.cpp)
target_include_directories(novus-mapeditor
        PUBLIC
        include)
target_link_libraries(novus-mapeditor
        PRIVATE
        Novus::Common
        Novus::MdlPart
        Physis::Physis
        Qt6::Core
        Qt6::Widgets)

install(TARGETS novus-mapeditor ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES mapeditor.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/novus)

ecm_add_app_icon(novus-mapeditor
        ICONS
        ${CMAKE_SOURCE_DIR}/resources/16-map.png
        ${CMAKE_SOURCE_DIR}/resources/32-map.png
        ${CMAKE_SOURCE_DIR}/resources/48-map.png
        ${CMAKE_SOURCE_DIR}/resources/256-map.png
)

if (WIN32)
    set_target_properties(novus-mapeditor PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "MapEditor")

    install(FILES $<TARGET_RUNTIME_DLLS:novus-mapeditor> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.mapeditor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.mapeditor.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
endif()
