# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_executable(novus-armoury)
target_sources(novus-armoury
        PRIVATE
        include/boneeditor.h
        include/cmpeditor.h
        include/fullmodelviewer.h
        include/gearlistmodel.h
        include/gearlistwidget.h
        include/gearview.h
        include/mainwindow.h
        include/materialview.h
        include/metadataview.h
        include/penumbraapi.h
        include/settingswindow.h
        include/singlegearview.h

        src/boneeditor.cpp
        src/cmpeditor.cpp
        src/fullmodelviewer.cpp
        src/gearlistmodel.cpp
        src/gearlistwidget.cpp
        src/gearview.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/materialview.cpp
        src/metadataview.cpp
        src/penumbraapi.cpp
        src/settingswindow.cpp
        src/singlegearview.cpp)
target_include_directories(novus-armoury
        PUBLIC
        include
        PRIVATE
        ${CMAKE_BINARY_DIR})
target_link_libraries(novus-armoury
        PUBLIC
        Novus::Common
        Novus::MdlPart
        Novus::CmpPart
        Novus::SklbPart
        Physis::Physis
        Physis::Logger
        imgui
        Qt6::Core
        Qt6::Widgets
        Qt6::Concurrent
        Qt6::Network)

install(TARGETS novus-armoury ${KF${QT_MAJOR_VERSION}_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_add_app_icon(novus-armoury
        ICONS
        zone.xiv.armoury.svg
        ${CMAKE_SOURCE_DIR}/resources/16-armoury.png
        ${CMAKE_SOURCE_DIR}/resources/32-armoury.png
        ${CMAKE_SOURCE_DIR}/resources/48-armoury.png
        ${CMAKE_SOURCE_DIR}/resources/256-armoury.png
)

if (WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:novus-armoury> DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    install(FILES zone.xiv.armoury.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES zone.xiv.armoury.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
endif()